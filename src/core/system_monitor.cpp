#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <climits>
#include <unistd.h>
#include <thread>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255  // POSIX минимум; Linux определяет это в <limits.h>
#endif

#include "system_monitor.h"
#include "utils.h"

SystemMonitor::SystemMonitor()
    : last_cpu_read_(std::chrono::steady_clock::now())
    , last_idle_time_(0)
    , last_total_time_(0)
    , running_(true) {
    // Первый вызов устанавливает базовые значения для последующего дифференциала
    calculateCpuUsage();
}

SystemMonitor::~SystemMonitor() {
    stop();
}

void SystemMonitor::stop() {
    running_ = false;
}

std::string SystemMonitor::readFile(const std::string& path) {
    // Определяем префикс один раз: если примонтирован /host/proc, используем его
    static const std::string proc_prefix = []() -> std::string {
        if (access("/host/proc/stat", F_OK) == 0) {
            return "/host";
        }
        return "";
    }();

    std::ifstream file(proc_prefix + path);
    if (!file.is_open()) {
        return "";
    }
    std::ostringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

std::string SystemMonitor::readHostname() {
    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        hostname[HOST_NAME_MAX] = '\0';
        return std::string(hostname);
    }
    return "unknown";
}

double SystemMonitor::readUptime() {
    const std::string content = readFile("/proc/uptime");
    if (content.empty()) {
        return 0.0;
    }
    double uptime_seconds = 0.0;
    std::istringstream(content) >> uptime_seconds;
    return uptime_seconds / 3600.0;
}

uint64_t SystemMonitor::readProcStat(uint64_t& idle) {
    const std::string content = readFile("/proc/stat");
    if (content.empty()) {
        idle = 0;
        return 0;
    }

    std::istringstream iss(content);
    std::string cpu;
    uint64_t user = 0, nice = 0, system = 0, idle_time = 0,
             iowait = 0, irq = 0, softirq = 0, steal = 0;

    iss >> cpu >> user >> nice >> system >> idle_time >> iowait >> irq >> softirq >> steal;

    if (cpu != "cpu") {
        idle = 0;
        return 0;
    }

    idle = idle_time + iowait;
    return user + nice + system + idle_time + iowait + irq + softirq + steal;
}

double SystemMonitor::calculateCpuUsage() {
    uint64_t idle  = 0;
    uint64_t total = readProcStat(idle);

    // Первый вызов — сохраняем базовые значения, реальный процент недоступен
    if (last_total_time_ == 0) {
        last_total_time_ = total;
        last_idle_time_  = idle;
        last_cpu_read_   = std::chrono::steady_clock::now();
        return 0.0;
    }

    // Защита от переполнения счётчиков ядра (крайне редко, но возможно)
    if (total < last_total_time_ || idle < last_idle_time_) {
        last_total_time_ = total;
        last_idle_time_  = idle;
        last_cpu_read_   = std::chrono::steady_clock::now();
        return 0.0;
    }

    const uint64_t total_diff = total - last_total_time_;
    const uint64_t idle_diff  = idle  - last_idle_time_;

    last_total_time_ = total;
    last_idle_time_  = idle;
    last_cpu_read_   = std::chrono::steady_clock::now();

    if (total_diff == 0) {
        return 0.0;
    }

    return static_cast<double>(total_diff - idle_diff) / static_cast<double>(total_diff) * 100.0;
}

void SystemMonitor::readMemoryInfo(uint64_t& total, uint64_t& free, uint64_t& available) {
    total = free = available = 0;

    const std::string content = readFile("/proc/meminfo");
    if (content.empty()) {
        return;
    }

    std::istringstream iss(content);
    std::string line;

    while (std::getline(iss, line)) {
        // Разбиваем строку и отбрасываем пустые токены (множественные пробелы)
        const std::vector<std::string> parts = utils::split(line, ' ');

        std::string key;
        uint64_t    value = 0;
        bool        found_value = false;

        for (const auto& token : parts) {
            if (token.empty()) {
                continue;
            }
            if (key.empty()) {
                key = token;
                // Убираем двоеточие в конце ключа
                if (!key.empty() && key.back() == ':') {
                    key.pop_back();
                }
            } else if (!found_value && utils::parseUint64(token, value)) {
                found_value = true;
            }
        }

        if (!found_value) {
            continue;
        }

        if      (key == "MemTotal")     { total     = value; }
        else if (key == "MemFree")      { free      = value; }
        else if (key == "MemAvailable") { available = value; }
    }
}

SystemInfo SystemMonitor::getCurrentInfo() {
    SystemInfo info;
    info.hostname         = readHostname();
    info.uptime_hours     = readUptime();
    info.cpu_usage_percent = calculateCpuUsage();

    readMemoryInfo(info.total_memory_kb, info.free_memory_kb, info.available_memory_kb);

    info.memory_usage_percent = (info.total_memory_kb > 0)
        ? 100.0 * static_cast<double>(info.total_memory_kb - info.available_memory_kb)
              / static_cast<double>(info.total_memory_kb)
        : 0.0;

    return info;
}

std::string SystemInfo::toString() const {
    std::ostringstream ss;
    ss << "=== System Information ===\n";
    ss << "Timestamp:      " << utils::formatTimestamp()                                     << "\n";
    ss << "Hostname:       " << hostname                                                      << "\n";
    ss << "Uptime:         " << utils::formatUptime(uptime_hours)                            << "\n";
    ss << "CPU Usage:      " << std::fixed << std::setprecision(1) << cpu_usage_percent      << "%\n";
    ss << "Memory Usage:   " << std::fixed << std::setprecision(1) << memory_usage_percent   << "% "
       << "(" << (total_memory_kb - available_memory_kb) / 1024 << " MB"
       << " / " << total_memory_kb / 1024 << " MB)\n";
    ss << "==========================";
    return ss.str();
}

void SystemMonitor::printInfo(const SystemInfo& info) {
    std::cout << info.toString() << "\n";
}

void SystemMonitor::runContinuousMonitoring(int interval_seconds) {
    std::cout << "Starting continuous monitoring (Ctrl+C to stop)...\n";

    while (running_) {
        const SystemInfo info = getCurrentInfo();
        printInfo(info);
        std::cout << "\n";

        for (int i = 0; i < interval_seconds && running_; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    std::cout << "Monitoring stopped.\n";
}
