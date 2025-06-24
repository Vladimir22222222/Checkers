#pragma once
#include <chrono>
#include <thread>

#include "../Models/Project_path.h"
#include "Board.h"
#include "Config.h"
#include "Hand.h"
#include "Logic.h"

class Game
{
  public:
      // Конструктор: инициализирует доску, обработчик ввода и логику игры
    Game() : board(config("WindowSize", "Width"), config("WindowSize", "Hight")), hand(&board), logic(&board, &config)
    {
        // Очистка файла логов при создании игры
        ofstream fout(project_path + "log.txt", ios_base::trunc);
        fout.close();
    }

    // Основная функция игры
    int play()
    {
        auto start = chrono::steady_clock::now();
        // Обработка режима повтора игры
        if (is_replay)
        {
            logic = Logic(&board, &config);
            config.reload();
            board.redraw();
        }
        else
        {
            board.start_draw();
        }
        is_replay = false;

        int turn_num = -1;
        bool is_quit = false;
        const int Max_turns = config("Game", "MaxNumTurns");
        // Главный игровой цикл
        while (++turn_num < Max_turns)
        {
            beat_series = 0;
            // Поиск возможных ходов для текущего игрока
            logic.find_turns(turn_num % 2);
            // Если ходов нет - игра окончена
            if (logic.turns.empty())
                break;
            // Установка уровня сложности бота
            logic.Max_depth = config("Bot", string((turn_num % 2) ? "Black" : "White") + string("BotLevel"));
            // Ход игрока или бота
            if (!config("Bot", string("Is") + string((turn_num % 2) ? "Black" : "White") + string("Bot")))
            {
                auto resp = player_turn(turn_num % 2);
                if (resp == Response::QUIT)
                {
                    is_quit = true;
                    break;
                }
                else if (resp == Response::REPLAY)
                {
                    is_replay = true;
                    break;
                }
                else if (resp == Response::BACK)
                {
                    // Отмена хода (если возможно)
                    if (config("Bot", string("Is") + string((1 - turn_num % 2) ? "Black" : "White") + string("Bot")) &&
                        !beat_series && board.history_mtx.size() > 2)
                    {
                        board.rollback();
                        --turn_num;
                    }
                    if (!beat_series)
                        --turn_num;

                    board.rollback();
                    --turn_num;
                    beat_series = 0;
                }
            }
            else
                bot_turn(turn_num % 2);
        }
        // Запись времени игры в лог
        auto end = chrono::steady_clock::now();
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Game time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();

        // Обработка повторной игры
        if (is_replay)
            return play();
        if (is_quit)
            return 0;
        // Определение результата игры
        int res = 2;
        if (turn_num == Max_turns)
        {
            res = 0;
        }
        else if (turn_num % 2)
        {
            res = 1;
        }
        // Отображение результата и ожидание реакции игрока
        board.show_final(res);
        auto resp = hand.wait();
        if (resp == Response::REPLAY)
        {
            is_replay = true;
            return play();
        }
        return res;
    }

  private:
      // Функция выполнения хода бота
    void bot_turn(const bool color)
    {
        auto start = chrono::steady_clock::now();

        // Искусственная задержка для визуализации хода
        auto delay_ms = config("Bot", "BotDelayMS");
        // new thread for equal delay for each turn
        thread th(SDL_Delay, delay_ms);
        // Поиск лучших ходов
        auto turns = logic.find_best_turns(color);
        th.join();
        bool is_first = true;
        // Выполнение найденных ходов
        for (auto turn : turns)
        {
            if (!is_first)
            {
                SDL_Delay(delay_ms);
            }
            is_first = false;
            beat_series += (turn.xb != -1);
            board.move_piece(turn, beat_series);
        }

        // Запись времени хода в лог
        auto end = chrono::steady_clock::now();
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Bot turn time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();
    }

    // Функция выполнения хода игрока
    Response player_turn(const bool color)
    {
        // Подсветка возможных начальных позиций
        vector<pair<POS_T, POS_T>> cells;
        for (auto turn : logic.turns)
        {
            cells.emplace_back(turn.x, turn.y);
        }
        board.highlight_cells(cells);
        move_pos pos = {-1, -1, -1, -1};
        POS_T x = -1, y = -1;
        // Выбор начальной позиции фигуры
        while (true)
        {
            auto resp = hand.get_cell();
            if (get<0>(resp) != Response::CELL)
                return get<0>(resp);
            pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};

            bool is_correct = false;
            // Проверка корректности выбранной клетки
            for (auto turn : logic.turns)
            {
                if (turn.x == cell.first && turn.y == cell.second)
                {
                    is_correct = true;
                    break;
                }
                if (turn == move_pos{x, y, cell.first, cell.second})
                {
                    pos = turn;
                    break;
                }
            }
            if (pos.x != -1)
                break;
            if (!is_correct)
            {
                if (x != -1)
                {
                    board.clear_active();
                    board.clear_highlight();
                    board.highlight_cells(cells);
                }
                x = -1;
                y = -1;
                continue;
            }
            // Подсветка возможных ходов для выбранной фигуры
            x = cell.first;
            y = cell.second;
            board.clear_highlight();
            board.set_active(x, y);
            vector<pair<POS_T, POS_T>> cells2;
            for (auto turn : logic.turns)
            {
                if (turn.x == x && turn.y == y)
                {
                    cells2.emplace_back(turn.x2, turn.y2);
                }
            }
            board.highlight_cells(cells2);
        }
        // Выполнение хода
        board.clear_highlight();
        board.clear_active();
        board.move_piece(pos, pos.xb != -1);
        if (pos.xb == -1)
            return Response::OK;
        // Обработка серии взятий
        beat_series = 1;
        while (true)
        {
            logic.find_turns(pos.x2, pos.y2);
            if (!logic.have_beats)
                break;

            // Подсветка возможных продолжений взятия
            vector<pair<POS_T, POS_T>> cells;
            for (auto turn : logic.turns)
            {
                cells.emplace_back(turn.x2, turn.y2);
            }
            board.highlight_cells(cells);
            board.set_active(pos.x2, pos.y2);
            // Выбор продолжения хода
            while (true)
            {
                auto resp = hand.get_cell();
                if (get<0>(resp) != Response::CELL)
                    return get<0>(resp);
                pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};

                bool is_correct = false;
                for (auto turn : logic.turns)
                {
                    if (turn.x2 == cell.first && turn.y2 == cell.second)
                    {
                        is_correct = true;
                        pos = turn;
                        break;
                    }
                }
                if (!is_correct)
                    continue;

                // Выполнение продолжения хода
                board.clear_highlight();
                board.clear_active();
                beat_series += 1;
                board.move_piece(pos, beat_series);
                break;
            }
        }

        return Response::OK;
    }

  private:
      Config config;       // Конфигурация игры
      Board board;         // Игровая доска
      Hand hand;           // Обработчик ввода
      Logic logic;         // Игровая логика
      int beat_series;     // Счетчик серии взятий
      bool is_replay = false; // Флаг режима повтора игры
};
