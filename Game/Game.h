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
    Game() : board(config("WindowSize", "Width"), config("WindowSize", "Hight")), hand(&board), logic(&board, &config)
    {
        ofstream fout(project_path + "log.txt", ios_base::trunc); // Открытие файла журнала для записи с очищением его содержимого
        fout.close(); // Закрытие файла после открытия
    }

    // Начало игры в шашки
    int play()
    {
        auto start = chrono::steady_clock::now(); // Начало отсчета времени игры
        if (is_replay) // Проверка, является ли игра повтором
        {
            logic = Logic(&board, &config); // Инициализация логики игры
            config.reload(); // Перезагрузка конфигурации
            board.redraw(); // Перерисовка игрового поля
        }
        else
        {
            board.start_draw(); // Начало рисования игрового поля
        }
        is_replay = false; // Сброс флага повтора

        int turn_num = -1; // Номер текущего хода
        bool is_quit = false; // Флаг выхода из игры
        const int Max_turns = config("Game", "MaxNumTurns"); // Максимальное количество ходов из конфигурации
        while (++turn_num < Max_turns) // Цикл по ходам, пока не достигнуто максимальное количество ходов
        {
            beat_series = 0; // Сброс серии побежденных фигур
            logic.find_turns(turn_num % 2); // Поиск возможных ходов для текущего игрока
            if (logic.turns.empty()) // Проверка, есть ли доступные ходы
                break; // Выход из цикла, если нет доступных ходов

            // Установка уровня сложности бота на основе номера хода
            logic.Max_depth = config("Bot", string((turn_num % 2) ? "Black" : "White") + string("BotLevel"));

            if (!config("Bot", string("Is") + string((turn_num % 2) ? "Black" : "White") + string("Bot"))) // Проверка, играет ли бот в текущем ходу
            {
                auto resp = player_turn(turn_num % 2); // Ход игрока
                if (resp == Response::QUIT) // Проверка, нажата ли кнопка выхода
                {
                    is_quit = true; // Установка флага выхода
                    break; // Выход из цикла
                }
                else if (resp == Response::REPLAY) // Проверка, нажата ли кнопка повтора
                {
                    is_replay = true; // Установка флага повтора
                    break; // Выход из цикла
                }
                else if (resp == Response::BACK) // Проверка, была ли нажата кнопка "Назад"
                {
                    if (config("Bot", string("Is") + string((1 - turn_num % 2) ? "Black" : "White") + string("Bot")) &&
                        !beat_series && board.history_mtx.size() > 2) // Проверка условий для отката
                    {
                        board.rollback(); // Откат хода
                        --turn_num; // Декремент номера хода
                    }
                    if (!beat_series) // Если нет серии побежденных фигур
                        --turn_num; // Декремент номера хода

                    board.rollback(); // Откат хода
                    --turn_num; // Декремент номера хода
                    beat_series = 0; // Сброс серии побежденных фигур
                }
            }
            else
                bot_turn(turn_num % 2); // Ход бота
        }

        // Замер времени окончания игры
        auto end = chrono::steady_clock::now();
        ofstream fout(project_path + "log.txt", ios_base::app); // Открытие файла журнала для записи с добавлением к существующему содержимому
        fout << "Game time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n"; // Запись времени игры в файл
        fout.close(); // Закрытие файла

        if (is_replay) // Если установлен флаг повтора
            return play(); // Повторное начало игры
        if (is_quit) // Если установлен флаг выхода
            return 0; // Возврат 0 для выхода

        int res = 2; // Результат игры, 2 - ничья по умолчанию
        if (turn_num == Max_turns) // Проверка, завершилась ли игра по времени
        {
            res = 0; // Результат 0 - ничья
        }
        else if (turn_num % 2) // Проверка, кто выиграл
        {
            res = 1; // Результат 1 - победа второго игрока
        }
        board.show_final(res); // Показать окончательный результат игры
        auto resp = hand.wait(); // Ожидание ввода игрока
        if (resp == Response::REPLAY) // Проверка, была ли нажата кнопка повтора
        {
            is_replay = true; // Установка флага повтора
            return play(); // Повторное начало игры
        }
        return res; // Возврат результата игры
    }

private:
    // Остальные методы и переменные класса ...
};
