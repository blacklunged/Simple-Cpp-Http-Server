#include "../include/Socket.hpp"
#include "../include/Logger.hpp"

Socket::Socket(Type type): m_fd(-1), m_type(type){
    createSocket();
}

Socket::Socket(int file_descriptor): m_fd(file_descriptor), m_type(Type::TCP){
    if (!isValid() ) {
        Logger::logError("Invalid file descriptor! " + m_fd);
        throw std::invalid_argument("ivalid file descriptor!");
    }
}

Socket::Socket(Socket&& other) 
    noexcept : m_fd(other.m_fd), m_type(other.m_type)
{
    other.m_fd = -1;
};

Socket& Socket::operator=(Socket&& other) noexcept{
    if (this != &other){
        close();

        m_fd = other.m_fd;
        m_type = other.m_type;
        other.m_fd = -1;
    }
    return  *this;

}

Socket::~Socket(){
    close();
}


void Socket::createSocket(){
    int domain = AF_INET;
    int type = (m_type == Type::TCP) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = 0;
    m_fd = socket(domain, type, protocol);

    if (!isValid()) {
        Logger::logError("Error creating socket");

        throw std::system_error(errno, std::generic_category(), "socket creation failed");
    }
    setReuseAddress(true);
}

void Socket::bind(const std::string &address, int port){
    checkDescriptor();
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0 ){
        throw std::runtime_error("Invalid IP address " + address);
    }

    if (::bind(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        throw std::system_error(errno, std::generic_category(), "bind failed");
    }
    std::cout << "Socket bound to " << address << ":" << port << std::endl;
}

void Socket::bind(int port){
    checkDescriptor();
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        throw std::system_error(errno, std::generic_category(), "bind failed");
    }
    std::cout << "Socket bound to *:" << port << std::endl;
}

void Socket::listen(int backlog){
    checkDescriptor();

    if (::listen(m_fd, backlog) < 0) {
        throw std::system_error(errno, std::generic_category(), "listen failed");   
    }
    std::cout << "Socket listening with backlog " << backlog << std::endl;
}

Socket Socket::accept(){
    checkDescriptor();
    int client_fd =  ::accept(m_fd, nullptr, nullptr); 
    if (client_fd < 0) {
        throw std::system_error(errno, std::generic_category(), "accept failed");
    }

    return Socket(client_fd);
}


Socket Socket::accept(struct sockaddr_in& client_address){
    checkDescriptor();
    socklen_t client_len = sizeof(client_address);
    int client_fd =  ::accept(m_fd, (struct sockaddr*)&client_address, &client_len); 
    if (client_fd < 0) {
        throw std::system_error(errno, std::generic_category(), "accept failed");
    }

    return Socket(client_fd);
}

ssize_t Socket::send(const std::string& data) const{
    return send(data.c_str(), data.length());
}

ssize_t Socket::send(const void* data, size_t length) const{
    checkDescriptor();

    ssize_t result = ::send(m_fd, data, length, MSG_NOSIGNAL);
    if (result < 0 ) {
        throw std::system_error(errno, std::generic_category(), "sending failed");
    }
    return result;
}

ssize_t Socket::recv(void* buffer, size_t buffer_size) const{
    checkDescriptor();
    ssize_t result = ::recv(m_fd, buffer, buffer_size, 0);
    if (result < 0) {
        throw std::system_error(errno, std::generic_category(), "failed to read data");
    }
    return result;
}

std::string Socket::recv(size_t buffer_size) const{
    checkDescriptor();
    std::vector<char> buffer(buffer_size);
    ssize_t bytes_recived = recv(buffer.data(), buffer_size);
    
    if (bytes_recived > 0){
        return std::string(buffer.data(), bytes_recived);
    }

    return "";
}


std::string Socket::getClientAddress(const struct sockaddr_in& client_address){
    char ip_str[INET_ADDRSTRLEN];
    const char* result = inet_ntop(AF_INET, &client_address.sin_addr, ip_str, INET_ADDRSTRLEN );
    if ( result != nullptr) {
        return std::string(ip_str);

    }
    else return "unknown";
}

int Socket::getClientPort(const struct sockaddr_in& client_address){
    return ntohs(client_address.sin_port);
}

void Socket::setReuseAddress(bool enable){
    checkDescriptor();
    int opt = enable ? 1 : 0;
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::system_error(errno, std::generic_category(), "setsockopt SO_REUSEADDR failed");
    }
}

void Socket::setNonBlocking(bool enable){
    checkDescriptor();
    
    int flags = fcntl(m_fd, F_GETFL, 0);
    if (flags < 0) {
        throw std::system_error(errno, std::generic_category(), "fcntl F_GETFL failed");
    }
    
    if (enable) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    if (fcntl(m_fd, F_SETFL, flags) < 0) {
        throw std::system_error(errno, std::generic_category(), "fcntl F_SETFL failed");
    }
}

void Socket::setReceiveTimeout(int seconds, int microseconds){
    checkDescriptor();
    
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = microseconds;
    
    if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw std::system_error(errno, std::generic_category(), "setsockopt SO_RCVTIMEO failed");
    }
}

void Socket::close() {
    if(m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}
void Socket::checkDescriptor() const{
    if (m_fd < 0) {
        throw std::runtime_error("invalid error");
    }
}