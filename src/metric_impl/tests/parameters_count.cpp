#include "metric_impl/parameters_count.hpp"  // интерфейс класса метрики "Количество параметров функции"
#include "metric_impl/test_utils.hpp"        // подключение шаблонной функции-хелпера вычисления метрики

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

// Параметрический класс-тест метрики "Количество параметров функции"
// Параметры: имя файла, имя функции, ожидаемое количество параметров
class ParametersCountTest : public ::testing::TestWithParam<std::tuple<std::string, std::string, int>> {};

// Тест метрики "Количество параметров функции"
TEST_P(ParametersCountTest, CheckCount) {
    auto [filename, funcname, expected] = GetParam();
    auto result = CalcMetric<CountParametersMetric>(filename, funcname);
    EXPECT_EQ(std::get<int>(result), expected);
}

// Набор из 10 тестов метрики "Количество параметров функции"
INSTANTIATE_TEST_SUITE_P(ParametersCountTestSuite, ParametersCountTest,
                         ::testing::Values(std::make_tuple("simple.py", "test_simple", 0),
                                           std::make_tuple("if.py", "testIf", 1),
                                           std::make_tuple("nested_if.py", "Testnestedif", 2),
                                           std::make_tuple("many_parameters.py", "__test_multiparameters__", 5),
                                           std::make_tuple("exceptions.py", "Try_Exceptions", 0),
                                           std::make_tuple("comments.py", "Func_comments", 3),
                                           std::make_tuple("loops.py", "TestLoops", 1),
                                           std::make_tuple("ternary.py", "teSt_ternary", 1),
                                           std::make_tuple("many_lines.py", "testmultiline", 0),
                                           std::make_tuple("match_case.py", "test_Match_case", 1)));
}  // namespace analyzer::metric::metric_impl
