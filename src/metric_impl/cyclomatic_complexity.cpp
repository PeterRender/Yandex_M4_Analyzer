#include "metric_impl/cyclomatic_complexity.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace analyzer::metric::metric_impl {

namespace rv = std::ranges::views;  // псевдоним пространства имен отображений
namespace rs = std::ranges;         // псевдоним пространства имен диапазонов

// Метод, возвращающий название метрики
std::string CyclomaticComplexityMetric::Name() const { return kName; }

// Метод, реализующий вычисление метрики (принимает функцию, возвращает ее цикломатическую сложность)
MetricResult::ValueType CyclomaticComplexityMetric::CalculateImpl(const function::Function &f) const {
    // Получаем строковое представление AST (абстрактного синтаксического дерева) функции.
    // Это S-выражение, сгенерированное утилитой tree-sitter, например:
    // "(function_definition name: (identifier) ... (if_statement ...) (for_statement ...))"
    auto &function_ast = f.ast;

    // Список типов узлов AST, каждый из которых увеличивает цикломатическую сложность на 1.
    // Эти узлы соответствуют управляющим конструкциям языка Python, создающим новые пути выполнения:
    // - if / elif - условные переходы
    // - циклы (for, while) - могут выполниться 0 или более раз
    // - except - обработчик исключений (новый путь при возникновении исключения)
    // - case - отдельная ветка в match (каждая case создает новый путь)
    // - assert - может не выполниться при отключенных ассертах, но считается
    // - тернарный оператор (conditional_expression) - это if в выражении
    constexpr std::array<std::string_view, 8> complexity_nodes = {
        "if_statement",            // if
        "elif_clause",             // elif
        "for_statement",           // for
        "while_statement",         // while
        "except_clause",           // except
        "case_clause",             // case
        "assert",                  // assert
        "conditional_expression",  // для тернарного оператора

        // Примечание: следующие конструкции не увеличивают цикломатическую сложность:
        // - else_clause - это часть уже учтенного if/elif, не создает нового пути
        // - try_statement - сам по себе не создает ветвления, только контекст для except/finally
        // - finally_clause - выполняется всегда, независимо от возникновения исключения
        // - match_statement - это только начало конструкции, ветки считаются через case_clause
        // - case_clause с паттерном "_" - это дефолтная ветка (аналог else), не создает нового пути
    };

    // Цель: подсчитать, сколько раз в строке `function_ast` встречаются
    // любые из узлов из `complexity_nodes`.
    //
    // Важно:
    // - Имена узлов уникальны и не являются подстроками других имён, поэтому
    //   поиск подстроки (например, `"if_statement"`) безопасен.
    // - Каждое вхождение узла = +1 к сложности.
    // - В конце к общей сумме нужно прибавить 1 (базовая сложность функции без ветвлений).
    //
    // Пример:
    // Если AST содержит "(if_statement ...) (for_statement ...) (if_statement ...)",
    // то найдено 3 узла → сложность = 3 + 1 = 4.
    //
    // Подсказка:
    // Можно пройтись по каждому `node_type` из `complexity_nodes` и подсчитать,
    // сколько раз он встречается в `function_ast`, используя `std::string::find`
    // в цикле (это допустимо, так как вы работаете со строковым представлением AST,
    // а не с исходным кодом напрямую).

    // Лямбда для проверки, является ли case дефолтным (case _:)
    auto is_default_case = [&](size_t case_pos) -> bool {
        // Вид обычного case: (case_clause (case_pattern [line, col] - [line, col] (integer...
        // Вид дефолтного case: (case_clause (case_pattern [line, col] - [line, col]) consequence: (block...
        // Критерий различия: если у case_pattern закрывающая круглая скобка идет впереди открывающей (нет дочерних
        // узлов), значит это дефолтный case, который не увеличивает цикломатическую сложность.

        // Ищем позицию узла case_pattern внутри case_clause
        size_t pattern_pos = function_ast.find("case_pattern", case_pos);
        if (pattern_pos == std::string::npos)
            return false;

        // Находим ближайшие открывающую и закрывающую скобки после case_pattern
        size_t open_bracket = function_ast.find('(', pattern_pos + 12);   // ищем '(' после "case_pattern"
        size_t close_bracket = function_ast.find(')', pattern_pos + 12);  // ищем ')' после "case_pattern"

        // Если закрывающая скобка идет до открывающей, значит это дефолтный case
        return (close_bracket < open_bracket);
    };

    // Лямбда, подсчитывающая количество вхождений заданного типа узла (из complexity_nodes) в AST-дерево функции
    auto count_node_matches = [&function_ast, &is_default_case](std::string_view node_type) -> size_t {
        size_t count = 0;
        size_t pos = 0;

        // Ищем все вхождения node_type в строке AST
        while ((pos = function_ast.find(node_type, pos)) != std::string::npos) {
            // Для case_clause проверяем, не является ли он дефолтным
            if (node_type == "case_clause" && is_default_case(pos)) {
                pos += node_type.length();  // пропускаем дефолтный case
                continue;
            }
            count++;
            pos += node_type.length();  // продолжаем поиск со следующей позиции
        }
        return count;
    };

    // Создаем отображение из количеств вхождений каждого типа узла
    auto matches_view = complexity_nodes | rv::transform(count_node_matches);

    // Суммируем все количества вхождений с помощью fold_left_first
    // (fold_left_first принимает два аргумента: диапазон и функцию сложения)
    auto total_control_nodes = rs::fold_left_first(matches_view, std::plus<>());

    // Возвращаем подсчитанную цикломатическую сложность (int, обернутый в variant)
    // 1. complexity_nodes не пуст, transform сохраняет размер диапазона, поэтому fold_left_first всегда вернет значение
    // 2. В конце добавляем базовую цикломатическая сложность = 1 (всегда есть как минимум один путь выполнения)
    return static_cast<int>(total_control_nodes.value()) + 1;
}
}  // namespace analyzer::metric::metric_impl
