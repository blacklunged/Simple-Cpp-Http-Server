#ifndef Socket_hpp
#define Socket_hpp

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h> 
#include <vector>
#include <unistd.h>
#include <fcntl.h>

class Socket
{

public:

    enum class Type{
        TCP,
        UDP
    };

    explicit Socket (Type type);
    explicit Socket (int file_descriptor);


    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    void bind(const std::string &address, int port);
    void bind(int port);
    void listen(int backlog = 10);

    Socket accept();
    Socket accept(struct sockaddr_in& client_address);
    
    ssize_t send(const std::string& data) const;
    ssize_t send(const void* data, size_t length) const;

    ssize_t recv(void* buffer, size_t buffer_size) const;
    std::string recv(size_t buffer_size = 4096) const;

    static std::string getClientAddress(const struct sockaddr_in& client_address);  
    static int getClientPort(const struct sockaddr_in& client_address);
    
    int getFileDescriptor() const noexcept {return m_fd; };
    bool isValid() const noexcept {return m_fd > 0; };
    void close();

    void setReuseAddress(bool enable = true);
    void setNonBlocking(bool enable = true);
    void setReceiveTimeout(int seconds, int microseconds = 0);
    
    ~Socket();


private:
    int m_fd;
    Type m_type;

    void createSocket();
    void checkDescriptor() const;
    
};






#endif