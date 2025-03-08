#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <atomic>

#include "Message.h"
#include "ThreadSafeQueue.h"
#include "Lobby.h"

class GameManager {
public:
    GameManager(std::shared_ptr<ThreadSafeQueue<message_out>> msg_out) : message_out_(msg_out){}

    std::vector<std::string> split(std::vector<u_int8_t> body){
        std::stringstream stream(std::string(body.begin(), body.end()));
        std::vector<std::string> result;
        std::string item;
        while (std::getline(stream, item, ',')) {
            result.push_back(item);
        }
        return result;
    }
    Lobby get_lobby(std::string lobby_id){
        for (size_t i = 0; i < lobbies_.size(); ++i) {
            if (lobbies_[i].get_code() == lobby_id) {
              return lobbies_[i];
            }
        }
    }
    void join_lobby(std::vector<u_int8_t> body,int playerID){
        std::vector<std::string> lobby_info = split(body);
        Lobby lobby = get_lobby(lobby_info[0]);
        lobby.add_player(playerID);


    }

    void create_lobby(std::vector<u_int8_t> body,int playerID){
        std::vector<std::string> lobby_info = split(body);
        Lobby lobby = Lobby(lobby_info);
        lobby.add_player(playerID);
        lobbies_.push_back(Lobby(lobby));
    }

    void start() {
        running_ = true;
        // launching the thread to handle the message in the queue
        worker_thread_ = std::thread([this](){
            while(running_) {
                while(!message_queue_.is_empty()) {
                    auto msg = message_queue_.pop();
                    switch (msg->msg_header.instruction) {
                        case instruct_t::CREATE_LOBBY:
                            create_lobby(msg->body,msg->msg_header.sender_id); //créer un lobby avec le mdp contenu dans le body,
                                                     // le body contient le mdp le type de game et toute les infos nécessaires séparés par des virgules
                        break;
                        case instruct_t::JOIN_LOBBY:
                            join_lobby(msg->body,msg->msg_header.sender_id); //join un lobby selon un certains mdp
                        case instruct_t::START_GAME:
                            // start_game(msg->body);
                        default:
                        break;
            }
                    // handling the message depending on the instruction and if 
                    // needed to return smth to the client just build a msg_out
                    // and call send_message
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            std::cout << "[GAME_MANAGER] Thread Stopped" << std::endl;
        });
    }

    void stop() {
        running_ = false;
        // https://stackoverflow.com/questions/38894188/thread-join-c-behaviour
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    void add_message(message &msg) {
        // handling the message properly
        std::cout << "[GAME_MANAGER] Received a new message: " << msg << std::endl;
        message_queue_.push(msg);
    }

    void send_message(message_out &msg) {
        // adding msg to send for the server to handle
        message_out_->push(msg);
    }

private:
    // message received by the server to handle by this
    ThreadSafeQueue<message> message_queue_;

    // queue of message to send to server (where server is the owner of this shared_ptr)
    std::shared_ptr<ThreadSafeQueue<message_out>> message_out_;

    // thread handling the message in
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
    std::vector<Lobby> lobbies_;
};

#endif // GAMEMANAGER_H