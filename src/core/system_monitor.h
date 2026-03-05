#pragma once

#include <string>
#include <chrono>
#include <atomic>
#include <cstdint>

/// @brief Снимок состояния системы в момент времени.
struct SystemInfo {
    std::string hostname;             ///< Имя хоста
    double      uptime_hours;         ///< Время работы системы в часах
    double      cpu_usage_percent;    ///< Загрузка процессора, %
    double      memory_usage_percent; ///< Использование памяти, %
    uint64_t    total_memory_kb;      ///< Общий объём RAM, КБ
    uint64_t    free_memory_kb;       ///< Свободная физическая память, КБ
    uint64_t    available_memory_kb;  ///< Доступная память (включая кэш), КБ

    /// @brief Сформировать строковое представление всех полей.
    /// @return Отформатированная многострочная строка.
    std::string toString() const;
};

/// @brief Монитор системных ресурсов Linux.
///
/// Читает данные из файловой системы /proc и предоставляет информацию
/// о загрузке процессора, использовании памяти и времени работы системы.
/// Поддерживает запуск внутри Docker-контейнера с примонтированным /host/proc.
///
/// Копирование запрещено — объект владеет внутренним состоянием CPU-дифференциала.
class SystemMonitor {
public:
    /// @brief Конструктор. Выполняет первичное чтение /proc/stat для последующего дифференциала.
    SystemMonitor();

    /// @brief Деструктор. Останавливает мониторинг, если он запущен.
    ~SystemMonitor();

    SystemMonitor(const SystemMonitor&)            = delete;
    SystemMonitor& operator=(const SystemMonitor&) = delete;

    /// @brief Получить текущий снимок состояния системы.
    /// @return Заполненная структура SystemInfo.
    SystemInfo getCurrentInfo();

    /// @brief Вывести информацию о системе в стандартный поток вывода.
    /// @param info Снимок состояния системы для отображения.
    void printInfo(const SystemInfo& info);

    /// @brief Запустить непрерывный мониторинг с заданным интервалом.
    ///
    /// Блокирует вызывающий поток до вызова stop() или получения сигнала.
    /// @param interval_seconds Интервал между измерениями в секундах (> 0).
    void runContinuousMonitoring(int interval_seconds);

    /// @brief Остановить непрерывный мониторинг.
    ///
    /// Метод безопасен для вызова из обработчика сигнала.
    void stop();

private:
    std::chrono::steady_clock::time_point last_cpu_read_;       ///< Момент последнего чтения /proc/stat
    uint64_t                              last_idle_time_;      ///< Суммарное время простоя при последнем чтении
    uint64_t                              last_total_time_;     ///< Суммарное процессорное время при последнем чтении
    std::atomic<bool>                     running_;             ///< Флаг работы цикла мониторинга

    /// @brief Прочитать содержимое файла, учитывая возможный префикс /host.
    /// @param path Путь к файлу относительно корня (например, "/proc/uptime").
    /// @return Содержимое файла или пустая строка при ошибке.
    std::string readFile(const std::string& path);

    /// @brief Получить имя хоста через gethostname().
    /// @return Имя хоста или "unknown" при ошибке.
    std::string readHostname();

    /// @brief Прочитать аптайм из /proc/uptime.
    /// @return Время работы системы в часах.
    double readUptime();

    /// @brief Вычислить загрузку CPU как дифференциал с предыдущим измерением.
    /// @return Процент загрузки CPU [0.0, 100.0], 0.0 при первом вызове.
    double calculateCpuUsage();

    /// @brief Прочитать суммарное и idle-время из /proc/stat.
    /// @param[out] idle Суммарное время простоя (idle + iowait).
    /// @return Суммарное время всех состояний CPU, 0 при ошибке чтения.
    uint64_t readProcStat(uint64_t& idle);

    /// @brief Прочитать информацию о памяти из /proc/meminfo.
    /// @param[out] total     Общий объём RAM, КБ.
    /// @param[out] free      Свободная физическая память, КБ.
    /// @param[out] available Доступная память (включая кэш), КБ.
    void readMemoryInfo(uint64_t& total, uint64_t& free, uint64_t& available);
};
