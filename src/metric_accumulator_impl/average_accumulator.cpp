#include "metric_accumulator_impl/average_accumulator.hpp"

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
void AverageAccumulator::Accumulate(const metric::MetricResult &metric_result) {
    // Проверяем, что аккумулятор еще не финализирован
    if (is_finalized) {
        throw std::runtime_error("AverageAccumulator::Accumulate called after Finalize");
    }

    // Добавляем значение к сумме и увеличиваем счетчик
    // sum += std::get<int>(metric_result.value);
    // ВРЕМЕННО: используем value напрямую (это int)
    sum += metric_result.value;
    count++;
}

// Метод, финализирующий накопление (вычисляет среднее)
void AverageAccumulator::Finalize() {
    average = (count == 0) ? 0.0 : (static_cast<double>(sum) / count);
    is_finalized = true;
}

// Метод, сбрасывающий состояние аккумулятора
void AverageAccumulator::Reset() {
    is_finalized = false;
    sum = 0;
    count = 0;
    average = 0;
}

// Метод, возвращающий вычисленное среднее значение
double AverageAccumulator::Get() const {
    if (!is_finalized) {
        throw std::runtime_error("AverageAccumulator::Get() called before Finalize()");
    }
    return average;
}
}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
