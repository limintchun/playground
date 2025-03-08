#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <boost/asio.hpp>
#include <thread>
#include <regex>
#include <fstream>
#include "json.hpp"

#include "Message.h"
#include "Session.h"
#include "ThreadSafeQueue.h"

using namespace boost::asio;
using boost::asio::ip::tcp;

constexpr  int USERNAME_MIN_CHAR = 3;
constexpr  int TOKEN_SIZE = 36;

class Client {
public:
    void test_client();

    Client(uint16_t port = 8080);
    void start();
    void stop();
    bool is_running() const;
    void update();
    void checkAuth();


    std::string getUsername();
    std::string getPassword();
    void singUserIn();
    void handle_message(message& msg);
    void storeSessionInfo(const std::string& client_token,const std::string& username);
    [[nodiscard]] bool getAuthStatus() const;


private:
    bool is_user_authenticated_;
    std::string username_;

    void message_server(const message& msg);
    void connect(tcp::endpoint& endpoint);

    io_context io_context_;         // context
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    std::thread context_thread_;    // thread for running context
    std::atomic<bool> running_{false};  // atomic flag to handle the stop method.
    std::shared_ptr<ThreadSafeQueue<message>> messages_in_;
    std::unique_ptr<Session> session;

};

#endif // CLIENT_H
