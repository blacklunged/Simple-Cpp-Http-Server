#include "../include/Server.hpp"
#include "../include/Logger.hpp"
#include "../include/Config.hpp"

#include <iostream>
#include <filesystem>
#include <csignal>
#include <atomic>


std::atomic<Server*> g_server(nullptr);

void signalHandler(int signum) {
    std::cout << "\n\nReceived signal " << signum << std::endl;

    Server* server = g_server.load();
    if (server && server->isRunning()) {
        Logger::logInfo("Signal received, shutting down...");
        server->stop();
    }
}

int main(int argc, char* argv[]) {

    Config cfg;
    cfg.loadFromFile("server.conf");
    cfg.parse(argc, argv);  
    cfg.print(); 
        try {
        Logger::init(cfg.access_log, cfg.error_log);

        Logger::logInfo("Application started");

        std::string root_dir = cfg.root_dir;
        int port = cfg.port;
        size_t thread_count = cfg.thread_count;

        if (!std::filesystem::exists(root_dir)) {
            std::filesystem::create_directories(root_dir);
            std::cout << "✓ Created directory: " << root_dir << std::endl;
            Logger::logInfo("Created root directory: " + root_dir);
        }

        Server server(root_dir, port, thread_count);

        g_server.store(&server);
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        server.start();

        server.run();

        std::cout << "\nServer shutdown complete." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        Logger::logError(std::string("Fatal error: ") + e.what());
        return 1;
    }

    return 0;
}
