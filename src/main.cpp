#include <csignal>
#include <unistd.h>

#include "system_monitor.h"
#include "utils.h"

/// @brief Глобальный указатель на монитор для доступа из обработчика сигнала.
static SystemMonitor* g_monitor = nullptr;

/// @brief Обработчик сигналов SIGINT и SIGTERM.
///
/// Использует только async-signal-safe функции (write, stop через атомарный флаг).
/// @param signum Номер полученного сигнала.
static void signalHandler(int signum) {
    // write() является async-signal-safe в отличие от std::cout
    const char msg[] = "\nReceived signal, shutting down...\n";
    if (write(STDOUT_FILENO, msg, sizeof(msg) - 1) < 0) { /* async-signal-safe, ошибку игнорируем */ }
    (void)signum;
    if (g_monitor) {
        g_monitor->stop();
    }
}

int main(int argc, char* argv[]) {
    bool continuous_mode = false;
    int  interval        = 2;

    if (!utils::parseArguments(argc, argv, continuous_mode, interval)) {
        return 1;
    }

    SystemMonitor monitor;
    g_monitor = &monitor;

    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);

    if (continuous_mode) {
        monitor.runContinuousMonitoring(interval);
    } else {
        const SystemInfo info = monitor.getCurrentInfo();
        monitor.printInfo(info);
    }

    return 0;
}
