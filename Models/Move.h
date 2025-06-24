#pragma once
#include <stdlib.h>

typedef int8_t POS_T;

struct move_pos
{
    // Координаты начальной позиции фигуры
    POS_T x, y;             // from (откуда)

    // Координаты конечной позиции фигуры  
    POS_T x2, y2;           // to (куда)

    // Координаты побитой фигуры (если есть)
    // (-1, -1) - если ход без взятия
    POS_T xb = -1, yb = -1; // beaten (побитая фигура)

    // Конструктор для обычного хода (без взятия)
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2)
        : x(x), y(y), x2(x2), y2(y2)
    {
    }

    // Конструктор для хода со взятием фигуры
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2,
        const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb)
    {
    }

    // Оператор сравнения ходов (без учета побитых фигур)
    bool operator==(const move_pos& other) const
    {
        return (x == other.x && y == other.y &&
            x2 == other.x2 && y2 == other.y2);
    }

    // Оператор неравенства
    bool operator!=(const move_pos& other) const
    {
        return !(*this == other);
    }
};
