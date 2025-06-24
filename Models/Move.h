#pragma once
#include <stdlib.h>  // ƒл€ использовани€ int8_t

// “ип дл€ хранени€ координат на доске(8x8
typedef int8_t POS_T;  // 8-битное целое со знаком -128..127

// —труктура, описывающа€ ход в игре
struct move_pos
{
    //  оординаты начальной позиции фигуры
    POS_T x, y;             // from откуда

    //  оординаты конечной позиции фигуры  
    POS_T x2, y2;           // to куда

    //  оординаты побитой фигуры если есть
    // (-1, -1) - если ход без вз€ти€
    POS_T xb = -1, yb = -1; // beaten побита€ фигура

    //  онструктор дл€ обычного хода 
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2)
        : x(x), y(y), x2(x2), y2(y2)
    {
    }

    //  онструктор дл€ хода со вз€тием фигуры
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2,
        const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb)
    {
    }

    // ќператор сравнени€ ходов 
    bool operator==(const move_pos& other) const
    {
        return (x == other.x && y == other.y &&
            x2 == other.x2 && y2 == other.y2);
    }

    // ќператор неравенства
    bool operator!=(const move_pos& other) const
    {
        return !(*this == other);
    }
};