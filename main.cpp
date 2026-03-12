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

#include "analyse.hpp"
#include "cmd_options.hpp"
#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"
#include "metric_accumulator_impl/accumulators.hpp"
#include "metric_impl/metrics.hpp"

int main(int argc, char *argv[]) {
    try {
        // ==== 1. Обработка аргументов командной строки ====
        // Создаем парсер опций командной строки и передаем ему аргументы
        analyzer::cmd::ProgramOptions options;
        if (!options.Parse(argc, argv))
            return 1;  // если парсинг не удался или был запрошен help, завершаем работу

        // ==== 2. Регистрация метрик ====
        // Создаем экстрактор метрик и регистрируем в нем все реализованные метрики
        using namespace analyzer::metric::metric_impl;
        analyzer::metric::MetricExtractor metric_extractor;
        // Регистрируем метрику цикломатической сложности
        metric_extractor.RegisterMetric(std::make_unique<CyclomaticComplexityMetric>());
        // Регистрируем метрику количества строк кода
        metric_extractor.RegisterMetric(std::make_unique<CodeLinesCountMetric>());
        // Регистрируем метрику количества параметров
        metric_extractor.RegisterMetric(std::make_unique<CountParametersMetric>());
        // Опционально: метрика стиля именования (пока закомментирована)
        // metric_extractor.RegisterMetric(std::make_unique<NamingStyleMetric>());

        // ==== 3. Анализ всех функций из переданных файлов ====
        // Вызываем главную функцию анализа, которая:
        // - для каждого Python-файла получает AST-дерево с помощью tree-sitter
        // - извлекает из AST-дерева все функции
        // - вычисляет для каждой функции все зарегистрированные метрики
        auto analysis = analyzer::AnalyseFunctions(options.GetFiles(), metric_extractor);

        // ==== 4. Вывод результатов для каждой функции (детальный анализ) ====
        std::println("Analysis for every function:");
        std::ranges::for_each(analysis, [&](const auto &elem) {
            const auto &[function, metrics] = elem;
            // Формируем полное имя функции: если это метод класса, добавляем имя класса
            std::println("  {}::{}{}: ", function.filename,
                         (function.class_name.has_value() ? function.class_name.value() + "::" : ""), function.name);

            // Выводим все метрики для данной функции
            std::ranges::for_each(metrics, [&](const auto &result) {
                std::print("    {}: ", result.metric_name);
                // std::visit([](auto &&val) { std::println("{}", val); }, result.value);
                // ВРЕМЕННО: используем value напрямую (это int)
                std::println("{}", result.value);
            });
        });

        // ==== 5. Регистрация аккумуляторов для агрегации результатов ====
        // Аккумуляторы собирают статистику по группам функций (по файлам, по классам, общую)
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        using namespace analyzer::metric_accumulator::metric_accumulator_impl;
        // Для цикломатической сложности считаем сумму и среднее
        accumulator.RegisterAccumulator(CyclomaticComplexityMetric::kName, std::make_unique<SumAverageAccumulator>());
        // Для количества строк кода считаем сумму и среднее
        accumulator.RegisterAccumulator(CodeLinesCountMetric::kName, std::make_unique<SumAverageAccumulator>());
        // Для количества параметров считаем только среднее
        accumulator.RegisterAccumulator(CountParametersMetric::kName, std::make_unique<AverageAccumulator>());
        // Опционально: для стиля именования нужен категориальный аккумулятор (пока закомментировано)
        // accumulator.RegisterAccumulator(NamingStyleMetric::kName, std::make_unique<CategoricalAccumulator>());

        // ==== 6. Вспомогательная лямбда для вывода агрегированных результатов ====
        auto print_accumulated_analysis = [](const auto &accumulator) {
            // Получаем и выводим результаты для цикломатической сложности
            auto &cc_acc_metric =
                accumulator.template GetFinalizedAccumulator<SumAverageAccumulator>(CyclomaticComplexityMetric::kName);
            std::println("    Sum Cyclomatic Complexity: {}", cc_acc_metric.Get().sum);
            std::println("    Average Cyclomatic Complexity per function: {}", cc_acc_metric.Get().average);
            // Получаем и выводим результаты для количества строк кода
            auto &cl_acc_metric =
                accumulator.template GetFinalizedAccumulator<SumAverageAccumulator>(CodeLinesCountMetric::kName);
            std::println("    Sum Code lines count: {}", cl_acc_metric.Get().sum);
            std::println("    Average Code lines count per function: {}", cl_acc_metric.Get().average);
            // Получаем и выводим результаты для количества параметров
            auto &cp_acc_metric =
                accumulator.template GetFinalizedAccumulator<AverageAccumulator>(CountParametersMetric::kName);
            std::println("    Average Parameters count per function: {}", cp_acc_metric.Get());
            // Опционально: вывод результатов для стиля именования (пока закомментировано)
            // auto &naming_acc_metric =
            //     accumulator.template GetFinalizedAccumulator<CategoricalAccumulator>(NamingStyleMetric::kName);
            // std::ranges::for_each(naming_acc_metric.Get(), [](const auto &elem) {
            //     std::println("    Naming style '{}' is occured {} times", elem.first, elem.second);
            // });
        };

        // ==== 7. Агрегация по файлам ====
        // Группируем результаты анализа по файлам
        auto analysis_by_files = analyzer::SplitByFiles(analysis);

        // Для каждой группы (файла) накапливаем метрики и выводим результат
        std::ranges::for_each(analysis_by_files, [&accumulator, &print_accumulated_analysis](const auto &analysis) {
            analyzer::AccumulateFunctionAnalysis(analysis, accumulator);
            std::println();
            std::println("Accumulated Analysis for file {}:", analysis.front().first.filename);
            print_accumulated_analysis(accumulator);
            accumulator.ResetAccumulators();  // сбрасываем аккумуляторы для следующего файла
        });

        // ==== 8. Агрегация по классам ====
        // Группируем результаты анализа по классам (только методы классов)
        auto analysis_by_classes = analyzer::SplitByClasses(analysis);

        // Для каждой группы (класса) накапливаем метрики и выводим результат
        std::ranges::for_each(analysis_by_classes, [&accumulator, &print_accumulated_analysis](const auto &analysis) {
            analyzer::AccumulateFunctionAnalysis(analysis, accumulator);
            std::println();
            std::println("Accumulated Analysis for сlass {}:", analysis.front().first.class_name.value());
            print_accumulated_analysis(accumulator);
            accumulator.ResetAccumulators();  // сбрасываем аккумуляторы для следующего класса
        });

        // ==== 9. Общая агрегация по всем функциям ====
        // Накапливаем метрики для всех функций из всех файлов
        analyzer::AccumulateFunctionAnalysis(analysis, accumulator);
        std::println();
        std::println("Accumulated Analysis for All Functions:");
        print_accumulated_analysis(accumulator);
        return 0;

    }
    // Обрабатываем runtime_error (например, ошибки открытия файлов, ошибки tree-sitter)
    catch (const std::runtime_error &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }
    // Обрабатываем любые другие стандартные исключения
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    // Обрабатываем неизвестные исключения
    catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}
