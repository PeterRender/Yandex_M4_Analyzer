#include "metric_impl/cyclomatic_complexity.hpp"  // интерфейс класса метрики "Цикломатическая сложность функции"
#include "metric_impl/test_utils.hpp"             // подключение шаблонной функции-хелпера вычисления метрики

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

// Параметрический класс-тест метрики "Цикломатическая сложность функции"
// Параметры: имя файла, имя функции, ожидаемая сложность
class CyclomaticComplexityTest : public ::testing::TestWithParam<std::tuple<std::string, std::string, int>> {};

// Тест метрики "Цикломатическая сложность функции"
TEST_P(CyclomaticComplexityTest, CheckComplexity) {
    auto [filename, funcname, expected] = GetParam();
    auto result = CalcMetric<CyclomaticComplexityMetric>(filename, funcname);
    EXPECT_EQ(std::get<int>(result), expected);
}

// Набор из 10 тестов метрики "Цикломатическая сложность функции"
INSTANTIATE_TEST_SUITE_P(
    CyclomaticComplexityTestSuite, CyclomaticComplexityTest,
    ::testing::Values(std::make_tuple("simple.py", "test_simple", 2),          // assert
                      std::make_tuple("if.py", "testIf", 2),                   // if
                      std::make_tuple("nested_if.py", "Testnestedif", 5),      // if + if + elif + assert
                      std::make_tuple("loops.py", "TestLoops", 4),             // for + while + if
                      std::make_tuple("exceptions.py", "Try_Exceptions", 3),   // except + assert
                      std::make_tuple("match_case.py", "test_Match_case", 3),  // case + case
                      std::make_tuple("ternary.py", "teSt_ternary", 3),        // ternary + ternary
                      std::make_tuple("comments.py", "Func_comments", 1),      // нет управляющих конструкций
                      std::make_tuple("many_lines.py", "testmultiline", 2),    // assert
                      std::make_tuple("many_parameters.py", "__test_multiparameters__", 2)  // assert
                      ));
}  // namespace analyzer::metric::metric_impl
