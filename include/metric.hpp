#pragma once
#include <unistd.h>

#include <algorithm>
#include <any>
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

#include "function.hpp"

namespace fs = std::filesystem;
namespace rv = std::ranges::views;
namespace rs = std::ranges;

namespace analyzer::metric {

// Структура, хранящая результат вычисления одной метрики для функции
struct MetricResult {
    using ValueType = int;  // тип значения метрики (пока int, но может быть расширен до variant)
    // using ValueType = std::variant<int, std::string>; // если захотите реализовывать метрику
    // naming style
    std::string metric_name;  // название метрики (например, "Parameters count")
    ValueType value;          // вычисленное значение метрики
};

// Абстрактный базовый класс для всех метрик кода
// (интерфейс, от которого наследуются все конкретные метрики)
class IMetric {
public:
    // Виртуальный деструктор (для корректного удаления через указатель на базовый класс)
    virtual ~IMetric() = default;

    // Метод для вычисления метрики (не виртуальный, т.к. реализация общая для всех наследников)
    // Принимает функцию, возвращает структуру с результатом
    MetricResult Calculate(const function::Function &f) const {
        return MetricResult{.metric_name = Name(), .value = CalculateImpl(f)};
    }

protected:
    // === Виртуальные методы, которые должны реализовать наследники (чисто виртуальные функции) ===

    // Метод, реализующий вычисление метрики
    virtual MetricResult::ValueType CalculateImpl(const function::Function &f) const = 0;

    // Метод, возвращающий название метрики
    virtual std::string Name() const = 0;
};

// Псевдоним вектора результатов для нескольких метрик одной функции
using MetricResults = std::vector<MetricResult>;

// Класс-агрегатор, который хранит все зарегистрированные метрики и умеет вычислять их все для переданной функции
class MetricExtractor {
public:
    // Конструктор по умолчанию
    MetricExtractor() = default;

    // Реализуем правило пяти
    ~MetricExtractor() = default;                       // явно показываем, что не нужен особый деструктор
    MetricExtractor(const MetricExtractor &) = delete;  // запрещаем копирование (unique_ptr не копируется)
    MetricExtractor &operator=(const MetricExtractor &) = delete;
    MetricExtractor(MetricExtractor &&) = default;  // разрешаем перемещение
    MetricExtractor &operator=(MetricExtractor &&) = default;

    // Метод, регистрирующий новую метрику (добавляет ее в хранилище метрик)
    void RegisterMetric(std::unique_ptr<IMetric> metric);

    // Метод, вычисляющий все зарегистрированные метрики для переданной функции
    // Возвращает вектор результатов (по одному на каждую метрику)
    MetricResults Get(const function::Function &func) const;

private:
    std::vector<std::unique_ptr<IMetric>> metrics_;  // хранилище метрик (содержит полиморфные объекты)
};

}  // namespace analyzer::metric
