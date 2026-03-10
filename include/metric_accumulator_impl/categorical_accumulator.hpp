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

#include "metric_accumulator.hpp"

namespace analyzer::metric_accumulator::metric_accumulator_impl {

// Аккумулятор для категориальных (строковых) метрик
// Подсчитывает частоту встречаемости каждого значения (например, для метрики naming style)
class CategoricalAccumulator : public IAccumulator {
public:
    // Метод, накапливающий очередное значение метрики
    // (ожидает, что metric_result.value имеет тип std::string)
    void Accumulate(const metric::MetricResult &metric_result) override;

    // Метод, финализирующий накопление
    // (в данном случае ничего не делает, т.к. частоты уже подсчитаны)
    virtual void Finalize() override;

    // Метод, сбрасывающий состояние аккумулятора
    virtual void Reset() override;

    // Метод, возвращающий словарь частот встречаемости значений
    const std::unordered_map<std::string, int> &Get() const;

private:
    std::unordered_map<std::string, int> categories_freq_;  // словарь частот встречаемости значений
};

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
