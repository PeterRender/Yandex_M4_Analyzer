#include "metric_impl/parameters_count.hpp"

#include <gtest/gtest.h>

namespace analyzer::metric::metric_impl {

namespace fs = std::filesystem;

std::string test_sub_dir = {"src/metric_impl"};

// Функция-хелпер, вычисляющая метрику для заданной функции из Python-файла
int GetParameterCount(const std::string &filename, const std::string &func_name) {
    fs::path filepath = fs::current_path() / test_sub_dir / filename;  // путь к тестовому Python-файлу

    // Проверка наличия тестового Python-файла с заданным именем
    if (!fs::exists(filepath)) {
        throw std::runtime_error("Test file not found: " + filepath.string());
    }

    try {
        // Преобразуем Python-файл в AST-дерево (объект File) с помощью tree-sitter
        file::File file(filepath.string());

        // Извлекаем все функции из AST-дерева
        function::FunctionExtractor extractor;
        auto functions = extractor.Get(file);

        // Ищем функцию с заданным именем
        auto it = std::ranges::find_if(functions, [&func_name](const auto &f) { return f.name == func_name; });
        if (it == functions.end()) {
            throw std::runtime_error("Function " + func_name + " not found in " + filename);
        }

        // Вычисляем метрику для заданной функции
        CountParametersMetric metric;
        auto result = metric.Calculate(*it);

        // Возвращаем полученное значение метрики
        // return std::get<int>(result.value);
        return result.value;

    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Error processing ") + filename + ": " + e.what());
    }
}

// Тест 1: Функция без параметров
TEST(ParametersCountTest, NoParams) {
    // В файле simple.py есть функция test_simple() без параметров
    EXPECT_EQ(GetParameterCount("simple.py", "test_simple"), 0);
}

// Тест 2: Функция с одним параметром и простым телом
TEST(ParametersCountTest, SingleParamSimpleBody) {
    // В файле if.py есть функция testIf(x) с одним параметром
    EXPECT_EQ(GetParameterCount("if.py", "testIf"), 1);
}

// Тест 3: Функция с двумя параметрами
TEST(ParametersCountTest, TwoParams) {
    // В файле nested_if.py есть функция Testnestedif(x, y) с двумя параметрами
    EXPECT_EQ(GetParameterCount("nested_if.py", "Testnestedif"), 2);
}

// Тест 4: Функция с параметрами с типом и значениями по умолчанию
TEST(ParametersCountTest, TypedAndDefaultParams) {
    // В файле many_parameters.py есть функция __test_multiparameters__(a, b, c=5, *args, **kwargs)
    // Всего должно быть 5 параметров: a, b, c, *args, **kwargs
    EXPECT_EQ(GetParameterCount("many_parameters.py", "__test_multiparameters__"), 5);
}

// Тест 5: Функция без параметров с телом, содержащим обработку исключений
TEST(ParametersCountTest, NoParamExceptionBody) {
    // В файле exceptions.py есть функция Try_Exceptions() без параметров с телом с обработкой исключений
    EXPECT_EQ(GetParameterCount("exceptions.py", "Try_Exceptions"), 0);
}

// Тест 6: Функция с тремя параметрами
TEST(ParametersCountTest, ThreeParams) {
    // В файле comments.py есть функция Func_comments(result, a, b) с тремя параметрами
    EXPECT_EQ(GetParameterCount("comments.py", "Func_comments"), 3);
}

// Тест 7: Функция с одним параметром и сложным телом
TEST(ParametersCountTest, SingleParamComplexBody) {
    // В файле loops.py есть функция TestLoops(n) с одним параметром и сложным телом
    EXPECT_EQ(GetParameterCount("loops.py", "TestLoops"), 1);
}

// Тест 8: Функция с одним параметром и телом с тернарным оператором
TEST(ParametersCountTest, SingleParamTernary) {
    // В файле ternary.py есть функция teSt_ternary(x) с одним параметром и тернарным оператором
    EXPECT_EQ(GetParameterCount("ternary.py", "teSt_ternary"), 1);
}

// Тест 9: Функция без параметров с многострочным телом
TEST(ParametersCountTest, NoParamsMultilineBody) {
    // В файле many_lines.py есть функция testmultiline() без параметров
    EXPECT_EQ(GetParameterCount("many_lines.py", "testmultiline"), 0);
}

// Тест 10: Функция с одним параметром и match-case телом
TEST(ParametersCountTest, SingleParamMatchCaseBody) {
    // В файле match_case.py есть функция test_Match_case(x) с одним параметром
    EXPECT_EQ(GetParameterCount("match_case.py", "test_Match_case"), 1);
}

}  // namespace analyzer::metric::metric_impl
