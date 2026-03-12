#include "metric_accumulator_impl/average_accumulator.hpp"  // интерфейс класса аккумулятора, вычисляющего среднее арифметическое значений метрики

#include <gtest/gtest.h>

namespace analyzer::metric_accumulator::metric_accumulator_impl::test {

using namespace analyzer::metric;

// Тест 1: Аккумулятор без данных
TEST(AvgAccTest, Empty) {
    AverageAccumulator acc;
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 0.0);
}

// Тест 2: Аккумулятор с одним значением
TEST(AvgAccTest, SingleValue) {
    AverageAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 7});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 7.0);
}

// Тест 3: Аккумулятор с несколькими значениями
TEST(AvgAccTest, MultiValues) {
    AverageAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 1});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 2});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 3});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 2.0);  // (1+2+3)/3 = 2
}

// Тест 4: Аккумулятор с повторным использованием после Reset
TEST(AvgAccTest, Reuse) {
    AverageAccumulator acc;

    // Первое использование (дробный результат)
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 1});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 2});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 1.5);

    // Сброс и второе использование
    acc.Reset();
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 10});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 20});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 30});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 20.0);  // (10+20+30)/3 = 20
}

// Тест 5: Проверка исключения при Get до Finalize
TEST(AvgAccTest, GetBeforeFinalizeThrows) {
    AverageAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 7});
    EXPECT_THROW(acc.Get(), std::runtime_error);
}

// Тест 6: Проверка исключения при Accumulate после Finalize
TEST(AvgAccTest, AccumulateAfterFinalizeThrows) {
    AverageAccumulator acc;
    acc.Finalize();
    EXPECT_THROW(acc.Accumulate(MetricResult{.metric_name = "test", .value = 7}), std::runtime_error);
}

// Тест 7: Большое количество значений
TEST(AvgAccTest, LotsOfValues) {
    AverageAccumulator acc;
    for (int i = 1; i <= 1000; ++i) {
        acc.Accumulate(MetricResult{.metric_name = "test", .value = i});
    }
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 500.5);  // среднее от 1 до 1000 = 500.5
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
