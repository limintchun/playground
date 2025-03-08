// main.cpp

#include <atomic>
#include <csignal>

#include "chrono"
#include "Server.h"
#include "Message.h"


std::atomic<bool> running{true};

void signal_handler(int signum) {
    running = false;
}

int main() {
    // Gestion primaire des signaux 
    // https://stackoverflow.com/questions/68100462/signal-handling-c
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        Server server;
        server.start(8080);

        // keep updating (where update pause the thread)
        while(running && server.is_running()) {
            server.update();
        }

        std::cout << "[MAIN] Stopping Server" << std::endl;
        server.stop();

    } catch (const std::exception &e) {
        std::cerr << "[MAIN] Error occured: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
