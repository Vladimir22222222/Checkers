#pragma once
#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// Класс для обработки пользовательского ввода (мышь, окно)
class Hand
{
  public:
      // Конструктор, принимает указатель на доску
    Hand(Board *board) : board(board)
    {
    }
    // Основной метод обработки ввода - возвращает кортеж из:
    // 1. Тип действия (Response)
    // 2. X-координата клетки (если действие CELL)
    // 3. Y-координата клетки (если действие CELL)
    tuple<Response, POS_T, POS_T> get_cell() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        int x = -1, y = -1;
        int xc = -1, yc = -1;
        while (true)
        {
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type)
                {
                    // Обработка закрытия окна
                case SDL_QUIT:
                    resp = Response::QUIT;
                    break;
                    // Обработка клика мыши
                case SDL_MOUSEBUTTONDOWN:
                    x = windowEvent.motion.x;
                    y = windowEvent.motion.y;
                    // Преобразование координат экрана в координаты доски
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);
                    // Обработка кнопки "Назад" (левый верхний угол)
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1)
                    {
                        resp = Response::BACK;
                    }
                    // Обработка кнопки "Повторить" (правый верхний угол)
                    else if (xc == -1 && yc == 8)
                    {
                        resp = Response::REPLAY;
                    }
                    // Обработка клика по клетке доски
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8)
                    {
                        resp = Response::CELL;
                    }
                    // Клик вне значимых областей
                    else 
                    {
                        xc = -1;
                        yc = -1;
                    }
                    break;
                    // Обработка изменения размера окна// Обработка изменения размера окна
                case SDL_WINDOWEVENT:
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        board->reset_window_size();
                        break;
                    }
                }
                // Выход из цикла при значимом событии
                if (resp != Response::OK)
                    break;
            }
        }
        return {resp, xc, yc};
    }

    // Метод ожидания действия пользователя (используется после окончания игры)
    Response wait() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        while (true)
        {
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type)
                {
                    // Закрытие окна
                case SDL_QUIT:
                    resp = Response::QUIT;
                    break;
                    // Изменение размера окна
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    board->reset_window_size();
                    break;
                    // Клик мыши
                case SDL_MOUSEBUTTONDOWN: {
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    // Обработка только кнопки "Повторить"
                    if (xc == -1 && yc == 8)
                        resp = Response::REPLAY;
                }
                break;
                }
                // Выход при получении QUIT или REPLAY
                if (resp != Response::OK)
                    break;
            }
        }
        return resp;
    }

  private:
      // Указатель на игровую доску
    Board *board;
};
