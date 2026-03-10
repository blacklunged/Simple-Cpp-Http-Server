#include "../include/HttpClientHandler.hpp"
#include "../include/Logger.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>

HttpClientHandler::HttpClientHandler(const std::string& root_dir)
    : m_root_directory(root_dir)
{
    if (!std::filesystem::exists(m_root_directory)) {
        std::filesystem::create_directories(m_root_directory);
        Logger::logInfo("Created root directory: " + m_root_directory);
    } else {
        Logger::logInfo("Using root directory: " + m_root_directory);
    }
}

void HttpClientHandler::handleClient(Socket client_socket) {

    try {
        HttpRequest request = receiveRequest(client_socket);
        Logger::logInfo(
            "Received request: " + request.method + " " + request.path 
        );

        if (request.isValid()) {
            HttpResponse response = processRequest(request);
            sendResponse(client_socket, response);

            Logger::logInfo(
                "Sent response: " + std::to_string(response.status_code) +
                " " + response.status_text + " for path " + request.path
            );
        } else {
            Logger::logError("Invalid HTTP request received");
            sendResponse(client_socket, HttpResponse::badRequest());
        }
    }
    catch (const std::exception& e) {
        try {
            std::string msg = std::string("Error handling client: ") + e.what();
            Logger::logError(msg);
            std::cerr << msg << std::endl;
        }
        catch (...) {
        }
    }
}

HttpRequest HttpClientHandler::receiveRequest(Socket& client_socket) const {
    std::string raw_request = client_socket.recv(4096);
    if (raw_request.empty()) {
        Logger::logError("Empty request or connection closed while receiving");
        throw std::runtime_error("empty request or connection closed");
    }

    return HttpRequest::parse(raw_request);
}

void HttpClientHandler::sendResponse(Socket& client_socket,
                                     const HttpResponse& response) const
{
    std::string raw_response;
    raw_response = "HTTP/1.1 " + std::to_string(response.status_code) + " "
                 + response.status_text + "\r\n";

    for (const auto& header : response.headers) {
        raw_response += header.first + ": " + header.second + "\r\n";
    }

    raw_response += "\r\n";
    if (!response.body.empty()) {
        raw_response += response.body;
    }

    try {
        client_socket.send(raw_response);
    } catch (const std::exception& e) {
        Logger::logError(std::string("Error sending response: ") + e.what());
        throw;
    }
}

HttpResponse HttpClientHandler::processRequest(const HttpRequest& request) const {
    if (request.method == "GET") {
        return handleGet(request);
    }

    if (request.method == "HEAD") {
        return handleHead(request);
    }

    Logger::logError("Unsupported HTTP method: " + request.method);
    return HttpResponse::badRequest();
}

HttpResponse HttpClientHandler::handleGet(const HttpRequest& request) const {
    std::string file_path = resolvePath(request.path);

    if (!isPathSafe(file_path)) {
        Logger::logError("Forbidden path (GET): " + file_path +
                         " for request path " + request.path);
        return HttpResponse::forbidden();
    }

    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Logger::logInfo("File not found (GET): " + file_path);
        return HttpResponse::notFound();
    }

    std::streamsize size = file.tellg();
    if (size < 0) {
        Logger::logError("Failed to get file size (GET): " + file_path);
        return HttpResponse::notFound();
    }
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(static_cast<size_t>(size));
    if (!file.read(buffer.data(), size)) {
        Logger::logError("Failed to read file (GET): " + file_path);
        return HttpResponse::notFound();
    }

    Logger::logInfo(
        "Served file (GET): " + file_path +
        " size=" + std::to_string(size) +
        " mime=" + getMimeType(file_path)
    );

    HttpResponse response = HttpResponse::ok(
        std::string(buffer.data(), buffer.size()),
        getMimeType(file_path)
    );
    return response;
}

HttpResponse HttpClientHandler::handleHead(const HttpRequest& request) const {
    std::string file_path = resolvePath(request.path);

    if (!isPathSafe(file_path)) {
        Logger::logError("Forbidden path (HEAD): " + file_path +
                         " for request path " + request.path);
        return HttpResponse::forbidden();
    }

    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Logger::logInfo("File not found (HEAD): " + file_path);
        return HttpResponse::notFound();
    }

    std::streamsize size = file.tellg();
    if (size < 0) {
        Logger::logError("Failed to get file size (HEAD): " + file_path);
        return HttpResponse::notFound();
    }
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(static_cast<size_t>(size));
    if (!file.read(buffer.data(), size)) {
        Logger::logError("Failed to read file (HEAD): " + file_path);
        return HttpResponse::notFound();
    }

    Logger::logInfo(
        "Served file headers (HEAD): " + file_path +
        " size=" + std::to_string(size) +
        " mime=" + getMimeType(file_path)
    );

    HttpResponse response = HttpResponse::ok("", getMimeType(file_path));
    response.body.clear();
    return response;
}

std::string HttpClientHandler::resolvePath(const std::string& request_path) const {
    std::string path = request_path;
    if (path == "/" || path.empty()) {
        path = "/index.html";
    }
    return m_root_directory + path;
}

bool HttpClientHandler::isPathSafe(const std::string& path) const {
    try {
        namespace fs = std::filesystem;

        fs::path root = fs::canonical(m_root_directory);
        fs::path target = fs::weakly_canonical(path);

        auto [mis_root, mis_target] = std::mismatch(
            root.begin(), root.end(),
            target.begin(), target.end()
        );

        bool safe = (mis_root == root.end());

        // Logger::logInfo(
        //     std::string("[isPathSafe] path=") + path +
        //     " root=" + root.string() +
        //     " target=" + target.string() +
        //     " safe=" + (safe ? "true" : "false")
        // );

        return safe;
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::logError(std::string("error: ") + e.what() +
                         " path=" + path);
        return false;
    }
}

std::string HttpClientHandler::getMimeType(const std::string& filePath) const {
    size_t dot_pos = filePath.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "application/octet-stream";
    }

    std::string extention = filePath.substr(dot_pos + 1);
    std::transform(extention.begin(), extention.end(),
                   extention.begin(), ::tolower);

    static const std::unordered_map<std::string, std::string> mime_types = {
        {"html", "text/html"},
        {"htm",  "text/html"},
        {"css",  "text/css"},
        {"js",   "application/javascript"},
        {"mjs",  "application/javascript"},
        {"json", "application/json"},
        {"png",  "image/png"},
        {"jpg",  "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"gif",  "image/gif"},
        {"bmp",  "image/bmp"},
        {"svg",  "image/svg+xml"},
        {"ico",  "image/x-icon"},
        {"webp", "image/webp"},
        {"ttf",  "font/ttf"},
        {"otf",  "font/otf"},
        {"woff", "font/woff"},
        {"woff2","font/woff2"},
        {"txt",  "text/plain"},
        {"csv",  "text/csv"},
        {"pdf",  "application/pdf"},
        {"zip",  "application/zip"},
        {"tar",  "application/x-tar"},
        {"gz",   "application/gzip"},
        {"xml",  "application/xml"},
        {"doc",  "application/msword"},
        {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {"",     "application/octet-stream"}
    };

    auto it = mime_types.find(extention);
    if (it != mime_types.end()) {
        return it->second;
    }

    return "application/octet-stream";
}
