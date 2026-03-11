#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"

namespace analyzer {

namespace rv = std::ranges::views;  // псевдоним пространства имен отображений
namespace rs = std::ranges;         // псевдоним пространства имен диапазонов

// Псевдоним типа для результата анализа - вектор пар (функция, вектор её метрик)
using AnalysisResult = std::vector<std::pair<function::Function, metric::MetricResults>>;

/**
 * @brief Анализирует список Python-файлов и извлекает метрики для всех функций и методов.
 *
 * Эта функция — центральный "конвейер" обработки:
 * 1. Принимает имена файлов.
 * 2. Для каждого файла создаёт объект `File`, который автоматически парсит его через tree-sitter
 *    и строит AST.
 * 3. Извлекает из AST все функции и методы с помощью `FunctionExtractor`.
 * 4. Объединяет все функции из всех файлов в один плоский список (`join`).
 * 5. Для каждой функции вычисляет набор метрик через переданный `metric_extractor`.
 * 6. Возвращает вектор пар: (функция, результаты её метрик).
 */
auto AnalyseFunctions(const std::vector<std::string> &files, const metric::MetricExtractor &metric_extractor) {

    // Лямбда, преобразующая Python-файл в AST-дерево (объект File) с помощью tree-sitter
    auto create_ast = [](const std::string &filename) { return file::File(filename); };

    // Лямбда, извлекающая из AST-дерева Python-файла структуры-описатели всех найденных функций
    function::FunctionExtractor extractor;  // объект-хелпер для извлечения всех функций из AST-дерева Python-файла
    auto extract_funcs = [&extractor](const file::File &file) { return extractor.Get(file); };

    // Лямбда, вычисляющая все зарегистрированные метрики для переданной функции
    auto calc_metrics = [&metric_extractor](const function::Function &func) {
        return std::make_pair(func, metric_extractor.Get(func));
    };

    // Создаем конвейер обработки.
    return files | rv::transform(create_ast) |  // преобразуем каждый Python-файл в AST-дерево (объект File)
           rv::transform(extract_funcs) |       // из каждого File извлекаем вектор структур-описателей всех функций
           rv::join |                     // объединяем все векторы структур-описателей функций в один плоский диапазон
           rv::transform(calc_metrics) |  // для каждой функции вычисляем метрики и формируем пару (функция, метрики)
           rs::to<std::vector>();         // преобразуем полученный диапазон в вектор результата анализа
}

/**
 *
 * @brief Группирует результаты анализа по классам.
 *
 * Эта функция:
 * 1. Отфильтровывает только те функции, которые являются **методами классов**
 *    (у них `class_name.has_value()` == true).
 * 2. Группирует последовательные элементы с одинаковым именем класса с помощью `chunk_by`.
 *
 * Важно:
 * - `chunk_by` работает только с **последовательными** одинаковыми элементами!
 *   Поэтому предполагается, что входной диапазон уже упорядочен по классам
 *   (например, порядок методов в AST сохраняется как в исходном файле).
 * - Если порядок нарушен, один и тот же класс может быть разбит на несколько групп.
 *
 *  Чтобы убедиться, что фильтрация работает, проверьте, что свободные функции (без class_name)
 * действительно исчезают из результата.
 */
auto SplitByClasses(const auto &analysis) {

    // Лямбда, проверяющая, является ли функция методом класса
    auto is_class_method = [](const auto &item) { return item.first.class_name.has_value(); };

    // Лямбда, проверяющая, что два метода принадлежат одному классу
    auto got_same_class = [](const auto &a, const auto &b) { return a.first.class_name == b.first.class_name; };

    return analysis | rv::filter(is_class_method) |  // фильтруем только методы классов
           rv::chunk_by(got_same_class) |            // группируем методы по одинаковым классам
           rs::to<std::vector>();                    // преобразуем диапазон в вектор векторов результатов анализа
}

/**
 * @brief Группирует результаты анализа по исходным файлам.
 *
 * Эта функция:
 * - Разбивает весь список функций на группы, где каждая группа содержит
 *   только функции из одного и того же файла (`filename`).
 * - Использует `chunk_by`, поэтому **порядок функций в `analysis` должен быть по файлам**.
 */
auto SplitByFiles(const auto &analysis) {
    // Лямбда, проверяющая, что две функции принадлежат одному файлу
    auto got_same_file = [](const auto &a, const auto &b) { return a.first.filename == b.first.filename; };

    return analysis | rv::chunk_by(got_same_file) |  // группируем функции по одинаковым файлам
           rs::to<std::vector>();                    // преобразуем диапазон в вектор векторов результатов анализа
}

/**
 * @brief Агрегирует метрики всех функций с помощью аккумулятора.
 *
 * Эта функция:
 * - Проходит по каждому элементу результата `AnalyseFunctions`
 *   (то есть по каждой функции и её метрикам).
 * - Передаёт результаты метрик (`elem.second`) в аккумулятор через `AccumulateNextFunctionResults`.
 */
void AccumulateFunctionAnalysis(const auto &analysis, const metric_accumulator::MetricsAccumulator &accumulator) {
    rs::for_each(analysis,
                 [&accumulator](const auto &elem) { accumulator.AccumulateNextFunctionResults(elem.second); });
}

}  // namespace analyzer
