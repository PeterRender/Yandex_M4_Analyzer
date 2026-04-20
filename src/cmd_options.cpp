#include "cmd_options.hpp"

#include <exception>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

namespace analyzer::cmd {

namespace po = boost::program_options;

// Конструктор по умолчанию
ProgramOptions::ProgramOptions() : desc_("Allowed options") {
    // Добавляем в хранилище две опции командной строки приложения Analyser:
    // 1. --help, -h - вывод справки (необязательная)
    // 2. --file, -f - список файлов для анализа (обязательная, может принимать несколько значений)
    desc_.add_options()("help,h", "Display help message")(
        "file,f", po::value<std::vector<std::string>>(&files_)->required()->multitoken(),
        "List of files to process (required)");
}

// Деструктор
ProgramOptions::~ProgramOptions() = default;

// Метод, реализующий парсинг (разбор) аргументов командной строки (argc, argv)
bool ProgramOptions::Parse(int argc, char *argv[]) {
    try {
        po::variables_map vm;  // контейнер, в который будут сохраняться распознанные опции командной строки

        // Парсим командную строку с использованием нашего хранилища опций
        po::store(po::command_line_parser(argc, argv).options(desc_).run(), vm);

        // Если пользователь запросил справку (опция --help или -h)
        if (vm.count("help")) {
            desc_.print(std::cout);  // выводим описание всех доступных опций
            return false;            // возвращаем false, так как программе не нужно выполнять основной функционал
        }

        // Проверяем наличие всех обязательных опций (в нашем случае --file)
        // Если обязательной опции нет, будет выброшено исключение
        po::notify(vm);

        // Проверяем, что список файлов для анализа не пуст
        if (files_.empty()) {
            std::cerr << "Error: At least one file must be specified\n";
            desc_.print(std::cout);
            return false;
        }

        return true;  // парсинг успешен, можно запускать основной функционал
    }
    // Обрабатываем cпецифичные ошибки boost::program_options
    catch (const po::error &e) {
        std::cerr << "Error parsing command line: " << e.what() << "\n";
        desc_.print(std::cout);
        return false;
    }
    // Обрабатываем runtime-ошибки (выводим справку с правильным синтаксисом)
    catch (const std::runtime_error &e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        desc_.print(std::cout);
        return false;
    }
    // Обрабатываем любые другие стандартные исключения (выводим справку с правильным синтаксисом)
    catch (const std::exception &e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
        desc_.print(std::cout);
        return false;
    }
}

}  // namespace analyzer::cmd
