#include "metric.hpp"

#include <unistd.h>

#include <algorithm>
#include <any>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "function.hpp"

namespace analyzer::metric {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

// Метод, регистрирующий новую метрику (добавляет ее в хранилище метрик)
void MetricExtractor::RegisterMetric(std::unique_ptr<IMetric> metric) { metrics_.push_back(std::move(metric)); }

/**
 * @brief Вычисляет все зарегистрированные метрики для заданной функции.
 *
 * Эта функция применяет каждый метрический объект из контейнера `metrics`
 * к переданной функции `func` и собирает результаты в вектор.
 */
MetricResults MetricExtractor::Get(const function::Function &func) const {
    // Создаем отображение, преобразующее каждый элемент массива metrics_ (unique_ptr<IMetric>) в MetricResult,
    // и вставляем полученные структуры в вектор результатов.
    // При возврате вектора результатов работает RVO-оптимизация компилятора
    return MetricResults(metrics_ |
                         rv::transform([&func](const auto &metric_ptr) { return metric_ptr->Calculate(func); }) |
                         rs::to<std::vector>()  // поддерживается в C++23
    );
}
}  // namespace analyzer::metric
