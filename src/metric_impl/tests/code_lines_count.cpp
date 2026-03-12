#include "metric_impl/code_lines_count.hpp"  // интерфейс класса метрики "Количество строк кода функции"
#include "metric_impl/test_utils.hpp"        // подключение шаблонной функции-хелпера вычисления метрики

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

// Параметрический класс-тест метрики "Количество строк кода функции"
// Параметры: имя файла, имя функции, ожидаемое количество строк кода
// Примечание: при подсчете строк кода не учитываются строки с def, пустые строки и строки с комментариями
class CodeLinesCountTest : public ::testing::TestWithParam<std::tuple<std::string, std::string, int>> {};

// Тест метрики "Количество строк кода функции"
TEST_P(CodeLinesCountTest, CheckCount) {
    auto [filename, funcname, expected] = GetParam();
    auto result = CalcMetric<CodeLinesCountMetric>(filename, funcname);
    EXPECT_EQ(std::get<int>(result), expected);
}

// Набор из 10 тестов метрики "Количество строк кода функции"
INSTANTIATE_TEST_SUITE_P(CodeLinesCountTestSuite, CodeLinesCountTest,
                         ::testing::Values(std::make_tuple("simple.py", "test_simple", 5),
                                           std::make_tuple("if.py", "testIf", 3),
                                           std::make_tuple("nested_if.py", "Testnestedif", 8),
                                           std::make_tuple("loops.py", "TestLoops", 6),
                                           std::make_tuple("exceptions.py", "Try_Exceptions", 7),
                                           std::make_tuple("match_case.py", "test_Match_case", 7),
                                           std::make_tuple("ternary.py", "teSt_ternary", 1),
                                           std::make_tuple("comments.py", "Func_comments", 3),
                                           std::make_tuple("many_lines.py", "testmultiline", 11),
                                           std::make_tuple("many_parameters.py", "__test_multiparameters__", 1)));
}  // namespace analyzer::metric::metric_impl
