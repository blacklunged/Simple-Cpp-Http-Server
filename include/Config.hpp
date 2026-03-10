#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <unordered_map>
#include <vector>

class Config {
public:
    int port = 8080;
    size_t thread_count = 4;
    std::string root_dir = "../www";
    std::string access_log = "../logs/access.log";
    std::string error_log = "../logs/error.log";

    void parse(int argc, char* argv[]);
    bool loadFromFile(const std::string& filename);  
    void printHelp(const char* program_name) const;
    void print() const;
};

#endif
