#pragma once

#include <string>

#include <boost/program_options.hpp>

namespace analyzer::cmd {

// Класс для обработки аргументов командной строки приложения Analyser
// (использует библиотеку boost::program_options для парсинга параметров)
class ProgramOptions {
public:
    // Конструктор по умолчанию
    ProgramOptions();

    // Деструктор
    ~ProgramOptions();

    // Метод, реализующий парсинг (разбор) аргументов командной строки (argc, argv)
    // (возвращает true, если парсинг успешен и можно продолжать работу,
    // возвращает false, если запрошена справка (--help) или произошла ошибка
    bool Parse(int argc, char *argv[]);

    // Возвращает список файлов для анализа, переданных через опцию --file (-f)
    const std::vector<std::string> &GetFiles() const { return files_; }

private:
    std::vector<std::string> files_;  // cписок файлов для анализа (обязательный параметр)

    // Хранилище допустимых опций командной строки приложения Analyser
    // (используется для парсинга фактических аргументов)
    boost::program_options::options_description desc_;
};

}  // namespace analyzer::cmd
