#pragma once
#include <unistd.h>

#include "metric_accumulator.hpp"

namespace analyzer::metric_accumulator::metric_accumulator_impl {

// Аккумулятор, вычисляющий сумму и среднее арифметическое значений метрики
// (например, суммарная и средняя цикломатическая сложность)
class SumAverageAccumulator : public IAccumulator {
public:
    // Вложенная структура для хранения пары результатов (сумма и среднее)
    struct SumAverage {
        int sum;         // сумма всех накопленных значений
        double average;  // среднее арифметическое

        // Автоматическая генерация операторов сравнения (spaceship operator, C++20)
        auto operator<=>(const SumAverage &) const = default;
    };

    // Метод, накапливающий очередное значение метрики
    void Accumulate(const metric::MetricResult &metric_result) override;

    // Метод, финализирующий накопление (вычисляет среднее)
    virtual void Finalize() override;

    // Метод, сбрасывающий состояние аккумулятора
    virtual void Reset() override;

    // Метод, возвращающий вычисленные сумму и среднее
    // (должен вызываться после Finalize)
    SumAverage Get() const;

private:
    int sum = 0;         // сумма всех накопленных значений
    int count = 0;       // количество накопленных значений
    double average = 0;  // вычисленное среднее (после Finalize)
};

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
