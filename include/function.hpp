#pragma once
#include <unistd.h>

#include <algorithm>
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

#include "file.hpp"

namespace fs = std::filesystem;
namespace rv = std::ranges::views;
namespace rs = std::ranges;

namespace analyzer::function {

// Структура, хранящая метаданные об одной функции/методе из анализируемого Python-файла
struct Function {
    std::string filename;                   // имя файла, где определена функция
    std::optional<std::string> class_name;  // имя класса, если это метод (иначе std::nullopt)
    std::string name;                       // имя функции
    std::string ast;                        // AST-поддерево только для этой функции
};

// Класс-хелпер, извлекающий все функции из AST-дерева всего Python-файла
// Он парсит S-выражение, находит все узлы (function_definition и для каждого
// определяет, является ли функция методом класса, и извлекает её AST-поддерево (AST-подстроку)
class FunctionExtractor {
public:
    // Конструктор по умолчанию
    FunctionExtractor() = default;  // явно просим компилятор сгенерировать конструктор

    // Реализуем правило пяти
    ~FunctionExtractor() = default;                         // явно показываем, что не нужен особый деструктор
    FunctionExtractor(const FunctionExtractor &) = delete;  // запрещаем копирование
    FunctionExtractor &operator=(const FunctionExtractor &) = delete;
    FunctionExtractor(FunctionExtractor &&) = default;  // разрешаем перемещение
    FunctionExtractor &operator=(FunctionExtractor &&) = default;

    // Метод, извлекающий из AST-дерева Python-файла структуры-описатели всех найденных функций
    std::vector<Function> Get(const analyzer::file::File &file);

private:
    // Структура, хранящая позиции узлов AST-дерева в исходном коде
    struct Position {
        size_t line;  // номер строки в исходном коде
        size_t col;   // номер колонки в исходном коде
    };

    // Структура, хранящая данные о местоположении имени функции в исходном коде
    struct FunctionNameLocation {
        Position start;    // начало имени функции (строка, колонка)
        Position end;      // конец имени функции (строка, колонка)
        std::string name;  // имя функции
    };

    // Структура, хранящая данные о классе (для определения, принадлежит ли функция классу)
    struct ClassInfo {
        std::string name;  // имя класса
        Position start;    // начало определения класса
        Position end;      // конец определения класса
    };

    // Метод, извлекающий позицию имени функции из её AST-поддерева
    FunctionNameLocation GetNameLocation(const std::string &function_ast);

    // Метод, извлекающий имя функции по его позиции из исходных строк Python-файла
    std::string GetNameFromSource(const std::string &function_ast, const std::vector<std::string> &lines);

    // Метод, определяющий, находится ли функция внутри какого-то класса (по позициям в AST-дереве всего файла)
    std::optional<ClassInfo> FindEnclosingClass(const std::string &ast, const FunctionNameLocation &func_loc);

    // Метод, извлекающий имя класса из исходных строк Python-файла (по структуре-описателю класса)
    std::string GetClassNameFromSource(const ClassInfo &class_info, const std::vector<std::string> &lines);
};

}  // namespace analyzer::function
