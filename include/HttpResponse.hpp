#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <unordered_map>


class HttpResponse{
    public:
        int status_code;
        std::string status_text;
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        HttpResponse(int code, const std::string& text);

        static HttpResponse ok(const std::string& content = "", const std::string& content_type = "text/plain" );
        static HttpResponse forbidden();
        static HttpResponse notFound();
        static HttpResponse badRequest();
}; 
#endif