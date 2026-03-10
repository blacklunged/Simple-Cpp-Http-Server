#include "../include/Server.hpp"
#include "../include/Logger.hpp"
#include <iostream>
#include <functional>

Server::Server(const std::string& root_dir, int port, size_t thread_count)
    : m_root_dir(root_dir),
      m_port(port),
      m_server_socket(Socket::Type::TCP),
      m_handler(std::make_unique<HttpClientHandler>(root_dir)),
      m_thread_pool(thread_count),
      m_running(false)
{
    Logger::logInfo(
        "Server instance created: root_dir=" + root_dir +
        " port=" + std::to_string(port) +
        " threads=" + std::to_string(thread_count)
    );
}

Server::~Server() {
    if (m_running) {
        stop();
    }
    Logger::logInfo("Server instance destroyed");
}

void Server::start() {
    try {
        Logger::logInfo("Server starting...");

        m_server_socket.setReuseAddress(true);

        m_server_socket.bind(m_port);

        Logger::logInfo(
            "Socket bound to port " + std::to_string(m_port)
        );

        m_server_socket.listen(10);

        m_running = true;

        Logger::logInfo(
            "Server started successfully! "
            "Listening on http://0.0.0.0:" + std::to_string(m_port) +
            " with root directory: " + m_root_dir
        );

        

    } catch (const std::exception& e) {
        Logger::logError(
            std::string("Failed to start server: ") + e.what()
        );
        throw;
    }
}

void Server::run() {
    if (!m_running) {
        Logger::logError("Cannot run server: it is not started");
        throw std::runtime_error("Server is not started");
    }

    while (m_running) {
        try {
            struct sockaddr_in client_addr{};

            Socket client_socket = m_server_socket.accept(client_addr);

            std::string client_ip = Socket::getClientAddress(client_addr);
            int client_port = Socket::getClientPort(client_addr);

            std::cout << "\n[CONNECTION] New client connected from "
                      << client_ip << ":" << client_port << std::endl;

            Logger::logInfo(
                "Client connected: " + client_ip + ":" + std::to_string(client_port)
            );
            auto sock_ptr = std::make_shared<Socket>(std::move(client_socket));
            m_thread_pool.enqueue(
                [this, client_ip, client_port,
                 sock_ptr]() mutable {
                    auto tid = std::this_thread::get_id();
                    std::ostringstream oss;
                    oss << tid; 
                    try {
                        m_handler->handleClient(std::move(*sock_ptr));

                        std::cout << "[CONNECTION] Client disconnected: "
                                  << client_ip << ":" << client_port << std::endl;

                        Logger::logInfo(
                            "Client disconnected: " + client_ip + ":" +
                            std::to_string(client_port) +" in thread " + oss.str()
                        );

                    } catch (const std::exception& e) {
                        Logger::logError(
                            std::string("Error handling client ") + client_ip +
                            ":" + std::to_string(client_port) +
                            " - " + e.what()
                        );
                    }
                }
            );
            Logger::logInfo(
                "Queue size: " +std::to_string(m_thread_pool.getQueueSize())
            );

        } catch (const std::exception& e) {
            if (m_running) {
                Logger::logError(
                    std::string("Exception in accept loop: ") + e.what()
                );
            }
        }
    }

}

void Server::stop() {
    if (!m_running) {
        return;
    }

    Logger::logInfo("Server stopping...");

    m_running = false;

    try {
        m_server_socket.close();
        Logger::logInfo("Server socket closed");
    } catch (const std::exception& e) {
        Logger::logError(
            std::string("Error closing server socket: ") + e.what()
        );
    }

    Logger::logInfo("Stopping ThreadPool...");
    m_thread_pool.stop();

    Logger::logInfo("Server stopped successfully");

    std::cout << "\n=== HTTP Server Stopped ===" << std::endl;
}
