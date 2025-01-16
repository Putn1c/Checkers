#pragma once
#include <stdlib.h>

typedef int8_t POS_T; // Определение типа POS_T как 8-битного целого числа со знаком для использования в координатах

// Структура для описания положения и перемещения фигуры
struct move_pos
{
    POS_T x, y;             // Начальные координаты (откуда)
    POS_T x2, y2;           // Конечные координаты (куда)
    POS_T xb = -1, yb = -1; // Координаты побежденной фигуры (-1, -1 означает, что побежденной фигуры нет)

    // Конструктор для перемещения без побежденных фигур
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2) : x(x), y(y), x2(x2), y2(y2)
    {
    }

    // Конструктор для перемещения с побежденной фигурой
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2, const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb)
    {
    }

    // Оператор сравнения для проверки равенства двух перемещений
    bool operator==(const move_pos& other) const
    {
        return (x == other.x && y == other.y && x2 == other.x2 && y2 == other.y2); // Сравнение всех координат
    }

    // Оператор неравенства для проверки неравенства двух перемещений
    bool operator!=(const move_pos& other) const
    {
        return !(*this == other); // Неравенство определяется через оператор равенства
    }
};
