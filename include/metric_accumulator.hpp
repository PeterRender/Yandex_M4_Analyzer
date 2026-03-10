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

#include "metric.hpp"

namespace analyzer::metric_accumulator {

// Абстрактный базовый класс для всех аккумуляторов метрик
// Аккумулятор накапливает значения метрик (например, считает сумму, среднее,
// частоту встречаемости) при обработке нескольких функций
class IAccumulator {
public:
    // Виртуальный деструктор (для корректного удаления через указатель на базовый класс)
    virtual ~IAccumulator() = default;

    // Метод, накапливающий очередное значение метрики
    // (вызывается для каждой функции в обрабатываемой группе (файл, класс или все функции))
    virtual void Accumulate(const metric::MetricResult &metric_result) = 0;

    // Метод, финализирующий накопление (вычисляет итоговое значение: среднее, сумму и т.д.)
    // (должен вызываться после обработки всех функций группы)
    virtual void Finalize() = 0;

    // Метод, сбрасывающий состояние аккумулятора
    // (вызывается перед началом обработки новой группы)
    virtual void Reset() = 0;

protected:
    bool is_finalized = false;  // флаг, указывающий, был ли вызван Finalize
};

// Класс-агрегатор, который хранит все зарегистрированные аккумуляторы
// (по одному на каждую метрику) и умеет передавать им результаты вычислений
class MetricsAccumulator {
public:
    // Конструктор по умолчанию
    MetricsAccumulator() = default;

    // Реализуем правило пяти
    ~MetricsAccumulator() = default;
    MetricsAccumulator(const MetricsAccumulator &) = delete;  // запрещаем копирование
    MetricsAccumulator &operator=(const MetricsAccumulator &) = delete;
    MetricsAccumulator(MetricsAccumulator &&) = default;  // разрешаем перемещение
    MetricsAccumulator &operator=(MetricsAccumulator &&) = default;

    // Шаблонный метод, регистрирующий аккумулятор для конкретной метрики
    // (принимает имя метрики и unique_ptr на аккумулятор)
    template <typename Accumulator>
    void RegisterAccumulator(const std::string &metric_name, std::unique_ptr<Accumulator> acc) {
        accumulators_.emplace(metric_name, std::move(acc));
    }
    // Шаблонный метод, получающий финализированный аккумулятор по имени метрики
    // (вызывает Finalize() перед возвратом (если еще не был вызван))
    template <typename Accumulator>
    const Accumulator &GetFinalizedAccumulator(const std::string &metric_name) const {
        auto metric_accululator = accumulators_.at(metric_name);
        metric_accululator->Finalize();
        return dynamic_cast<const Accumulator &>(*metric_accululator);
    }

    // Метод, передающий результаты вычисления метрик для одной функции всем зарегистрированным аккумуляторам
    // (для каждого результата (по metric_name) находит соответствующий аккумулятор и вызывает его метод Accumulate)
    void AccumulateNextFunctionResults(const std::vector<metric::MetricResult> &metric_results) const;

    // Метод, сбрасывающий состояние всех зарегистрированных аккумуляторов
    // (вызывается перед началом обработки новой группы (например, нового файла))
    void ResetAccumulators();

private:
    // Хранилище аккумуляторов: ключ - имя метрики, значение - указатель на аккумулятор
    // (используется shared_ptr вместо unique_ptr, чтобы можно было возвращать const ссылку
    // из GetFinalizedAccumulator и при этом копировать аккумулятор нельзя)
    std::unordered_map<std::string, std::shared_ptr<IAccumulator>> accumulators_;
};

}  // namespace analyzer::metric_accumulator
