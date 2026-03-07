#include "function.hpp"

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
#include "utils.hpp"

namespace fs = std::filesystem;
namespace rv = std::ranges::views;
namespace rs = std::ranges;

namespace analyzer::function {

// Метод, извлекающий из AST-дерева Python-файла структуры-описатели всех найденных функций
std::vector<Function> FunctionExtractor::Get(const analyzer::file::File &file) {

    // Алгоритм работы:
    // 1. Ищем в AST-строке все вхождения маркера "(function_definition"
    // 2. Для каждого найденного маркера выделяем подстроку, содержащую полное AST-поддерево функции
    // 3. Из этого поддерева извлекаем имя функции (по позиции узла identifier)
    // 4. Определяем, является ли функция методом класса (находится ли она внутри class_definition)
    // 5. Формируем структуру Function и добавляем в вектор-результат

    std::vector<Function> functions;
    size_t start = 0;                                   // текущая позиция поиска в AST-строке
    const std::string marker = "(function_definition";  // маркер начала определения функции
    const std::string &ast = file.GetAst();             // получаем AST-строку Python-файла

    // Цикл поиска всех вхождений маркера функции в AST-строке
    while ((start = ast.find(marker, start)) != std::string::npos) {
        // Нашли начало функции. Теперь нужно найти её конец (по балансу скобок).
        size_t open_braces = 1;                // начинаем с 1, так как уже нашли открывающую скобку маркера
        size_t end = start + marker.length();  // текущая позиция сразу после маркера

        // Идем по строке, считая открывающие и закрывающие скобки
        while (end < ast.size() && open_braces > 0) {
            if (ast[end] == '(')
                open_braces++;  // встретили открывающую скобку - уровень вложенности растет
            else if (ast[end] == ')')
                open_braces--;  // встретили закрывающую - уровень падает
            end++;
        }

        // Извлекаем подстроку, содержащую полное AST-поддерево этой функции
        auto func_ast = ast.substr(start, end - start);

        // Получаем данные о местоположении имени функции в исходном коде
        auto name_loc = GetNameLocation(func_ast);

        // По позиции извлекаем само имя функции из исходных строк файла
        std::string func_name = GetNameFromSource(func_ast, file.GetSourceLines());

        // Создаем структуру Function, пока без имени класса (оно задается позже)
        Function func{.filename = file.GetName(),  // имя файла, где найдена функция
                      .class_name = std::nullopt,  // пока не знаем, метод класса это или нет
                      .name = func_name,           // имя функции
                      .ast = func_ast};            // AST-поддерево функции

        // Пытаемся найти класс, который содержит эту функцию
        auto class_info = FindEnclosingClass(ast, name_loc);
        if (class_info) {
            // Если функция находится внутри класса, то извлекаем имя класса из исходных строк и записываем его
            // в структуру-описатель функции
            func.class_name = GetClassNameFromSource(*class_info, file.GetSourceLines());
        }

        functions.push_back(func);  // добавляем структуру-описатель функции в вектор-результат
        start = end;                // продолжаем поиск со следующей позиции
    }

    return functions;
}

// Метод, извлекающий позицию имени функции из её AST-поддерева
FunctionExtractor::FunctionNameLocation FunctionExtractor::GetNameLocation(const std::string &function_ast) {
    // AST-поддерево функции содержит узел identifier (это и есть имя функции),
    // позиция которого указана в виде пары координат "[строка1, колонка1] - [строка2, колонка2]"

    // Ищем первый identifier в AST-поддереве функции
    size_t id_pos = function_ast.find("(identifier");
    if (id_pos == std::string::npos)
        return {};  // если не нашли - возвращаем пустую структуру

    // Ищем начало первых координат - открывающую квадратную скобку после identifier
    size_t coord_start = function_ast.find('[', id_pos);
    size_t coord_end = function_ast.find(']', coord_start);
    std::string coords = function_ast.substr(coord_start + 1, coord_end - coord_start - 1);

    // Первые координаты имеют формат "строка, колонка"
    size_t comma = coords.find(',');
    Position start{
        static_cast<size_t>(ToInt(coords.substr(0, comma))),    // номер строки начала
        static_cast<size_t>(ToInt(coords.substr(comma + 2)))};  // номер колонки начала (пробел после запятой)

    // Ищем вторые координаты (после дефиса)
    size_t dash = function_ast.find('[', coord_end);
    size_t end_bracket = function_ast.find(']', dash);
    std::string end_coords = function_ast.substr(dash + 1, end_bracket - dash - 1);

    comma = end_coords.find(',');
    Position end{static_cast<size_t>(ToInt(end_coords.substr(0, comma))),    // номер строки конца
                 static_cast<size_t>(ToInt(end_coords.substr(comma + 2)))};  // номер колонки конца

    // Возвращаем структуру-описатель местоположения имени функции (имя пока пустое, заполняется позже)
    return {start, end, ""};
}

// Метод, извлекающий имя функции по его позиции из исходных строк Python-файла
std::string FunctionExtractor::GetNameFromSource(const std::string &function_ast,
                                                 const std::vector<std::string> &lines) {
    // Извлекаем позицию имени функции из её AST-поддерева
    auto loc = GetNameLocation(function_ast);
    if (loc.start.line >= lines.size())  // проверяем, что номер строки в пределах файла
        return "unknown";

    // Получаем ссылку на строку Python-файла, содержащую начало имени функции
    const std::string &target_line = lines[loc.start.line];
    if (loc.start.col >= target_line.size())  // проверяем, что номер столбца в пределах файла
        return "unknown";

    // Извлекаем подстроку из исходной строки по координатам
    return target_line.substr(loc.start.col, loc.end.col - loc.start.col);
}

// Метод, определяющий, находится ли функция внутри какого-то класса (по позициям в AST-дереве всего файла)
std::optional<FunctionExtractor::ClassInfo>
FunctionExtractor::FindEnclosingClass(const std::string &ast, const FunctionNameLocation &func_loc) {

    // Для этого ищем все классы в AST-дереве файла и проверяем, попадает ли позиция функции внутрь класса
    size_t class_pos = 0;                                  // текущая позиция поиска в AST-строке
    const std::string class_marker = "(class_definition";  // маркер начала определения класса
    std::optional<ClassInfo> last_enclosing_class;

    // Перебираем все классы в AST-дереве файла
    while ((class_pos = ast.find(class_marker, class_pos)) != std::string::npos) {
        // Находим позицию начала класса (первые координаты)
        size_t coord_start = ast.find('[', class_pos);
        size_t coord_end = ast.find(']', coord_start);
        std::string coords = ast.substr(coord_start + 1, coord_end - coord_start - 1);

        // Первые координаты имеют формат "строка, колонка"
        size_t comma = coords.find(',');
        Position class_start{static_cast<size_t>(ToInt(coords.substr(0, comma))),  // номер строки начала
                             static_cast<size_t>(ToInt(coords.substr(
                                 comma + 1)))};  // номер колонки начала (здесь нет пробела после запятой!)

        // Ищем вторые координаты (после дефиса)
        size_t dash = ast.find('-', coord_end);
        size_t second_coord_start = ast.find('[', dash);
        size_t second_coord_end = ast.find(']', second_coord_start);
        std::string end_coords = ast.substr(second_coord_start + 1, second_coord_end - second_coord_start - 1);

        comma = end_coords.find(',');
        Position class_end{static_cast<size_t>(ToInt(end_coords.substr(0, comma))),    // номер строки конца
                           static_cast<size_t>(ToInt(end_coords.substr(comma + 1)))};  // номер колонки конца

        // Проверяем, попадает ли позиция функции внутрь этого класса
        // Функция считается принадлежащей классу, если её начало находится между началом и концом класса
        bool after_class_start = (func_loc.start.line > class_start.line) ||
                                 (func_loc.start.line == class_start.line && func_loc.start.col >= class_start.col);
        bool before_class_end = (func_loc.start.line < class_end.line) ||
                                (func_loc.start.line == class_end.line && func_loc.start.col <= class_end.col);

        if (after_class_start && before_class_end) {
            // Функция внутри этого класса. Теперь нужно проверить, что у класса есть имя
            // Ищем узел "name:" внутри определения класса
            size_t name_start = ast.find("name:", coord_end);
            if (name_start != std::string::npos) {
                // Нашли "name:", теперь ищем identifier (имя класса)
                size_t id_start = ast.find("(identifier", name_start);
                if (id_start != std::string::npos) {
                    // Функция внутри этого класса и у класса есть имя - сохраняем информацию о классе
                    ClassInfo class_info;
                    class_info.start = class_start;
                    class_info.end = class_end;
                    last_enclosing_class = class_info;
                }
            }
        }

        class_pos = second_coord_end;  // продолжаем поиск следующих классов
    }

    return last_enclosing_class;
}

// Метод, извлекающий имя класса из исходных строк Python-файла (по структуре-описателю класса)
std::string FunctionExtractor::GetClassNameFromSource(const ClassInfo &class_info,
                                                      const std::vector<std::string> &lines) {
    if (class_info.start.line >= lines.size())
        return "unknown";

    const std::string &class_line = lines[class_info.start.line];

    // Ищем ключевое слово "class"
    size_t class_pos = class_line.find("class");
    if (class_pos == std::string::npos)
        return "unknown";

    // Пропускаем пробелы после "class"
    size_t name_start = class_line.find_first_not_of(" \t", class_pos + 5);
    if (name_start == std::string::npos)
        return "unknown";

    // Ищем конец имени класса (до пробела, двоеточия, открывающей скобки или фигурной скобки)
    size_t name_end = class_line.find_first_of(" :{(", name_start);
    if (name_end == std::string::npos)
        name_end = class_line.length();

    return class_line.substr(name_start, name_end - name_start);
}

}  // namespace analyzer::function
