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

    // 4. Считаем узлы верхнего уровня в блоке parameters
    // (каждый такой узел соответствует одному параметру функции)

    size_t pos = params_marker.length();          // позиция первого символа после "(parameters"
    size_t block_length = params_block.length();  // число символов в блоке параметров функции

    int count = 0;  // счетчик параметров функции

    while (pos < block_length) {
        // Пропускаем пробелы и служебные символы
        pos = params_block.find_first_not_of(" \n\t", pos);
        if (pos == std::string_view::npos || pos >= block_length)
            break;

        // Если нашли открывающую скобку - это начало узла параметра
        if (params_block[pos] == '(') {
            count++;  // каждый узел верхнего уровня в parameters - это один параметр функции
            pos++;

            // Пропускаем весь узел целиком, используя баланс скобок
            // (нас не интересует внутренняя структура узла)
            int balance = 1;
            while (pos < block_length && balance > 0) {
                if (params_block[pos] == '(')
                    balance++;  // встретили открывающую скобку - уровень вложенности растет
                else if (params_block[pos] == ')')
                    balance--;  // встретили закрывающую - уровень падает
                pos++;
            }
        } else {
            pos++;  // пропускаем другие символы (например, пробелы между узлами)
        }
    }

    // Возвращаем подсчитанное количество параметров функции (int, обернутый в variant)
    return static_cast<int>(count);
}

}  // namespace analyzer::metric::metric_impl
