#include "../include/HttpResponse.hpp"


HttpResponse::HttpResponse(int code, const std::string& text) 
    : status_code(code), status_text(text) {

    headers["Connection"] = "close";
}

HttpResponse HttpResponse::ok(const std::string& content, const std::string& content_type){
    HttpResponse response(200, "OK");
    response.body = content;
    response.headers["Content-type"] = content_type;
    response.headers["Content-Length"] = std::to_string(content.size());
    return response;
}


HttpResponse HttpResponse::forbidden(){
    HttpResponse response(403, "Forbidden");
    std::string content = "<html><body><h1>403 Forbidden</h1></body></html>";
    response.body = content;
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = std::to_string(content.size());
    return response;
}

HttpResponse HttpResponse::notFound(){
    HttpResponse response(404, "Not Found");
    std::string content = "<html><body><h1>404 Not Found</h1></body></html>";
    response.body = content;
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = std::to_string(content.size());
    return response;
}


HttpResponse HttpResponse::badRequest(){
    HttpResponse response(400, "Bad Request");
    std::string content = "<html><body><h1>400 Bad Request</h1></body></html>";
    response.body = content;
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = std::to_string(content.size());
    return response;
}