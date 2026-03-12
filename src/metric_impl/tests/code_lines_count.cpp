#include "metric_impl/code_lines_count.hpp"  // интерфейс класса метрики "Количество строк кода функции"
#include "metric_impl/test_utils.hpp"        // подключение шаблонной функции-хелпера вычисления метрики

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

// Примечание: при подсчете строк кода не учитываются строки с def, пустые строки и строки с комментариями

// Тест 1: Простая функция с 5 строками кода
TEST(CodeLinesCountTest, Simple) { EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("simple.py", "test_simple"), 5); }

// Тест 2: Функция с if, 3 строки кода
TEST(CodeLinesCountTest, SingleIf) { EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("if.py", "testIf"), 3); }

// Тест 3: Функция с вложенными if, 8 строк кода
TEST(CodeLinesCountTest, NestedIf) { EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("nested_if.py", "Testnestedif"), 8); }

// Тест 4: Функция с циклами, 6 строк кода
TEST(CodeLinesCountTest, Loops) { EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("loops.py", "TestLoops"), 6); }

// Тест 5: Функция с исключениями, 7 строк кода
TEST(CodeLinesCountTest, Exceptions) {
    EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("exceptions.py", "Try_Exceptions"), 7);
}

// Тест 6: Функция с match-case, 7 строк кода
TEST(CodeLinesCountTest, MatchCase) {
    EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("match_case.py", "test_Match_case"), 7);
}

// Тест 7: Функция с тернарным оператором, 1 строка кода
TEST(CodeLinesCountTest, Ternary) { EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("ternary.py", "teSt_ternary"), 1); }

// Тест 8: Функция с комментариями, 3 строки кода
TEST(CodeLinesCountTest, Comments) { EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("comments.py", "Func_comments"), 3); }

// Тест 9: Многострочная функция, 11 строк кода
TEST(CodeLinesCountTest, ManyLines) {
    EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("many_lines.py", "testmultiline"), 11);
}

// Тест 10: Функция с множеством параметров, 1 строка кода
TEST(CodeLinesCountTest, ManyParameters) {
    EXPECT_EQ(CalcMetric<CodeLinesCountMetric>("many_parameters.py", "__test_multiparameters__"), 1);
}

}  // namespace analyzer::metric::metric_impl
