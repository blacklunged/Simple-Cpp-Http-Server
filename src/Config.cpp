#include "../include/Config.hpp"
#include <iostream>
#include <sstream>
#include <fstream>






bool Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Config file not found: " << filename << ", using defaults\n";
        return false;
    }

    std::string line;
    std::string current_section;

    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        if (line[0] == '[') {
            size_t end = line.find(']');
            if (end != std::string::npos) {
                current_section = line.substr(1, end - 1);
            }
            continue;
        }

        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);

        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if (current_section == "server") {
            if (key == "port") port = std::stoi(value);
            else if (key == "threads") thread_count = static_cast<size_t>(std::stoi(value));
            else if (key == "root_dir") root_dir = value;
        } else if (current_section == "logging") {
            if (key == "access_log") access_log = value;
            else if (key == "error_log") error_log = value;
        }
    }

    std::cout << "✓ Config loaded from " << filename << std::endl;
    return true;
}


void Config::parse(int argc, char* argv[]) {
    std::unordered_map<std::string, std::string> args;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg.size() >= 2 && arg[0] == '-' && arg[1] != '-') {
            std::string key = arg.substr(1, 1);
            std::string value;

            if (arg.size() > 2) {
                value = arg.substr(2);
            } else if (i + 1 < argc) {
                value = argv[++i];
            } else {
            }

            args[key] = value;
        }
        else if (arg.size() > 2 && arg.substr(0, 2) == "--") {
            std::string full_key = arg.substr(2);
            std::string key, value;

            size_t eq_pos = full_key.find('=');
            if (eq_pos != std::string::npos) {
                key = full_key.substr(0, eq_pos);
                value = full_key.substr(eq_pos + 1);
            } else {
                key = full_key;
                if (i + 1 < argc) {
                    value = argv[++i]; 
                }
            }
            args[key] = value;
        }
    }

    if (args.count("p") || args.count("port")) {
        std::string val = args.count("p") ? args["p"] : args["port"];
        port = std::stoi(val);
    }

    if (args.count("t") || args.count("threads")) {
        std::string val = args.count("t") ? args["t"] : args["threads"];
        thread_count = static_cast<size_t>(std::stoi(val));
    }

    if (args.count("r") || args.count("root")) {
        root_dir = args.count("r") ? args["r"] : args["root"];
    }

    if (args.count("a") || args.count("access-log")) {
        access_log = args.count("a") ? args["a"] : args["access-log"];
    }

    if (args.count("e") || args.count("error-log")) {
        error_log = args.count("e") ? args["e"] : args["error-log"];
    }

    if (args.count("h") || args.count("help")) {
        printHelp(argv[0]);
        exit(0);
    }
}

void Config::printHelp(const char* program_name) const {
    std::cout << "Usage: " << program_name << " [options]\n\n"
              << "Options:\n"
              << "  -p, --port PORT           Server port (default: " << port << ")\n"
              << "  -t, --threads NUM         Number of worker threads (default: " << thread_count << ")\n"
              << "  -r, --root DIR            Root directory for files (default: " << root_dir << ")\n"
              << "  -a, --access-log FILE     Access log file (default: " << access_log << ")\n"
              << "  -e, --error-log FILE      Error log file (default: " << error_log << ")\n"
              << "  -h, --help                Show this help\n\n"
              << "Examples:\n"
              << "  " << program_name << "                    # default settings\n"
              << "  " << program_name << " -p 3000 -t 8      # port 3000, 8 threads\n"
              << "  " << program_name << " --root ./public   # custom root dir\n"
              << std::endl;
}

void Config::print() const {
    std::cout << "=== Server Configuration ===\n"
              << "Port: " << port << "\n"
              << "Threads: " << thread_count << "\n"
              << "Root dir: " << root_dir << "\n"
              << "Access log: " << access_log << "\n"
              << "Error log: " << error_log << "\n"
              << "===========================\n" << std::endl;
}
