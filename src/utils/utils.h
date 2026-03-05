#pragma once

#include <string>
#include <vector>
#include <cstdint>

/// @brief Вспомогательные утилиты приложения.
namespace utils {

/// @brief Форматировать время работы системы в читаемый вид.
///
/// Пример: 25.5 часа → "1d 1h 30m"
/// @param hours Время работы в часах (>= 0).
/// @return Строка вида "Xd Yh Zm" (секция дней опускается при 0).
std::string formatUptime(double hours);

/// @brief Получить текущую дату и время в виде строки.
/// @return Строка формата "YYYY-MM-DD HH:MM:SS" по местному времени.
std::string formatTimestamp();

/// @brief Разобрать беззнаковое целое из строки.
///
/// Возвращает false при наличии нецифровых символов (кроме ведущих/завершающих пробелов),
/// при отрицательных значениях и при переполнении.
/// @param      str    Входная строка.
/// @param[out] result Результат разбора при успехе.
/// @return true при успешном разборе, false иначе.
bool parseUint64(const std::string& str, uint64_t& result);

/// @brief Разобрать число с плавающей точкой из строки.
/// @param      str    Входная строка.
/// @param[out] result Результат разбора при успехе.
/// @return true при успешном разборе, false иначе.
bool parseDouble(const std::string& str, double& result);

/// @brief Разбить строку по разделителю.
///
/// Пустые токены сохраняются (при множественных разделителях подряд).
/// @param str       Входная строка.
/// @param delimiter Символ-разделитель.
/// @return Вектор подстрок (может содержать пустые элементы).
std::vector<std::string> split(const std::string& str, char delimiter);

/// @brief Удалить ведущие и завершающие пробельные символы.
/// @param str Входная строка.
/// @return Строка без пробелов по краям.
std::string trim(const std::string& str);

/// @brief Разобрать аргументы командной строки.
///
/// Поддерживаемые опции:
/// - (без аргументов) — однократный запуск;
/// - `-c` / `--continuous [N]` — непрерывный режим с интервалом N секунд;
/// - `-h` / `--help` — вывод справки.
///
/// @param      argc            Количество аргументов.
/// @param      argv            Массив аргументов.
/// @param[out] continuous_mode true, если запрошен непрерывный режим.
/// @param[out] interval        Интервал в секундах (по умолчанию 2).
/// @return true при успешном разборе, false при ошибке (сообщение выведено в stderr).
bool parseArguments(int argc, char* argv[], bool& continuous_mode, int& interval);

} // namespace utils
