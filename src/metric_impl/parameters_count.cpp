#include "metric_impl/parameters_count.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
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
std::string CountParametersMetric::Name() const { return kName; }

// Метод, реализующий вычисление метрики (принимает функцию, возвращает количество ее параметров)
MetricResult::ValueType CountParametersMetric::CalculateImpl(const function::Function &f) const {
    // Получаем строковое представление AST-дерева функции
    auto &function_ast = f.ast;

    // Список всех возможных типов узлов, представляющих параметры функции в AST
    constexpr std::array<std::string_view, 6> parameter_nodes = {
        "(identifier",               // обычный параметр: def foo(a)
        "(default_parameter",        // параметр со значением по умолчанию: def foo(a=5)
        "(typed_parameter",          // параметр с типом: def foo(a: int)
        "(typed_default_parameter",  // параметр с типом и значением: def foo(a: int = 5)
        "(list_splat_pattern",       // *args - собирает оставшиеся позиционные аргументы
        "(dictionary_splat_pattern"  // **kwargs - собирает оставшиеся именованные аргументы
    };

    // 1. Находим блок параметров функции в ее AST-дереве (параметры находятся внутри узла (parameters ...)
    const std::string params_marker = "(parameters";
    size_t params_start = function_ast.find(params_marker);
    if (params_start == std::string::npos) {
        return 0;  // если узел parameters не найден, значит у функции нет параметров
    }

    // 2. Находим конец блока параметров, используя баланс скобок
    // (это необходимо, чтобы извлечь точную AST-подстроку, содержащую только параметры)
    size_t balance = 1;  // начинаем с 1, т.к. уже нашли открывающую скобку маркера
    size_t params_end = params_start + params_marker.length();

    // Идем по строке, считая открывающие и закрывающие скобки
    while (params_end < function_ast.size() && balance > 0) {
        if (function_ast[params_end] == '(')
            balance++;  // встретили открывающую скобку - уровень вложенности растет
        else if (function_ast[params_end] == ')')
            balance--;  // встретили закрывающую - уровень падает
        params_end++;
    }

    // 3. Создаем отображение (std::string_view) подстроки, содержащей только блок параметров
    std::string_view params_block(function_ast.data() + params_start, params_end - params_start);

    // Лямбда, подсчитывающая количество вхождений заданного типа узла (из parameter_nodes) в AST блока параметров
    auto count_node_matches = [params_block](std::string_view node_type) -> size_t {
        size_t count = 0;
        size_t pos = 0;

        // Ищем все вхождения node_type в блоке параметров
        while ((pos = params_block.find(node_type, pos)) != std::string_view::npos) {
            count++;
            pos += node_type.length();  // продолжаем поиск со следующей позиции
        }
        return count;
    };

    // Создаем отображение из количеств вхождений каждого типа узла
    auto matches_view = parameter_nodes | rv::transform(count_node_matches);

    // Суммируем все количества вхождений с помощью fold_left_first
    // (fold_left_first принимает два аргумента: диапазон и функцию сложения)
    auto total_param_nodes = rs::fold_left_first(matches_view, std::plus<>());

    // Возвращаем подсчитанное количество параметров функции
    // (parameter_nodes не пуст, transform сохраняет размер диапазона, поэтому fold_left_first всегда вернет значение)
    return static_cast<MetricResult::ValueType>(total_param_nodes.value());
}

}  // namespace analyzer::metric::metric_impl
