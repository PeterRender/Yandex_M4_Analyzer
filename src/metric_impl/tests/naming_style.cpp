#include "metric_impl/naming_style.hpp"  // интерфейс класса метрики "Определение стиля имени функции"
#include "metric_impl/test_utils.hpp"    // подключение шаблонной функции-хелпера вычисления метрики

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

// Параметрический класс-тест метрики "Определение стиля имени функции"
// Параметры: имя файла, имя функции, ожидаемый стиль
class NamingStyleTest : public ::testing::TestWithParam<std::tuple<std::string, std::string, std::string>> {
protected:
};

// Тест метрики "Определение стиля имени функции"
TEST_P(NamingStyleTest, CheckStyle) {
    auto [filename, funcname, expected] = GetParam();
    auto result = CalcMetric<NamingStyleMetric>(filename, funcname);
    EXPECT_EQ(std::get<std::string>(result), expected);
}

// Набор тестов метрики "Определение стиля имени функции"
INSTANTIATE_TEST_SUITE_P(NamingStyleTestSuite, NamingStyleTest,
                         ::testing::Values(std::make_tuple("simple.py", "test_simple", "Snake Case"),
                                           std::make_tuple("many_parameters.py", "__test_multiparameters__",
                                                           "Snake Case"),
                                           std::make_tuple("loops.py", "TestLoops", "Pascal Case"),
                                           std::make_tuple("nested_if.py", "Testnestedif", "Pascal Case"),
                                           std::make_tuple("if.py", "testIf", "Camel Case"),
                                           std::make_tuple("many_lines.py", "testmultiline", "Lower Case"),
                                           std::make_tuple("comments.py", "Func_comments", "Unknown"),
                                           std::make_tuple("exceptions.py", "Try_Exceptions", "Unknown"),
                                           std::make_tuple("match_case.py", "test_Match_case", "Unknown"),
                                           std::make_tuple("ternary.py", "teSt_ternary", "Unknown")));
}  // namespace analyzer::metric::metric_impl
