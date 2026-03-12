#pragma once

#include <algorithm>
#include <filesystem>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"

namespace analyzer::metric::metric_impl {

namespace fs = std::filesystem;

// Конфигурация: путь к тестовым файлам относительно текущей директории
// Тесты запускаются из build/src/metric_impl/tests/, поэтому поднимаемся на нужный уровень
inline const std::string test_sub_dir = "src/metric_impl";

// Шаблонная функция-хелпер для вычисления любой метрики
template <typename MetricType>
metric::MetricResult::ValueType CalcMetric(const std::string &filename, const std::string &func_name) {
    fs::path filepath = fs::current_path() / test_sub_dir / filename;  // путь к тестовому Python-файлу

    // Проверка наличия тестового Python-файла с заданным именем
    if (!fs::exists(filepath)) {
        throw std::runtime_error("Test file not found: " + filepath.string());
    }

    // Пытаемся обработать тестовый Python-файл
    try {
        // Преобразуем Python-файл в AST-дерево (объект File) с помощью tree-sitter
        file::File file(filepath.string());

        // Извлекаем все функции из AST-дерева
        function::FunctionExtractor extractor;
        auto functions = extractor.Get(file);

        // Ищем функцию с заданным именем
        auto it = std::ranges::find_if(functions, [&func_name](const auto &f) { return f.name == func_name; });
        if (it == functions.end()) {
            throw std::runtime_error("Function " + func_name + " not found in " + filename);
        }

        // Вычисляем метрику для заданной функции
        MetricType metric;
        auto result = metric.Calculate(*it);

        // Возвращаем полученное значение метрики
        return result.value;

    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Error processing ") + filename + ": " + e.what());
    }
}

}  // namespace analyzer::metric::metric_impl