#include "metric_accumulator_impl/sum_average_accumulator.hpp"  // интерфейс класса аккумулятора, вычисляющего сумму и среднее арифметическое значений метрики

#include <unistd.h>

namespace analyzer::metric_accumulator::metric_accumulator_impl {

// Метод, накапливающий очередное значение метрики
void SumAverageAccumulator::Accumulate(const metric::MetricResult &metric_result) {
    // Проверяем, что аккумулятор еще не финализирован
    if (is_finalized) {
        throw std::runtime_error("SumAverageAccumulator::Accumulate called after Finalize");
    }

    // Добавляем значение к сумме и увеличиваем счетчик
    sum += std::get<int>(metric_result.value);
    count++;
}

// Метод, финализирующий накопление (вычисляет среднее)
void SumAverageAccumulator::Finalize() {
    average = (count == 0) ? 0.0 : (static_cast<double>(sum) / count);
    is_finalized = true;
}

// Метод, сбрасывающий состояние аккумулятора
void SumAverageAccumulator::Reset() {
    is_finalized = false;
    sum = 0;
    count = 0;
    average = 0;
}

// Метод, возвращающий вычисленные сумму и среднее
SumAverageAccumulator::SumAverage SumAverageAccumulator::Get() const {
    if (!is_finalized) {
        throw std::runtime_error("CategoricalAccumulator::Get() called before Finalize()");
    }
    return {sum, average};
}
}  // namespace analyzer::metric_accumulator::metric_accumulator_impl
