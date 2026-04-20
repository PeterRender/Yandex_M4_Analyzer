#include "file.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

namespace analyzer::file {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

// Конструктор, принимающий имя исходного анализируемого Python-файла
// (читает его содержимое и получает AST через tree-sitter)
File::File(const std::string &filename) : name_{filename} {
    std::ifstream file(name_);  // открываем файл для чтения

    if (!file.is_open()) {
        // Если файл не открывается, то бросаем исключение с понятным сообщением
        throw std::runtime_error("Can't open file: " + filename);
    }

    // Получаем AST дерево от tree-sitter
    ast_ = GetAst(filename);

    // Читаем исходные строки файла
    source_lines_ = ReadSourceFile(file);
}

// Метод для построчного чтения исходного Python-файла
std::vector<std::string> File::ReadSourceFile(std::ifstream &file) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

// Метод для получения AST-дерева от tree-sitter (использован синтаксис function-try-block)
std::string File::GetAst(const std::string &filename) try {
    // Формируем полную команду: префикс + имя_файла + "2>&1" (перенаправляем stderr в stdout для захвата ошибок)
    std::string full_cmd = File::command_prefix_ + filename + " 2>&1";
    std::string result;
    std::array<char, 256> buffer;

    // Используем unique_ptr с кастомным "удалителем" для автоматического закрытия pipe
    // ("удалитель" реализован как лямбда, которая закрывает pipe и проверяет код возврата)
    using PipePtr = std::unique_ptr<FILE, decltype([](FILE *pipe) {
                                        if (!pipe)
                                            return;

                                        int status = pclose(pipe);

                                        // Проверяем код возврата команды
                                        if (WIFEXITED(status)) {
                                            int exit_status = WEXITSTATUS(status);
                                            if (exit_status != 0) {
                                                throw std::runtime_error("Command failed with exit code " +
                                                                         std::to_string(exit_status));
                                            }
                                        } else {
                                            throw std::runtime_error("Command terminated abnormally");
                                        }
                                    })>;

    // Выполняем команду и открываем pipe для чтения её вывода
    FILE *raw_pipe = popen(full_cmd.c_str(), "r");
    if (!raw_pipe) {
        throw std::runtime_error("Failed to execute command: " + std::string(std::strerror(errno)));
    }
    PipePtr pipe(raw_pipe);  // unique_ptr автоматически закроет pipe при выходе из области видимости

    // Читаем вывод команды по блокам
    while (fgets(buffer.data(), buffer.size(), pipe.get())) {
        result += buffer.data();
    }

    return result;
} catch (const std::exception &e) {
    // Перехватываем любые исключения и добавляем имя анализируемого файла
    throw std::runtime_error("Error while getting ast from " + filename);
}

}  // namespace analyzer::file
