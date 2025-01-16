#pragma once
#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// Класс для обработки действий игрока
class Hand
{
public:
    Hand(Board* board) : board(board) // Конструктор класса Hand, инициализирующий указатель на объект доски
    {
    }

    // Метод для получения ввода игрока (выбор клетки)
    tuple<Response, POS_T, POS_T> get_cell() const
    {
        SDL_Event windowEvent; // Структура для обработки событий SDL
        Response resp = Response::OK; // Изначальный ответ - успешный
        int x = -1, y = -1; // Координаты мыши
        int xc = -1, yc = -1; // Ячеечные координаты, вычисленные из экранных координат

        while (true) // Бесконечный цикл для обработки ввода
        {
            if (SDL_PollEvent(&windowEvent)) // Проверяем наличие событий
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT: // Проверка, было ли событие выхода из приложения
                    resp = Response::QUIT; // Установка ответа на выход
                    break;

                case SDL_MOUSEBUTTONDOWN: // Обработка нажатий мыши
                    x = windowEvent.motion.x; // Получаем координаты курсора по оси X
                    y = windowEvent.motion.y; // Получаем координаты курсора по оси Y
                    xc = int(y / (board->H / 10) - 1); // Вычисляем ячейку по вертикали
                    yc = int(x / (board->W / 10) - 1); // Вычисляем ячейку по горизонтали

                    // Проверка условий для различных действий
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1)
                    {
                        resp = Response::BACK; // Если есть история ходов, игрок хочет вернуться обратно
                    }
                    else if (xc == -1 && yc == 8)
                    {
                        resp = Response::REPLAY; // Если нажато в области повтора игры
                    }
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8) // Проверка, находится ли клик внутри игрового поля
                    {
                        resp = Response::CELL; // Игрок выбрал клетку для хода
                    }
                    else // Если клик не в рамках игры
                    {
                        xc = -1; // Сброс координат
                        yc = -1;
                    }
                    break;

                case SDL_WINDOWEVENT: // Обработка событий окна
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) // Проверка изменения размера окна
                    {
                        board->reset_window_size(); // Сброс размеров доски
                        break;
                    }
                }
                if (resp != Response::OK) // Если произошел выход или другое действие
                    break; // Выход из цикла
            }
        }
        return { resp, xc, yc }; // Возврат кортежа с результатом, координатами ячейки
    }

    // Метод ожидания ввода от игрока
    Response wait() const
    {
        SDL_Event windowEvent; // Структура для обработки событий SDL
        Response resp = Response::OK; // Изначальный ответ - успешный
        while (true) // Бесконечный цикл для обработки ввода
        {
            if (SDL_PollEvent(&windowEvent)) // Проверяем наличие событий
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT: // Проверка, было ли событие выхода из приложения
                    resp = Response::QUIT; // Установка ответа на выход
                    break;

                case SDL_WINDOWEVENT_SIZE_CHANGED: // Проверка изменения размера окна
                    board->reset_window_size(); // Сброс размеров доски
                    break;

                case SDL_MOUSEBUTTONDOWN: { // Обработка нажатий мыши
                    int x = windowEvent.motion.x; // Получаем координаты курсора по оси X
                    int y = windowEvent.motion.y; // Получаем координаты курсора по оси Y
                    int xc = int(y / (board->H / 10) - 1); // Вычисляем ячейку по вертикали
                    int yc = int(x / (board->W / 10) - 1); // Вычисляем ячейку по горизонтали

                    if (xc == -1 && yc == 8) // Проверка нажатия на кнопку повтора
                        resp = Response::REPLAY; // Установка ответа на повтор
                }
                                        break;
                }
                if (resp != Response::OK) // Если произошел выход или другое действие
                    break; // Выход из цикла
            }
        }
        return resp; // Возврат результата ввода
    }

private:
    Board* board; // Указатель на объект доски
};

