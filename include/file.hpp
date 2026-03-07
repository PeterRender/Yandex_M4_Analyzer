#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

namespace analyzer::file {

// Класс, инкапсулирующий всю информацию об анализируемом Python-файле:
// - имя файла
// - AST дерево, полученное от tree-sitter
// - исходные строки файла (для извлечения имен функций/классов по позициям)
class File {
public:
    // Конструктор, принимающий имя исходного анализируемого Python-файла
    // (может бросить исключение std::invalid_argument или std::runtime_error)
    explicit File(const std::string &filename);

    // Реализуем правило пяти
    ~File() = default;            // явно показываем, что не нужен особый деструктор
    File(const File &) = delete;  // запрещаем копирование (чтобы случайно не дублировать тяжелые строки AST-дерева)
    File &operator=(const File &) = delete;
    File(File &&) = default;  // разрешаем перемещение
    File &operator=(File &&) = default;

    // Методы доступа к данным (только для чтения)
    const std::string &GetName() const noexcept { return name_; }
    const std::string &GetAst() const noexcept { return ast_; }
    const std::vector<std::string> &GetSourceLines() const noexcept { return source_lines_; }

    // Статический метод для получения префикса команды
    static const std::string &GetCommandPrefix() noexcept { return command_prefix_; }

private:
    std::string name_;                       // имя исходного Python-файла (путь к нему)
    std::string ast_;                        // AST-дерево в виде S-выражения (текстовый вывод tree-sitter parse)
    std::vector<std::string> source_lines_;  // построчное содержимое исходного Python-файла (для извлечения имен)

    // Статический префикс команды для вызова tree-sitter (общий для всех экземпляров)
    // Используется для формирования полной команды: префикс + имя_файла
    // static - потому что общий для всех экземпляров
    // inline - чтобы можно было инициализировать прямо в классе (C++17)
    static inline const std::string command_prefix_ =
        "tree-sitter parse --config-path /root/.config/tree-sitter/config.json ";

    // Метод для построчного чтения исходного Python-файла
    std::vector<std::string> ReadSourceFile(std::ifstream &file);

    // Метод для получения AST-дерева от tree-sitter
    std::string GetAst(const std::string &filename);
};

}  // namespace analyzer::file
