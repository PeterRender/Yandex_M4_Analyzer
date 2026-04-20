# AST структура Python-кода в tree-sitter
## Что такое AST?
AST (Abstract Syntax Tree) — это представление кода в виде иерархического дерева, где каждый узел соответствует конструкции языка (функция, класс, условие, цикл и т.д.).

## Основные узлы
### module (корневой узел)
Весь файл Python. Содержит все остальные узлы.
(module [start_row, start_col] - [end_row, end_col]
... дочерние узлы ...)

### class_definition
Определение класса.
(class_definition [start_row, start_col] - [end_row, end_col]
name: (identifier [start_row, start_col] - [end_row, end_col])      // имя класса
superclasses: (argument_list ...)                                   // родительские классы (если есть)
body: (block [start_row, start_col] - [end_row, end_col])           // тело класса
)

### function_definition
Определение функции.
(function_definition [start_row, start_col] - [end_row, end_col]
name: (identifier [start_row, start_col] - [end_row, end_col])          // имя функции
parameters: (parameters [start_row, start_col] - [end_row, end_col])    // список параметров
return_type: (type ...)                                                 // тип возврата (если есть)
body: (block [start_row, start_col] - [end_row, end_col])               // тело функции
)

## Параметры функций
### parameters
Список параметров функции. Может содержать разные типы узлов:

- (identifier ...)                  // простой параметр
- (typed_parameter ...)             // параметр с типом
- (typed_default_parameter ...)     // параметр с типом и значением по умолчанию

### Примеры из sample.py:
PYTHON:
def __init__(self, count: int = 42, name: str = "John"):

AST:
(function_definition [3, 4] - [5, 28]
  name: (identifier [3, 8] - [3, 16])           // "__init__"
  parameters: (parameters [3, 16] - [3, 68]
    (identifier [3, 17] - [3, 21])              // "self"
    (typed_default_parameter [3, 23] - [3, 46]
      name: (identifier [3, 23] - [3, 33])      // "count"
      type: (type [3, 35] - [3, 40])            // "int"
      value: (float [3, 43] - [3, 46])          // "42")
    (typed_default_parameter [3, 48] - [3, 67]
      name: (identifier [3, 48] - [3, 54])      // "name"
      type: (type [3, 56] - [3, 61])            // "str"
      value: (float [3, 64] - [3, 67])))        // "John"
)

## Тело функции (block)
body: (block [4, 8] - [5, 28]                   // тело init
(expression_statement [4, 8] - [4, 36])         // self.logger.info(count)
(expression_statement [5, 8] - [5, 28])         // self.value = name
)

## Конструкции для цикломатической сложности
| Конструкция | Узел в AST               | Пример из кода        |
|-------------|--------------------------|-----------------------|
| if          | "if_statement"           | "if x > 0:"           |
| elif        | "elif_clause"            | "elif x == 0:"        |
| else        | "else_clause"            | "else:"               |
| for         | "for_statement"          | "for i in range(10):" |
| while       | "while_statement"        | "while condition:"    |
| try         | "try_statement"          | "try:"                |
| except      | "except_clause"          | "except Exception:"   |
| finally     | "finally_clause"         | "finally:"            |
| with        | "with_statement"         | "with open(file):"    |
| match       | "match_statement"        | "match value:"        |
| case        | "case_clause"            | "case 42:"            |
| assert      | "assert_statement"       | "assert condition"    |
| and/or      | "boolean_operator"       | "x > 0 and y < 10"    |
| ternary     | "conditional_expression" | "x if cond else y"    |

### Как выглядят в AST:
(if_statement [3, 8] - [4, 8]           // if условие
condition: (comparison_operator ...)
consequence: (block ...)                // тело if
alternative: (block ...)                // else/elif (если есть)
)

(for_statement [10, 4] - [12, 8]        // for цикл
left: (identifier [10, 8] - [10, 9])    // переменная цикла
right: (call [10, 13] - [10, 24])       // диапазон
body: (block [11, 8] - [12, 8])         // тело цикла
)

## Позиции узлов
Каждый узел имеет позицию в формате "[строка, колонка] - [строка, колонка]":
- Первая пара = начало узла (строка, колонка)
- Вторая пара = конец узла (строка, колонка)

## Комментарии
(comment [22, 4] - [22, 47])    // строка с комментарием

## Как это использовать при подсчете метрик
### Для метрики "количество параметров":
1. Найти узел "function_definition"
2. Найти его дочерний узел "parameters"
3. Посчитать внутри него все узлы:
   - "identifier" (простой параметр)
   - "typed_parameter" (параметр с типом)
   - "typed_default_parameter" (параметр с типом и значением)

### Для метрики "цикломатическая сложность":
1. Начальное значение = 1
2. Обойти все узлы в "body" функции
3. Для каждого узла проверить его тип:
   - Если тип в таблице выше, то увеличить счетчик на 1

### Для метрики "количество строк кода":
1. Собрать все узлы в "body" функции
2. Для каждого узла взять номер строки ("start_point.row")
3. Убрать дубликаты (если на одной строке несколько узлов)
4. Исключить строки с комментариями (тип узла "comment")
5. Посчитать уникальные строки

## Полезные заметки
- Методы класса (в отличие от обычных функций) находятся внутри "class_definition"
- Первый параметр метода - "self", но это не обязательно
- Пустые параметры имеют вид типа "(parameters [18, 15] - [18, 17])" без дочерних узлов
- Цикломатическая сложность - число линейно независимых путей через код (min сложность = 1 путь, каждый if добавляет +1 путь)