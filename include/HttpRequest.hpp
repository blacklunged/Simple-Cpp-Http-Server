#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <unordered_map>

class HttpRequest{
    public:
        std::string method;
        std::string path;
        std::string version;
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        static HttpRequest parse(const std::string& raw_requiest);

        bool isValid() const;

        std::string getHeader(const std::string& name) const;

};
#endif