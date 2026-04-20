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

// Класс, отвечающий за подсчет метрики "Количество параметров функции"
class CountParametersMetric final : public IMetric {
public:
    // Статическая константа с названием метрики (общая для всех экземпляров)
    static inline const std::string kName = "Parameters count";

protected:
    // Метод, возвращающий название метрики
    std::string Name() const override;

    // Метод, реализующий вычисление метрики (принимает функцию, возвращает количество ее параметров)
    MetricResult::ValueType CalculateImpl(const function::Function &f) const override;
};

}  // namespace analyzer::metric::metric_impl
