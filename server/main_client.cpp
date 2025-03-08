#include <boost/asio.hpp>
#include "Client.h"
#include <thread>
#include <atomic>
#include <csignal>

using namespace boost::asio;

std::atomic<bool> running{true};

void signal_handler(int signum) {
    running = false;
}

int main()  {
    // Gestion primaire des signaux 
    // https://stackoverflow.com/questions/68100462/signal-handling-c
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {   
        Client client;
        client.start();

        while (running && client.is_running()) {
            client.update();
        }

        std::cout << "[MAIN CLIENT] Stopping Client" << std::endl;
        client.stop();
    }
    catch (std::exception& e) {
        std::cerr << "[MAIN CLIENT] Error occured: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}