#include "env_utils.hpp"

namespace Env
{

// Получение сырого значения переменной окружения
std::optional<std::string> getRaw(const std::string& name)
{
    if (const char* val = std::getenv(name.c_str()))
    {
        return std::string(val);
    }
    return std::nullopt;
}

std::string trim(std::string_view s)
{
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return std::string(s.substr(start, end - start + 1));
}

// Строка
std::optional<std::string> parse(const std::string& value)
{
    return value;
}

// Вектор строк (список через разделитель)
std::optional<std::vector<std::string>> parseList(const std::string& value, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, delimiter))
    {
        auto trimmed = trim(item);
        if (!trimmed.empty())
        {
            result.push_back(std::move(trimmed));
        }
    }
    return result;
}

} // namespace Env
