#ifndef ENV_UTILS_HPP
#define ENV_UTILS_HPP

#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <charconv>
#include <sstream>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <spdlog/spdlog.h>

namespace Env
{

// Получение сырого значения переменной окружения
std::optional<std::string> getRaw(const std::string& name);

// Строка
std::optional<std::string> parse(const std::string& value);

// Целые числа
template<typename T>
typename std::enable_if<std::is_integral<T>::value, std::optional<T>>::type
parse(const std::string& value)
{
    T result{};
    auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result);
    if (ec == std::errc{} && ptr == value.data() + value.size())
    {
        return result;
    }
    spdlog::warn("Failed to parse integer from '{}'", value);
    return std::nullopt;
}

// Вектор строк (список через разделитель)
std::optional<std::vector<std::string>> parseList(const std::string& value, char delimiter = ',');

// Чтение переменной окружения с парсингом и значением по умолчанию
template<typename T>
T get(const std::string& name, const T& defaultValue)
{
    auto raw = getRaw(name);
    if (!raw)
    {
        spdlog::debug("Env var '{}' not set, using default", name);
        return defaultValue;
    }

    if constexpr (std::is_same_v<T, std::vector<std::string>>)
    {
        auto parsed = parseList(*raw);
        if (parsed) return *parsed;
        spdlog::warn("Failed to parse list from '{}', using default", name);
        return defaultValue;
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
        return *raw;  // raw уже std::string
    }
    else
    {
        auto parsed = parse<T>(*raw);
        if (parsed) return *parsed;
        spdlog::warn("Failed to parse '{}' from env var '{}', using default", 
                    *raw, name);
        return defaultValue;
    }
}

// Версия с обязательной переменной (бросает исключение при ошибке)
template<typename T>
T getRequired(const std::string& name)
{
    auto raw = getRaw(name);
    if (!raw)
    {
        throw std::runtime_error("Required environment variable '" + name + "' is not set");
    }

    if constexpr (std::is_same_v<T, std::vector<std::string>>)
    {
        auto parsed = parseList(*raw);
        if (parsed) return *parsed;
        throw std::runtime_error("Failed to parse list from environment variable '" + name + "'");
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
        return *raw;  // raw уже std::string
    }
    else
    {
        auto parsed = parse<T>(*raw);
        if (parsed) return *parsed;
        throw std::runtime_error("Failed to parse value from environment variable '" + name + "': '" + *raw + "'");
    }
}

}

#endif // ENV_UTILS_HPP
