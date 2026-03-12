#include "metric_impl/code_lines_count.hpp"  // интерфейс класса метрики "Количество строк кода функции"
#include "metric_impl/test_utils.hpp"        // подключение шаблонной функции-хелпера вычисления метрики

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

// Примечание: при подсчете строк кода не учитываются строки с def, пустые строки и строки с комментариями

// Тест 1: Простая функция с 5 строками кода
TEST(CodeLinesCountTest, Simple) {
    auto result = CalcMetric<CodeLinesCountMetric>("simple.py", "test_simple");
    EXPECT_EQ(std::get<int>(result), 5);
}

// Тест 2: Функция с if, 3 строки кода
TEST(CodeLinesCountTest, SingleIf) {
    auto result = CalcMetric<CodeLinesCountMetric>("if.py", "testIf");
    EXPECT_EQ(std::get<int>(result), 3);
}

// Тест 3: Функция с вложенными if, 8 строк кода
TEST(CodeLinesCountTest, NestedIf) {
    auto result = CalcMetric<CodeLinesCountMetric>("nested_if.py", "Testnestedif");
    EXPECT_EQ(std::get<int>(result), 8);
}

// Тест 4: Функция с циклами, 6 строк кода
TEST(CodeLinesCountTest, Loops) {
    auto result = CalcMetric<CodeLinesCountMetric>("loops.py", "TestLoops");
    EXPECT_EQ(std::get<int>(result), 6);
}

// Тест 5: Функция с исключениями, 7 строк кода
TEST(CodeLinesCountTest, Exceptions) {
    auto result = CalcMetric<CodeLinesCountMetric>("exceptions.py", "Try_Exceptions");
    EXPECT_EQ(std::get<int>(result), 7);
}

// Тест 6: Функция с match-case, 7 строк кода
TEST(CodeLinesCountTest, MatchCase) {
    auto result = CalcMetric<CodeLinesCountMetric>("match_case.py", "test_Match_case");
    EXPECT_EQ(std::get<int>(result), 7);
}

// Тест 7: Функция с тернарным оператором, 1 строка кода
TEST(CodeLinesCountTest, Ternary) {
    auto result = CalcMetric<CodeLinesCountMetric>("ternary.py", "teSt_ternary");
    EXPECT_EQ(std::get<int>(result), 1);
}

// Тест 8: Функция с комментариями, 3 строки кода
TEST(CodeLinesCountTest, Comments) {
    auto result = CalcMetric<CodeLinesCountMetric>("comments.py", "Func_comments");
    EXPECT_EQ(std::get<int>(result), 3);
}

// Тест 9: Многострочная функция, 11 строк кода
TEST(CodeLinesCountTest, ManyLines) {
    auto result = CalcMetric<CodeLinesCountMetric>("many_lines.py", "testmultiline");
    EXPECT_EQ(std::get<int>(result), 11);
}

// Тест 10: Функция с множеством параметров, 1 строка кода
TEST(CodeLinesCountTest, ManyParameters) {
    auto result = CalcMetric<CodeLinesCountMetric>("many_parameters.py", "__test_multiparameters__");
    EXPECT_EQ(std::get<int>(result), 1);
}

}  // namespace analyzer::metric::metric_impl
