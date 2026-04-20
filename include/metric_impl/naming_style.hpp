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

// Класс, отвечающий за подсчет метрики "Определение стиля имени функции"
class NamingStyleMetric final : public IMetric {
public:
    // Статическая константа с названием метрики (общая для всех экземпляров)
    static inline const std::string kName = "Naming style";

protected:
    // Метод, возвращающий название метрики
    std::string Name() const override;

    // Метод, реализующий вычисление метрики
    // (принимает функцию, возвращает тип ее имени)
    MetricResult::ValueType CalculateImpl(const function::Function &f) const override;
};

}  // namespace analyzer::metric::metric_impl
