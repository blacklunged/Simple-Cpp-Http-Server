#include "../include/HttpRequest.hpp"
#include "../include/Logger.hpp"
#include <sstream>
#include <algorithm>
#include <vector>


static std::string trim(const std::string& str){
    if (str.empty()) return "";

    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);

}

static std::string toLowerCase(const std::string& str){
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){ return std::tolower(c); });

    return result;
}


HttpRequest HttpRequest::parse(const std::string& raw_request) {
    HttpRequest request;
    std::istringstream request_stream(raw_request);
    std::string line;

    if (std::getline(request_stream, line)){
        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path >> request.version;
    }

    if(!request.version.empty() && request.version.back() == '\r'){
        request.version.pop_back();
    }

    std::transform(request.method.begin(), request.method.end(), request.method.end(),
                     [](unsigned char c) { return std::tolower(c); });

    while(std::getline(request_stream, line)) {
        if (line.empty() || line == "\r") {
            break;
        }

        if(!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            continue;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos){
            std::string header_name = line.substr(0, colon_pos);
            std::string header_value = line.substr(colon_pos + 1);


            header_name = trim(header_name);
            header_value = trim(header_value);

            if (!header_name.empty()) {
                request.headers[toLowerCase(header_name)] = header_value;
            }
        
        }
    }

    auto content_length_it = request.headers.find("content-length");
    if(content_length_it != request.headers.end()) {
        try{
            size_t content_length = std::stoul(content_length_it->second);
            if (content_length > 0) {
                request.body.resize(content_length);
                request_stream.read(&request.body[0], content_length);
            }
        }
        catch(const std::exception& e) {
            Logger::logError(std::string("error while parsing request: ") + e.what());
        }
    }

    return request;
}


bool HttpRequest::isValid() const {
    if (method.empty() || path.empty() || version.empty()) {
        return false;
    }

    if (method != "GET" && method != "HEAD" && 
        method != "POST" && method != "PUT" && 
        method != "DELETE" && method != "OPTIONS") {
        return false;
    }

    if (version != "HTTP/1.0" && version != "HTTP/1.1"){
        return false;
    }

    if (path[0] != '/') {
        return false;
    }
    if (version == "HTTP/1.1") {
        if (headers.find("host") == headers.end()) {
            return false;
        }
    }
    return true;
}
    
std::string HttpRequest::getHeader(const std::string& name) const {
    auto it = headers.find(toLowerCase(name));
    if(it != headers.end()) {
        return it->second;
    }
    
    return "";
}