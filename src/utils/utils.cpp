#include <sstream>
#include <cctype>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "utils.h"

namespace utils {

std::string formatUptime(double hours) {
    const int days             = static_cast<int>(hours) / 24;
    const int remaining_hours  = static_cast<int>(hours) % 24;
    const int minutes          = static_cast<int>((hours - static_cast<int>(hours)) * 60);

    std::ostringstream ss;
    if (days > 0) {
        ss << days << "d ";
    }
    ss << remaining_hours << "h " << minutes << "m";
    return ss.str();
}

std::string formatTimestamp() {
    const auto now        = std::chrono::system_clock::now();
    const auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool parseUint64(const std::string& str, uint64_t& result) {
    if (str.empty()) {
        return false;
    }

    // Пропускаем ведущие пробелы
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }

    // Первый непробельный символ обязан быть цифрой
    if (start >= str.size() || !std::isdigit(static_cast<unsigned char>(str[start]))) {
        return false;
    }

    // Считываем цифры
    size_t pos = start;
    while (pos < str.size() && std::isdigit(static_cast<unsigned char>(str[pos]))) {
        ++pos;
    }

    // После цифр допускаются только пробелы
    for (size_t i = pos; i < str.size(); ++i) {
        if (!std::isspace(static_cast<unsigned char>(str[i]))) {
            return false;
        }
    }

    try {
        result = std::stoull(str.substr(start, pos - start));
        return true;
    } catch (...) {
        return false;
    }
}

bool parseDouble(const std::string& str, double& result) {
    try {
        result = std::stod(str);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(str);
    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string trim(const std::string& str) {
    const auto begin = std::find_if_not(str.begin(), str.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    const auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) {
        return std::isspace(c);
    }).base();
    return (begin < end) ? std::string(begin, end) : std::string();
}

bool parseArguments(int argc, char* argv[], bool& continuous_mode, int& interval) {
    continuous_mode = false;
    interval        = 2;

    if (argc < 2) {
        return true;
    }

    const std::string arg1 = argv[1];

    if (arg1 == "--help" || arg1 == "-h") {
        std::cout << "Usage: system_monitor [OPTIONS]\n"
                  << "Monitor system resources\n\n"
                  << "Options:\n"
                  << "  -c, --continuous [N]  Run in continuous mode with N second interval (default: 2)\n"
                  << "  -h, --help            Show this help message\n";
        return true;
    }

    if (arg1 == "--continuous" || arg1 == "-c") {
        continuous_mode = true;

        if (argc > 2) {
            double interval_double = 0.0;
            if (!parseDouble(argv[2], interval_double)) {
                std::cerr << "Error: Invalid interval value\n";
                return false;
            }
            if (interval_double <= 0.0) {
                std::cerr << "Error: Interval must be positive\n";
                return false;
            }
            interval = static_cast<int>(interval_double);
        }
        return true;
    }

    std::cerr << "Error: Unknown option: " << arg1 << "\n";
    return false;
}

} // namespace utils
