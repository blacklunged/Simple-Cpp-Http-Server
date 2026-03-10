#ifndef SERVER_HPP
#define SERVER_HPP

#include "Socket.hpp"
#include "HttpClientHandler.hpp"
#include "ThreadPool.hpp"
#include "Logger.hpp"

#include <atomic>
#include <memory>
#include <arpa/inet.h>

class Server {
public:

    Server(const std::string& root_dir, int port, size_t thread_count);

    ~Server();

    void start();

    void run();

    void stop();

    int getPort() const { return m_port; }

    std::string getRootDir() const { return m_root_dir; }

    size_t getThreadCount() const { return m_thread_pool.getThreadCount(); }

    bool isRunning() const { return m_running.load(); }

private:
    std::string m_root_dir;
    int m_port;

    Socket m_server_socket;
    std::unique_ptr<HttpClientHandler> m_handler;
    ThreadPool m_thread_pool;

    std::atomic<bool> m_running;
};

#endif // SERVER_HPP
