#ifndef SOCIALMANAGER_H
#define SOCIALMANAGER_H

#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include <atomic>
#include "json.hpp"
#include "sha512.hh"
#include "../database/DatabaseInit.hpp"
#include "../database/DatabaseUserManager.hpp"
#include "../database/DatabaseFriendManager.hpp"
#include "Message.h"
#include "ThreadSafeQueue.h"

class SocialManager {
public:
    explicit SocialManager(std::shared_ptr<ThreadSafeQueue<message_out>> msg_out);
    void start();
    void stop();
    std::string signUserIn(std::vector<uint8_t> user_combo);
    bool checkToken(std::vector<uint8_t> user_token);
    void msg_handler(message& msg);
    void add_message(message& msg);
    void send_message(message_out& msg);

private:
    std::string hashPassword(const std::string& plaintext);
    std::string generateClientToken();

    // message received by the server to handle by this
    ThreadSafeQueue<message> message_queue_;

    // queue of message to send to server (where server is the owner of this shared_ptr)
    std::shared_ptr<ThreadSafeQueue<message_out>> message_out_;

    // thread handling the message in
    std::atomic<bool> running_{false};
    std::thread worker_thread_;


    DatabaseInit database_;
    DatabaseUserManager database_user_manager_;

};

struct PlayerScore {
    std::string name;
    int score;
};

class Leaderboard {
private:
    std::vector<PlayerScore> scores;
    std::string filename;

public:
    Leaderboard(const std::string& file);
    void addScore(const std::string& name, int score);
    void display();
    void saveToFile() const;
    void loadFromFile();
};
#endif // SOCIALMANAGER_H
