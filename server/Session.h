// session.h

#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <deque>

#include "Message.h"
#include "ThreadSafeQueue.h"

using namespace boost::asio;

// This class is a deep copy with adaption from the tutorial given by Javidx9
// https://www.youtube.com/watch?v=UbjxGvrDrbw&list=PLIXt8mu2KcUJOwdLMp-Z-cDIZA1aZfVTN
// we wouldn't have a better understading on how a Server/Client works without him
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(ip::tcp::socket socket, std::shared_ptr<ThreadSafeQueue<message>> &message_in, int id) 
    : socket_(std::move(socket)), messages_in_(message_in), id_(id) {}

    void start() {
        std::cout << "[" << id_ <<  "] A new Session has started" << std::endl;
        read_header();
    }

    ~Session() {std::cout << "[" << id_ << "] Session destroyed" << std::endl;}

    bool isConnected() const {
        return socket_.is_open();
    }

    void send_message(const message &msg) {
        messages_out_.push_back(msg);
        if (is_writing || messages_out_.empty()) return; // empeĉhe des doubles writes
        is_writing = true;
        write_header();

    }

    int get_id() const {
        return id_;
    }
private:
    void read_header() {
        async_read(socket_, buffer(&tmp_message_.msg_header, sizeof(header)), 
            [this](std::error_code ec, std::size_t length) {
                if (!ec) {
                    // std::cout << "[DEBUG] " << id_ << " reading header" << std::endl;
                    tmp_message_.msg_header.sender_id = id_;

                    // resizing by the size of what the client tells us (we trust the client)
                    tmp_message_.body.resize(tmp_message_.msg_header.size); 
                    
                    read_body();
                } else {
                    std::cerr << "[" << id_ << "] Read header error: " << ec.message() << std::endl;
                    socket_.close();
                }
            }
        );
    }

    void read_body() {
        async_read(socket_, buffer(tmp_message_.body.data(), tmp_message_.msg_header.size),
            [this](std::error_code ec, std::size_t length) {
                if (!ec) {
                    // std::cout << "[DEBUG] " << id_ << " reading body" << std::endl;
                    messages_in_->push(tmp_message_);
                    
                    read_header();
                }else {
                    std::cerr << "[" << id_ << "] Read body error: " << ec.message() << std::endl;
                    socket_.close();
                }
            }
        );
    }

    void write_header() {
        if (messages_out_.empty()) return;
        async_write(socket_, buffer(&messages_out_.front().msg_header, sizeof(header)), 
            [this](const std::error_code& ec, std::size_t length) {
                if (!ec) {
                    write_body();
                } else {
                    std::cerr << "[" << id_ << "] Write header error: " << ec.message() << std::endl;
                    socket_.close();
                    is_writing = false;
                }
            }
        );
    }

    void write_body() {
        async_write(socket_, buffer(messages_out_.front().body.data(), messages_out_.front().body.size()),
            [this](const std::error_code& ec, std::size_t length) {
                if (!ec) {
                    messages_out_.pop_front(); // retire le message une fois entièrement envoyé

                    if (!messages_out_.empty()) {
                        write_header(); // enchaîne sur le prochain message si il y en a encore
                    } else {
                        is_writing = false;
                    }
                } else {
                    std::cerr << "[" << id_ << "] Write body error: " << ec.message() << std::endl;
                    socket_.close();
                    is_writing = false;
                }
            }
        );
    }

    ip::tcp::socket socket_;

    std::shared_ptr<ThreadSafeQueue<message>> messages_in_;     // owned by Server the message arriving on the server to be handle by it
    std::deque<message> messages_out_;                          // specific message to send to the client for this Session
    
    int id_;                                                    // ID of this Session

    message tmp_message_;                                       // tmp message to keep trace

    bool is_writing = false;
};

#endif // SESSION_H