#include "metric_accumulator_impl/categorical_accumulator.hpp"  // интерфейс класса аккумулятора для категориальных (строковых) метрик

#include <gtest/gtest.h>

#include <stdexcept>

namespace analyzer::metric_accumulator::metric_accumulator_impl::test {

using namespace analyzer::metric;

// Тест 1: Пустой аккумулятор
TEST(CatAccTest, Empty) {
    CategoricalAccumulator acc;
    acc.Finalize();
    EXPECT_TRUE(acc.Get().empty());
}

// Тест 2: Одно значение
TEST(CatAccTest, SingleValue) {
    CategoricalAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")});
    acc.Finalize();

    auto result = acc.Get();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.at("Snake Case"), 1);
}

// Тест 3: Несколько разных значений
TEST(CatAccTest, MultiDifferentValues) {
    CategoricalAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Pascal Case")});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Camel Case")});
    acc.Finalize();

    auto result = acc.Get();
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result.at("Snake Case"), 1);
    EXPECT_EQ(result.at("Pascal Case"), 1);
    EXPECT_EQ(result.at("Camel Case"), 1);
}

// Тест 4: Повторяющиеся значения
TEST(CatAccTest, RepeatedValues) {
    CategoricalAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Pascal Case")});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")});
    acc.Finalize();

    auto result = acc.Get();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result.at("Snake Case"), 3);
    EXPECT_EQ(result.at("Pascal Case"), 1);
}

// Тест 5: Сброс и повторное использование
TEST(CatAccTest, Reset) {
    CategoricalAccumulator acc;

    // Первое использование
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")});
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")});
    acc.Finalize();
    EXPECT_EQ(acc.Get().at("Snake Case"), 2);

    // Второе использование
    acc.Reset();
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Pascal Case")});
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.at("Pascal Case"), 1);
}

// Тест 6: Проверка исключения при Get до Finalize
TEST(CatAccTest, GetBeforeFinalizeThrows) {
    CategoricalAccumulator acc;
    acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")});
    EXPECT_THROW(acc.Get(), std::runtime_error);
}

// Тест 7: Проверка исключения при Accumulate после Finalize
TEST(CatAccTest, AccumulateAfterFinalizeThrows) {
    CategoricalAccumulator acc;
    acc.Finalize();
    EXPECT_THROW(acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Snake Case")}),
                 std::runtime_error);
}

// Тест 8: Много уникальных значений
TEST(CatAccTest, ManyUniqueValues) {
    CategoricalAccumulator acc;
    for (int i = 0; i < 100; ++i) {
        acc.Accumulate(MetricResult{.metric_name = "test", .value = std::string("Value") + std::to_string(i)});
    }
    acc.Finalize();

    auto result = acc.Get();
    EXPECT_EQ(result.size(), 100);
    // Проверяем несколько случайных значений
    EXPECT_EQ(result.at("Value0"), 1);
    EXPECT_EQ(result.at("Value45"), 1);
    EXPECT_EQ(result.at("Value99"), 1);
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
