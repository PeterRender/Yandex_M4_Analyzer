#include "metric_impl/parameters_count.hpp"  // интерфейс класса метрики "Количество параметров функции"
#include "metric_impl/test_utils.hpp"        // подключение шаблонной функции-хелпера вычисления метрики

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

// Тест 1: Функция без параметров
TEST(ParametersCountTest, NoParams) {
    // В файле simple.py есть функция test_simple() без параметров
    auto result = CalcMetric<CountParametersMetric>("simple.py", "test_simple");
    EXPECT_EQ(std::get<int>(result), 0);
}

// Тест 2: Функция с одним параметром и простым телом
TEST(ParametersCountTest, SingleParamSimpleBody) {
    // В файле if.py есть функция testIf(x) с одним параметром
    auto result = CalcMetric<CountParametersMetric>("if.py", "testIf");
    EXPECT_EQ(std::get<int>(result), 1);
}

// Тест 3: Функция с двумя параметрами
TEST(ParametersCountTest, TwoParams) {
    // В файле nested_if.py есть функция Testnestedif(x, y) с двумя параметрами
    auto result = CalcMetric<CountParametersMetric>("nested_if.py", "Testnestedif");
    EXPECT_EQ(std::get<int>(result), 2);
}

// Тест 4: Функция с параметрами с типом и значениями по умолчанию
TEST(ParametersCountTest, TypedAndDefaultParams) {
    // В файле many_parameters.py есть функция __test_multiparameters__(a, b, c=5, *args, **kwargs)
    // Всего должно быть 5 параметров: a, b, c, *args, **kwargs
    auto result = CalcMetric<CountParametersMetric>("many_parameters.py", "__test_multiparameters__");
    EXPECT_EQ(std::get<int>(result), 5);
}

// Тест 5: Функция без параметров с телом, содержащим обработку исключений
TEST(ParametersCountTest, NoParamExceptionBody) {
    // В файле exceptions.py есть функция Try_Exceptions() без параметров с телом с обработкой исключений
    auto result = CalcMetric<CountParametersMetric>("exceptions.py", "Try_Exceptions");
    EXPECT_EQ(std::get<int>(result), 0);
}

// Тест 6: Функция с тремя параметрами
TEST(ParametersCountTest, ThreeParams) {
    // В файле comments.py есть функция Func_comments(result, a, b) с тремя параметрами
    auto result = CalcMetric<CountParametersMetric>("comments.py", "Func_comments");
    EXPECT_EQ(std::get<int>(result), 3);
}

// Тест 7: Функция с одним параметром и сложным телом
TEST(ParametersCountTest, SingleParamComplexBody) {
    // В файле loops.py есть функция TestLoops(n) с одним параметром и сложным телом
    auto result = CalcMetric<CountParametersMetric>("loops.py", "TestLoops");
    EXPECT_EQ(std::get<int>(result), 1);
}

// Тест 8: Функция с одним параметром и телом с тернарным оператором
TEST(ParametersCountTest, SingleParamTernary) {
    // В файле ternary.py есть функция teSt_ternary(x) с одним параметром и тернарным оператором
    auto result = CalcMetric<CountParametersMetric>("ternary.py", "teSt_ternary");
    EXPECT_EQ(std::get<int>(result), 1);
}

// Тест 9: Функция без параметров с многострочным телом
TEST(ParametersCountTest, NoParamsMultilineBody) {
    // В файле many_lines.py есть функция testmultiline() без параметров
    auto result = CalcMetric<CountParametersMetric>("many_lines.py", "testmultiline");
    EXPECT_EQ(std::get<int>(result), 0);
}

// Тест 10: Функция с одним параметром и match-case телом
TEST(ParametersCountTest, SingleParamMatchCaseBody) {
    // В файле match_case.py есть функция test_Match_case(x) с одним параметром
    auto result = CalcMetric<CountParametersMetric>("match_case.py", "test_Match_case");
    EXPECT_EQ(std::get<int>(result), 1);
}

}  // namespace analyzer::metric::metric_impl
