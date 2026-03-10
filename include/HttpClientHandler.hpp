#ifndef HTTPCLIENTHANDLER_HPP
#define HTTPCLIENTHANDLER_HPP

#include "Socket.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <string>
#include <filesystem>


class HttpClientHandler {
    private:
        std::string m_root_directory; 
    public:
    explicit HttpClientHandler(const std::string& root_dir);

    void handleClient(Socket client_socket);

    private:
        HttpRequest receiveRequest(Socket& client_socket) const;
        void sendResponse(Socket& client_socket, const HttpResponse& response) const;
        HttpResponse processRequest(const HttpRequest& request) const;

        HttpResponse handleGet(const HttpRequest& request) const;
        HttpResponse handleHead(const HttpRequest& request) const;

        std::string resolvePath(const std::string& request_path) const;
        bool isPathSafe(const std::string& path) const;
        std::string getMimeType(const std::string& filePath) const;

};


#endif