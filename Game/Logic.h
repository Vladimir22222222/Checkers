#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic {
public:
    /**
     * Находит лучшую последовательность ходов для заданного цвета
     * @param color цвет игрока (true - белые, false - черные)
     * @return вектор лучших ходов
     */
    vector<move_pos> find_best_turns(const bool color) {
        next_best_state.clear();
        next_move.clear();

        // Начинаем поиск с корневой позиции
        find_first_best_turn(board->get_board(), color, -1, -1, 0);

        // Собираем цепочку лучших ходов
        vector<move_pos> result;
        int current_state = 0;
        while (current_state != -1 && next_move[current_state].x != -1) {
            result.push_back(next_move[current_state]);
            current_state = next_best_state[current_state];
        }
        return result;
    }

private:
    /**
     * Находит первый лучший ход (начало рекурсии)
     * @param mtx текущее состояние доски
     * @param color текущий игрок
     * @param x,y координаты фигуры (если есть взятие)
     * @param state текущее состояние
     * @param alpha значение для альфа-бета отсечения
     * @return оценка позиции
     */
    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color,
        const POS_T x, const POS_T y, size_t state,
        double alpha = -1) {
        // Инициализация
        next_best_state.push_back(-1);
        next_move.emplace_back(-1, -1, -1, -1);
        double best_score = -1;

        // Поиск возможных ходов
        if (state != 0) find_turns(x, y, mtx);
        auto current_turns = turns;
        bool has_captures = have_beats;

        // Если нет взятий - переходим к обычному поиску
        if (!has_captures && state != 0) {
            return find_best_turns_rec(mtx, !color, 0, alpha);
        }

        // Перебор всех возможных ходов
        for (auto& turn : current_turns) {
            size_t next_state = next_move.size();
            double score;

            if (has_captures) {
                score = find_first_best_turn(make_turn(mtx, turn), color,
                    turn.x2, turn.y2, next_state, best_score);
            }
            else {
                score = find_best_turns_rec(make_turn(mtx, turn), !color, 0, best_score);
            }

            // Обновление лучшего хода
            if (score > best_score) {
                best_score = score;
                next_best_state[state] = has_captures ? next_state : -1;
                next_move[state] = turn;
            }
        }

        return best_score;
    }

    /**
     * Рекурсивный поиск с альфа-бета отсечением
     * @param mtx состояние доски
     * @param color текущий игрок
     * @param depth глубина рекурсии
     * @param alpha,beta параметры отсечения
     * @param x,y координаты фигуры (если есть взятие)
     * @return оценка позиции
     */
    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color,
        const size_t depth, double alpha = -1,
        double beta = INF + 1, const POS_T x = -1,
        const POS_T y = -1) {
        // Базовый случай - достигнута максимальная глубина
        if (depth == Max_depth) {
            return calc_score(mtx, depth % 2 == color);
        }

        // Поиск возможных ходов
        if (x != -1) find_turns(x, y, mtx);
        else find_turns(color, mtx);

        auto current_turns = turns;
        bool has_captures = have_beats;

        // Обработка случая, когда нет взятий
        if (!has_captures && x != -1) {
            return find_best_turns_rec(mtx, !color, depth + 1, alpha, beta);
        }

        // Если нет ходов - терминальное состояние
        if (turns.empty()) return (depth % 2 ? 0 : INF);

        // Основной цикл оценки ходов
        double min_score = INF + 1;
        double max_score = -1;

        for (auto& turn : current_turns) {
            double score;

            if (!has_captures && x == -1) {
                score = find_best_turns_rec(make_turn(mtx, turn), !color,
                    depth + 1, alpha, beta);
            }
            else {
                score = find_best_turns_rec(make_turn(mtx, turn), color, depth,
                    alpha, beta, turn.x2, turn.y2);
            }

            // Обновление оценок
            min_score = min(min_score, score);
            max_score = max(max_score, score);

            // Альфа-бета отсечение
            if (depth % 2) alpha = max(alpha, max_score);
            else beta = min(beta, min_score);

            if (optimization != "O0" && alpha == beta) {
                return (depth % 2 ? max_score + 1 : min_score - 1);
            }
        }

        return (depth % 2 ? max_score : min_score);
    }

public:
 /*
 Находит все возможные ходы для фигур указанного цвета
 @param color цвет фигур, для которых ищутся ходы
 */
    void find_turns(const bool color)
    {
        find_turns(color, board->get_board());
    }

 /*
 Находит все возможные ходы для фигуры в указанной позиции
 @param x координата X фигуры
 @param y координата Y фигуры
 */
    void find_turns(const POS_T x, const POS_T y)
    {
        find_turns(x, y, board->get_board());
    }

private:
 /*
  Находит все возможные ходы для фигур указанного цвета на заданной доске
  @param color цвет фигур
  @param mtx состояние доски
 */
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx)
    {
        vector<move_pos> res_turns;
        bool have_beats_before = false;
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (mtx[i][j] && mtx[i][j] % 2 != color)
                {
                    find_turns(i, j, mtx);
                    if (have_beats && !have_beats_before)
                    {
                        have_beats_before = true;
                        res_turns.clear();
                    }
                    if ((have_beats_before && have_beats) || !have_beats_before)
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }
        turns = res_turns;
        shuffle(turns.begin(), turns.end(), rand_eng);
        have_beats = have_beats_before;
    }

    /*
      Находит все возможные ходы для фигуры в указанной позиции на заданной доске
      @param x координата X фигуры
      @param y координата Y фигуры
      @param mtx состояние доски
     */
    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx)
    {
        turns.clear();
        have_beats = false;
        POS_T type = mtx[x][y];
        // check beats
        switch (type)
        {
        case 1:
        case 2:
            // check pieces
            for (POS_T i = x - 2; i <= x + 2; i += 4)
            {
                for (POS_T j = y - 2; j <= y + 2; j += 4)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7)
                        continue;
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2;
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)
                        continue;
                    turns.emplace_back(x, y, i, j, xb, yb);
                }
            }
            break;
        default:
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1;
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break;
                            }
                            xb = i2;
                            yb = j2;
                        }
                        if (xb != -1 && xb != i2)
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb);
                        }
                    }
                }
            }
            break;
        }
        // check other turns
        if (!turns.empty())
        {
            have_beats = true;
            return;
        }
        switch (type)
        {
        case 1:
        case 2:
            // check pieces
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1);
                for (POS_T j = y - 1; j <= y + 1; j += 2)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                        continue;
                    turns.emplace_back(x, y, i, j);
                }
                break;
            }
        default:
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                            break;
                        turns.emplace_back(x, y, i2, j2);
                    }
                }
            }
            break;
        }
    }

  public:
    // Список возможных ходов
    vector<move_pos> turns;
    // Флаг, указывающий есть ли взятия в списке ходов
    bool have_beats;
    // Максимальная глубина рекурсии при поиске ходов
    int Max_depth;

  private:
    // Генератор случайных чисел для перемешивания ходов
    default_random_engine rand_eng;
    // Режим подсчета очков ("NumberAndPotential" или другой)
    string scoring_mode;
    // Уровень оптимизации ("O0" или другой)
    string optimization;
    // Список следующих лучших состояний для построения цепочки ходов
    vector<move_pos> next_move;
    // Список индексов следующих лучших состояний
    vector<int> next_best_state;
    // Указатель на объект доски
    Board *board;
    // Указатель на объект конфигурации
    Config *config;
};
