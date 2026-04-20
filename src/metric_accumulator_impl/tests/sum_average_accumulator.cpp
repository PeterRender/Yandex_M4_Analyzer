#include "metric_accumulator_impl/sum_average_accumulator.hpp"  // интерфейс класса аккумулятора, вычисляющего сумму и среднее арифметическое значений метрики

#include <gtest/gtest.h>

namespace analyzer::metric_accumulator::metric_accumulator_impl::test {

using namespace analyzer::metric;

// Тест 1: Аккумулятор без данных
TEST(SumAvgAccTest, Empty) {
    SumAverageAccumulator acc;
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, 0);
    EXPECT_DOUBLE_EQ(result.average, 0.0);
}

// Тест 2: Аккумулятор с одним значением
TEST(SumAvgAccTest, SingleValue) {
    SumAverageAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 7});
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, 7);
    EXPECT_DOUBLE_EQ(result.average, 7.0);
}

// Тест 3: Аккумулятор с несколькими значениями
TEST(SumAvgAccTest, MultiValues) {
    SumAverageAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 1});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 2});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 3});
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, 6);               // 1+2+3 = 6
    EXPECT_DOUBLE_EQ(result.average, 2.0);  // (1+2+3)/3 = 2
}

// Тест 4: Аккумулятор с повторным использованием после Reset
TEST(SumAvgAccTest, Reuse) {
    SumAverageAccumulator acc;

    // Первое использование (дробный результат)
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 1});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 2});
    acc.Finalize();
    auto result1 = acc.Get();
    EXPECT_EQ(result1.sum, 3);
    EXPECT_DOUBLE_EQ(result1.average, 1.5);

    // Сброс и второе использование
    acc.Reset();
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 10});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 20});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 30});
    acc.Finalize();
    auto result2 = acc.Get();
    EXPECT_EQ(result2.sum, 60);               // 10+20+30 = 60
    EXPECT_DOUBLE_EQ(result2.average, 20.0);  // (10+20+30)/3 = 20
}

// Тест 5: Проверка исключения при Get до Finalize
TEST(SumAvgAccTest, GetBeforeFinalizeThrows) {
    SumAverageAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = 7});
    EXPECT_THROW(acc.Get(), std::runtime_error);
}

// Тест 6: Проверка исключения при Accumulate после Finalize
TEST(SumAvgAccTest, AccumulateAfterFinalizeThrows) {
    SumAverageAccumulator acc;
    acc.Finalize();
    EXPECT_THROW(acc.Accumulate(MetricResult{.metric_name = "test", .value = 7}), std::runtime_error);
}

// Тест 7: Большое количество значений
TEST(SumAvgAccTest, LotsOfValues) {
    SumAverageAccumulator acc;
    int expected_sum = 0;
    for (int i = 1; i <= 1000; ++i) {
        acc.Accumulate(MetricResult{.metric_name = "test", .value = i});
        expected_sum += i;
    }
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, expected_sum);
    EXPECT_DOUBLE_EQ(result.average, 500.5);  // среднее от 1 до 1000 = 500.5
}

// Тест 8: Отрицательные числа
TEST(SumAvgAccTest, NegativeValues) {
    SumAverageAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = -5});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = -10});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = -15});
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, -30);               // -5 + -10 + -15 = -30
    EXPECT_DOUBLE_EQ(result.average, -10.0);  // -30/3 = -10
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
