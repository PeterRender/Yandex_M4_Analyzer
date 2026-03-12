#include "metric_impl/cyclomatic_complexity.hpp"  // интерфейс класса метрики "Цикломатическая сложность функции"
#include "metric_impl/test_utils.hpp"             // подключение шаблонной функции-хелпера вычисления метрики

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

// Тест 1: "assert" (+1), базовая (+1) = сложность 2
TEST(CyclomaticComplexityTest, Simple) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("simple.py", "test_simple");
    EXPECT_EQ(std::get<int>(result), 2);
}

// Тест 2: "if_statement" (+1), базовая (+1) = сложность 2
TEST(CyclomaticComplexityTest, SingleIf) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("if.py", "testIf");
    EXPECT_EQ(std::get<int>(result), 2);
}

// Тест 3: "if_statement" (+1), "if_statement" (+1), "elif_clause" (+1), "assert" (+1), базовая (+1) = сложность 5
TEST(CyclomaticComplexityTest, NestedIf) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("nested_if.py", "Testnestedif");
    EXPECT_EQ(std::get<int>(result), 5);
}

// Тест 4: "for_statement" (+1), "while_statement" (+1), "if_statement" (+1), базовая (+1) = сложность 4
TEST(CyclomaticComplexityTest, Loops) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("loops.py", "TestLoops");
    EXPECT_EQ(std::get<int>(result), 4);
}

// Тест 5: "except_clause" (+1), "assert" (+1), базовая (+1) = сложность 3
// Примечание: элементы try (часть контекста) и finally (выполняется всегда) не увеличивают сложность
TEST(CyclomaticComplexityTest, Exceptions) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("exceptions.py", "Try_Exceptions");
    EXPECT_EQ(std::get<int>(result), 3);
}

// Тест 6: "case_clause" (+1), "case_clause" (+1), базовая (+1) = сложность 3
// Примечание: дефолтный case (_) не увеличивает сложность
TEST(CyclomaticComplexityTest, MatchCase) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("match_case.py", "test_Match_case");
    EXPECT_EQ(std::get<int>(result), 3);
}

// Тест 7: "conditional_expression" (+1), "conditional_expression" (+1), базовая (+1) = сложность 3
TEST(CyclomaticComplexityTest, Ternary) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("ternary.py", "teSt_ternary");
    EXPECT_EQ(std::get<int>(result), 3);
}

// Тест 8: базовая (+1) = сложность 1
TEST(CyclomaticComplexityTest, Comments) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("comments.py", "Func_comments");
    EXPECT_EQ(std::get<int>(result), 1);
}

// Тест 9: "assert" (+1), базовая (+1) = сложность 2
TEST(CyclomaticComplexityTest, ManyLines) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("many_lines.py", "testmultiline");
    EXPECT_EQ(std::get<int>(result), 2);
}

// Тест 10: "assert" (+1), базовая (+1) = сложность 2
TEST(CyclomaticComplexityTest, ManyParameters) {
    auto result = CalcMetric<CyclomaticComplexityMetric>("many_parameters.py", "__test_multiparameters__");
    EXPECT_EQ(std::get<int>(result), 2);
}

}  // namespace analyzer::metric::metric_impl
