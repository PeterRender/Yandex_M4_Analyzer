#include "metric_impl/naming_style.hpp"  // интерфейс класса метрики "Определение стиля имени функции"

#include <unistd.h>

#include <algorithm>
#include <string>

namespace analyzer::metric::metric_impl {

// Метод, возвращающий название метрики
std::string NamingStyleMetric::Name() const { return kName; }

// Метод, реализующий вычисление метрики (принимает функцию, возвращает тип ее имени)
MetricResult::ValueType NamingStyleMetric::CalculateImpl(const function::Function &f) const {
    auto &functionName = f.name;

    // Если есть дефис в имени функции - стиль Unknown
    if (functionName.find('-') != std::string::npos) {
        return std::string("Unknown");
    }

    // Проверяем наличие подчеркиваний в имени функции
    bool hasUnderscore = functionName.find('_') != std::string::npos;

    // Проверяем наличие заглавных букв (хотя бы одна)
    bool hasUpper = std::any_of(functionName.begin(), functionName.end(), [](char c) { return isupper(c); });

    // Проверяем соответствие стилю snake_case
    if (hasUnderscore) {
        // Должны быть только строчные буквы и подчеркивания
        bool allLower = std::all_of(functionName.begin(), functionName.end(),
                                    [](char c) { return islower(c) || c == '_' || isdigit(c); });

        return (allLower) ? std::string("Snake Case") : std::string("Unknown");
    }

    // Проверяем соответствие стилям PascalCase/CamelCase
    if (hasUpper) {
        // Если первая буква заглавная, то это стиль PascalCase, иначе - это стиль camelCase
        return isupper(functionName[0]) ? std::string("Pascal Case") : std::string("Camel Case");
    }

    // Все символы строчные без разделителей
    return std::string("Lower Case");
}

}  // namespace analyzer::metric::metric_impl
