#pragma once

#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

// Глобальная функция преобразования числа из строкового вида в целочисленный.
inline int ToInt(std::string_view value) {
    // Удаляем пробелы в начале и конце
    while (!value.empty() && std::isspace(value.front())) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(value.back())) {
        value.remove_suffix(1);
    }

    int result{};
    auto [parse_end_ptr, error_code] = std::from_chars(value.begin(), value.end(), result);
    if (error_code != std::errc{} || parse_end_ptr != value.data() + value.size()) {
        throw std::runtime_error("Can't convert '" + std::string(value) + "' to integral");
    }
    return result;
}
