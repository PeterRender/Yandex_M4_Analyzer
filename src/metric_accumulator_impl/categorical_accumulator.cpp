#include "metric_accumulator_impl/categorical_accumulator.hpp"

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

namespace analyzer::metric_accumulator::metric_accumulator_impl {

// Метод, накапливающий очередное значение метрики
void CategoricalAccumulator::Accumulate(const metric::MetricResult &metric_result) {
    // Проверяем, что аккумулятор еще не финализирован
    if (is_finalized) {
        throw std::runtime_error("CategoricalAccumulator::Accumulate called after Finalize");
    }
    // Увеличиваем счетчик для данного значения
    // (предполагается, что metric_result.value имеет тип std::string)
    categories_freq_[std::get<std::string>(metric_result.value)]++;
}

// Метод, финализирующий накопление
void CategoricalAccumulator::Finalize() { is_finalized = true; }

// Метод, сбрасывающий состояние аккумулятора
void CategoricalAccumulator::Reset() {
    is_finalized = false;
    categories_freq_.clear();
}

// Метод, возвращающий словарь частот встречаемости значений
const std::unordered_map<std::string, int> &CategoricalAccumulator::Get() const {
    if (!is_finalized)
        throw std::runtime_error("CategoricalAccumulator::Get() called before Finalize()");
    return categories_freq_;
}
}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
