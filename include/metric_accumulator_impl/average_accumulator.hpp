#pragma once
#include <unistd.h>

#include "metric_accumulator.hpp"

namespace analyzer::metric_accumulator::metric_accumulator_impl {

// Аккумулятор, вычисляющий среднее арифметическое значений метрики
// (например, среднее количество параметров на функцию)
class AverageAccumulator : public IAccumulator {
public:
    // Метод, накапливающий очередное значение метрики
    void Accumulate(const metric::MetricResult &metric_result) override;

    // Метод, финализирующий накопление (вычисляет среднее)
    void Finalize() override;

    // Метод, сбрасывающий состояние аккумулятора
    void Reset() override;

    // Метод, возвращающий вычисленное среднее значение
    // (должен вызываться после Finalize)
    double Get() const;

private:
    int sum = 0;         // сумма всех накопленных значений
    int count = 0;       // количество накопленных значений
    double average = 0;  // вычисленное среднее (после Finalize)
};

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
