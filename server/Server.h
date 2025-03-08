// server.h

#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <atomic>

#include "Message.h"
#include "Session.h"
#include "ThreadSafeQueue.h"

#include "SocialManager.h"
#include "GameManager.h"

using namespace boost::asio;

// This class is a deep copy from the cpp con 2016 provided by Michael Caisse
// see https://www.youtube.com/watch?v=UUiM-KNH69E&t for a better understanding
// and also adapted from 
// https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio/example/cpp14/echo/async_tcp_echo_server.cpp
// This class is an adaption from the tutorial given by Javidx9
// https://www.youtube.com/watch?v=UbjxGvrDrbw&list=PLIXt8mu2KcUJOwdLMp-Z-cDIZA1aZfVTN
// we wouldn't have a better understading on how a Server/Client works without him
class Server {

public:
    Server() :  acceptor_(io_context_),
                socket_(io_context_),
                message_in_(std::make_shared<ThreadSafeQueue<message>>()),
                message_out_(std::make_shared<ThreadSafeQueue<message_out>>()),
                game_manager_(message_out_),
                social_manager_(message_out_) {}

    void start(uint16_t port = 8080) {
        running_ = true;

        // setting up the listening
        ip::tcp::endpoint endpoint(ip::make_address("127.0.0.1"), port);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(ip::tcp::acceptor::reuse_address(true)); // 
        acceptor_.bind(endpoint);
        acceptor_.listen();

        // launching the async accept process
        do_accept();

        // running the asio context in a secondary thread to allow 
        // update to work !
        context_thread_ = std::thread([this](){io_context_.run();});

        // launching the manager object to handle their specific task
        social_manager_.start();
        game_manager_.start();

        std::cout << "[SERVER] started" << std::endl;
    }

    void stop() {
        running_ = false;

        // stoping the manager instances
        game_manager_.stop();
        social_manager_.stop();

        // stoping the asio context (idk but maybe place it after the thread end... ?)
        io_context_.stop();

        // Join all threads
        // https://stackoverflow.com/questions/38894188/thread-join-c-behaviour
        if (context_thread_.joinable()) context_thread_.join();

        std::cout << "[SERVER] stopped" << std::endl;
    }

    void check_sessions_() {
        // checking if user are connected or not and removing the disconnected one
    }

    void message_client(std::shared_ptr<Session> client, const message &msg) {
        // messaging a specific client
        client->send_message(msg);
    }
    
    // searching the session by id and sending the message to the specific session id
    void message_id(int id, const message &msg) {
        for (auto& se_pair : sessions_) {
            if (se_pair.second->get_id() == id) {
                message_client(se_pair.second, msg);
            }
        }
    }
    
    void message_all_client(const message &msg) {
        for (auto& se_pair : sessions_) {
            auto& session = se_pair.second;
            if (session->isConnected()) {
                message_client(session, msg);
            } else {
                // method to remove the session from the vector
                // we need to ensure that the session object is
                // destroyed (make a debug message inside the 
                // destructor function)
            }
        }
    }

    void update() {
        // refreshly handle the messaging in
        while(!message_in_->is_empty()) {
            auto msg = message_in_->pop();
            // dispatching the message 
            if (msg.has_value()) {
                if (msg->msg_header.category == category_t::SOCIAL) {
                    social_manager_.add_message(*msg);
                } else if (msg->msg_header.category == category_t::GAME) {
                    game_manager_.add_message(*msg);
                }
            }
        }

        // refreshly handle the message out
        while(!message_out_->is_empty()) {
            auto msg_out = message_out_->pop();
            if (msg_out.has_value()) {
                int id = msg_out->session_id;
                try {
                    if (sessions_.at(id)->isConnected()) {
                        std::cout << "[SERVER] Sening a message to Session: " << id << std::endl;
                        sessions_.at(id)->send_message(msg_out->msg);
                    } else {
                        // removing session
                        sessions_.erase(id);
                    }
                } catch(const std::exception& ec) {
                    std::cerr << "[SERVER] Tried to access to a deleted Session";
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::microseconds(10000)); // 10 ms
    }

    bool is_running() const {
        return running_;
    }

    ~Server() {this->stop();}

private:

    void do_accept() {
        // ASYNC : Accepting connexion and creating a Session object linked in a map
        // and lauching the asynchronous task for the session
        acceptor_.async_accept(socket_, [this](const boost::system::error_code &ec) {
            if (!ec) {
                std::cout << "[SERVER] New Connexion Established" << std::endl;
                int current_id = id_counter_++;
                sessions_.emplace(current_id, std::make_shared<Session>(std::move(socket_), message_in_, current_id));
                sessions_.at(current_id)->start();
            } else {
                std::cerr << "[SERVER] [ERROR]: " << ec.message() << std::endl;
            }
            // Relaunching the async accept process
            do_accept();
            // test_server(); // [DEBUG] retrying
        });
    }
    
    std::map<int, std::shared_ptr<Session>> sessions_;           // map of user's session where their id is the key

    io_context io_context_;         // context

    std::thread context_thread_;    // thread for running context

    std::atomic<bool> running_{false};  // atomic flag to ensure the proper server to work or not.

    ip::tcp::acceptor acceptor_;    // acceptor objet
    ip::tcp::socket socket_;        // temporary socket to accept connexion

    std::shared_ptr<ThreadSafeQueue<message>> message_in_;       // queue of the input message readed
    std::shared_ptr<ThreadSafeQueue<message_out>> message_out_;  // queue of the input message to be send to the specific Session

    // manager of the server
    GameManager game_manager_;
    SocialManager social_manager_;

    int id_counter_ = 10000;        // to keep trace of the id
};

#endif // SERVER_H