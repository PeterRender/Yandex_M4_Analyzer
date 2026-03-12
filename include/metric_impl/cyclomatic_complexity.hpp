#pragma once
#include <unistd.h>

#include <algorithm>
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

#include "metric.hpp"

namespace analyzer::metric::metric_impl {

// Класс, отвечающий за подсчет метрики "Цикломатическая сложность функции"
class CyclomaticComplexityMetric final : public IMetric {
public:
    // Статическая константа с названием метрики (общая для всех экземпляров)
    static inline const std::string kName = "Cyclomatic Complexity";

protected:
    // Метод, возвращающий название метрики
    std::string Name() const override;

    // Метод, реализующий вычисление метрики
    // (принимает функцию, возвращает ее цикломатическую сложность)
    MetricResult::ValueType CalculateImpl(const function::Function &f) const override;
};

}  // namespace analyzer::metric::metric_impl
