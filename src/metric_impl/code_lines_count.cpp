#include "metric_impl/code_lines_count.hpp"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ranges>
#include <string>

#include "utils.hpp"  // для подключения функции ToInt преобразования числа из строкового вида в целочисленный.

namespace analyzer::metric::metric_impl {

namespace rv = std::ranges::views;  // псевдоним пространства имен отображений
namespace rs = std::ranges;         // псевдоним пространства имен диапазонов

// Метод, возвращающий название метрики
std::string CodeLinesCountMetric::Name() const { return kName; }

// Метод, реализующий вычисление метрики (принимает функцию, возвращает количество ее строк кода)
MetricResult::ValueType CodeLinesCountMetric::CalculateImpl(const function::Function &f) const {
    // Ссылка на AST-поддерево функции, для которой ведется подсчет метрики
    auto &function_ast = f.ast;

    // Вспомогательная лямбда для извлечения номера строки из диапазона узла AST.
    // Формат узла в S-выражении: (node_type [start_line,start_column] [end_line,end_column] ...)
    // Эта функция ищет открывающую скобку "[" после заданной позиции и парсит первую координату - номер строки.
    auto line_number = [&](int start_pos) {
        size_t line_pos = function_ast.find("[", start_pos);
        size_t comma_pos = function_ast.find(",", line_pos);
        return ToInt(function_ast.substr(line_pos + 1, comma_pos - line_pos - 1));
    };
    // Определяем начальную и конечную строки тела функции:
    // - начальная строка берётся из корневого узла функции (первое вхождение "[")
    // - конечная строка ищется по шаблону "] -"
    const int start_line = line_number(0);
    const int end_line = line_number(function_ast.find("] -"));

    // Лямбда, проверяющая, является ли конкретная строка "кодовой", то есть не комментарием.
    // Принцип работы:
    // 1. Ищем в AST все узлы, которые начинаются на указанной строке (по маркеру "[line,")
    // 2. Если таких узлов нет - строка пустая (нет кода)
    // 3. Если есть - определяем тип первого найденного узла
    // 4. Строка считается кодовой, если тип узла не "comment"
    auto is_code_line = [&](int line) {
        // Формируем поисковый маркер для данной строки
        std::string line_marker = "[" + std::to_string(line) + ",";

        // Ищем позицию начала координат узла на этой строке
        size_t line_pos = function_ast.find(line_marker);
        if (line_pos == std::string::npos)
            return false;  // на этой строке нет ни одного AST-узла (например, строка с отступами или пустая)

        // Находим начало узла (открывающую скобку) которому принадлежат эти координаты
        // (rfind ищет справа налево, находя ближайшую открывающую скобку)
        size_t node_start = function_ast.rfind('(', line_pos);
        if (node_start == std::string::npos)
            return false;  // аномалия: есть координаты, но нет узла

        // Определяем конец типа узла (до первого пробела, символа перевода строки или начала координат)
        size_t node_type_end = function_ast.find_first_of(" \n[", node_start + 1);

        // Извлекаем тип узла без копирования (string_view)
        // Пример извлеченного типа узла: "(function_definition [0,0] ..." -> тип "function_definition"
        std::string_view node_type =
            std::string_view(function_ast).substr(node_start + 1, node_type_end - node_start - 1);

        // Строка содержит код, если это не комментарий
        return node_type != "comment";
    };

    // Цель: подсчитать количество строк в диапазоне [start_line + 1, end_line],
    // которые действительно содержат код (а не только комментарии или пустые строки).
    //
    // Почему start_line + 1?
    // Потому что первая строка - это строка с объявлением функции (def ...),
    // а тело функции начинается со следующей строки (обычно с отступа).

    // Создаем композицию отображений:
    // 1. Генерируем диапазон чисел от (start_line + 1) до end_line включительно
    // 2. Фильтруем только строки с кодом
    auto code_lines_view = rv::iota(start_line + 1, end_line + 1) | rv::filter(is_code_line);

    // Возвращаем подсчитанное количество элементов в отфильтрованном диапазоне (int, обернутый в variant)
    return static_cast<int>(rs::distance(code_lines_view));
}
}  // namespace analyzer::metric::metric_impl
