#pragma once
#include <iostream>
#include <fstream>
#include <vector>

#include "../Models/Move.h"
#include "../Models/Project_path.h"

// Подключение библиотек SDL2 для работы с графикой в зависимости от платформы
#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#endif

using namespace std;

// Класс для представления игровой доски
class Board
{
public:
    Board() = default; // Конструктор по умолчанию
    Board(const unsigned int W, const unsigned int H) : W(W), H(H) // Конструктор с параметрами для задания размера доски
    {
    }

    // Метод для инициализации и рисования стартовой доски
    int start_draw()
    {
        // Инициализация библиотеки SDL
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            print_exception("SDL_Init can't init SDL2 lib"); // Обработка ошибки
            return 1; // Возврат кода ошибки
        }

        // Проверка заданных размеров доски, если они равны нулю, получаем размеры экрана
        if (W == 0 || H == 0)
        {
            SDL_DisplayMode dm;
            if (SDL_GetDesktopDisplayMode(0, &dm))
            {
                print_exception("SDL_GetDesktopDisplayMode can't get desktop display mode"); // Обработка ошибки
                return 1;
            }
            W = min(dm.w, dm.h); // Установка ширины
            W -= W / 15; // Уменьшение ширины
            H = W; // Установка высоты
        }

        // Создание окна и рендерера
        win = SDL_CreateWindow("Checkers", 0, H / 30, W, H, SDL_WINDOW_RESIZABLE);
        if (win == nullptr)
        {
            print_exception("SDL_CreateWindow can't create window"); // Обработка ошибки
            return 1;
        }
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (ren == nullptr)
        {
            print_exception("SDL_CreateRenderer can't create renderer"); // Обработка ошибки
            return 1;
        }

        // Загрузка текстур для игровой доски и фигур
        board = IMG_LoadTexture(ren, board_path.c_str());
        w_piece = IMG_LoadTexture(ren, piece_white_path.c_str());
        b_piece = IMG_LoadTexture(ren, piece_black_path.c_str());
        w_queen = IMG_LoadTexture(ren, queen_white_path.c_str());
        b_queen = IMG_LoadTexture(ren, queen_black_path.c_str());
        back = IMG_LoadTexture(ren, back_path.c_str());
        replay = IMG_LoadTexture(ren, replay_path.c_str());

        // Проверка успешности загрузки текстур
        if (!board || !w_piece || !b_piece || !w_queen || !b_queen || !back || !replay)
        {
            print_exception("IMG_LoadTexture can't load main textures from " + textures_path); // Обработка ошибки
            return 1;
        }

        // Получаем размер рендерера
        SDL_GetRendererOutputSize(ren, &W, &H);
        make_start_mtx(); // Создание стартовой матрицы
        rerender(); // Ререндеринг доски
        return 0; // Успешное завершение метода
    }

    // Метод для перерисовки доски
    void redraw()
    {
        game_results = -1; // Сбрасываем результаты игры
        history_mtx.clear(); // Очищаем историю ходов
        history_beat_series.clear(); // Очищаем серию побежденных фигур
        make_start_mtx(); // Создание стартовой матрицы заново
        clear_active(); // Очистка активной ячейки
        clear_highlight(); // Очистка подсветки
    }

    // Метод передвижения фигуры
    void move_piece(move_pos turn, const int beat_series = 0)
    {
        if (turn.xb != -1) // Если есть побежденная фигура
        {
            mtx[turn.xb][turn.yb] = 0; // Убираем побежденную фигуру с поля
        }
        move_piece(turn.x, turn.y, turn.x2, turn.y2, beat_series); // Перемещаем фигуру
    }

    // Перегруженный метод для перемещения фигуры с координатами
    void move_piece(const POS_T i, const POS_T j, const POS_T i2, const POS_T j2, const int beat_series = 0)
    {
        if (mtx[i2][j2]) // Проверка, пуста ли конечная позиция
        {
            throw runtime_error("final position is not empty, can't move"); // Исключение, если не пусто
        }
        if (!mtx[i][j]) // Проверка, пуста ли начальная позиция
        {
            throw runtime_error("begin position is empty, can't move"); // Исключение, если пусто
        }
        if ((mtx[i][j] == 1 && i2 == 0) || (mtx[i][j] == 2 && i2 == 7)) // Поверка на превращение в ферзя
            mtx[i][j] += 2; // Превращаем в ферзя
        mtx[i2][j2] = mtx[i][j]; // Перемещаем фигуру на новое место
        drop_piece(i, j); // Убираем фигуру с начальной позиции
        add_history(beat_series); // Добавляем ход в историю
    }

    // Метод для удаления фигуры с доски
    void drop_piece(const POS_T i, const POS_T j)
    {
        mtx[i][j] = 0; // Устанавливаем пустую ячейку
        rerender(); // Перерисовываем доску
    }

    // Метод для превращения фигуры в ферзя
    void turn_into_queen(const POS_T i, const POS_T j)
    {
        if (mtx[i][j] == 0 || mtx[i][j] > 2) // Проверяем, можно ли превратить
        {
            throw runtime_error("can't turn into queen in this position"); // Исключение, если нельзя
        }
        mtx[i][j] += 2; // Превращаем в ферзя
        rerender(); // Перерисовываем доску
    }

    // Метод для получения текущей матрицы доски
    vector<vector<POS_T>> get_board() const
    {
        return mtx; // Возврат матрицы
    }

    // Метод для подсветки выбранных клеток
    void highlight_cells(vector<pair<POS_T, POS_T>> cells)
    {
        for (auto pos : cells)
        {
            POS_T x = pos.first, y = pos.second;
            is_highlighted_[x][y] = 1; // Установка ячейки как выделенной
        }
        rerender(); // Перерисовываем доску
    }

    // Метод для очистки подсветки клеток
    void clear_highlight()
    {
        for (POS_T i = 0; i < 8; ++i)
        {
            is_highlighted_[i].assign(8, 0); // Сброс подсветки всех клеток
        }
        rerender(); // Перерисовываем доску
    }

    // Метод для установки активной ячейки
    void set_active(const POS_T x, const POS_T y)
    {
        active_x = x; // Сохранение координат активной ячейки
        active_y = y;
        rerender(); // Перерисовываем доску
    }

    // Метод для очистки активной ячейки
    void clear_active()
    {
        active_x = -1; // Сброс активной ячейки
        active_y = -1;
        rerender(); // Перерисовываем доску
    }

    // Метод для проверки, подсвечена ли клетка
    bool is_highlighted(const POS_T x, const POS_T y)
    {
        return is_highlighted_[x][y]; // Возврат статуса подсветки
    }

    // Метод для отката хода
    void rollback()
    {
        auto beat_series = max(1, *(history_beat_series.rbegin())); // Получаем количество побеждений для отката
        while (beat_series-- && history_mtx.size() > 1)
        {
            history_mtx.pop_back(); // Удаляем последний ход из истории
            history_beat_series.pop_back(); // Удаляем количество побежденных фигур
        }
        mtx = *(history_mtx.rbegin()); // Восстанавливаем доску из последней истории
        clear_highlight(); // Очищаем выделение
        clear_active(); // Очищаем активную ячейку
    }

    // Метод для отображения финального результата игры
    void show_final(const int res)
    {
        game_results = res; // Сохранение результата игры
        rerender(); // Перерисовываем доску
    }

    // Метод для обновления размеров окна
    void reset_window_size()
    {
        SDL_GetRendererOutputSize(ren, &W, &H); // Получаем новые размеры рендерера
        rerender(); // Перерисовываем доску
    }

    // Метод для завершения работы и освобождения ресурсов
    void quit()
    {
        SDL_DestroyTexture(board);
        SDL_DestroyTexture(w_piece);
        SDL_DestroyTexture(b_piece);
        SDL_DestroyTexture(w_queen);
        SDL_DestroyTexture(b_queen);
        SDL_DestroyTexture(back);
        SDL_DestroyTexture(replay);
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit(); // Завершение работы библиотеки SDL
    }

    // Деструктор для очистки ресурсов при уничтожении объекта
    ~Board()
    {
        if (win) // Если окно было создано
            quit(); // Освобождение ресурсов
    }

private:
    // Метод для добавления текущего состояния доски в историю
    void add_history(const int beat_series = 0)
    {
        history_mtx.push_back(mtx); // Добавление состояния доски в историю
        history_beat_series.push_back(beat_series); // Запись серии побеждений
    }

    // Метод для создания стартовой матрицы
    void make_start_mtx()
    {
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                mtx[i][j] = 0; // Изначально все клетки пустые
                if (i < 3 && (i + j) % 2 == 1)
                    mtx[i][j] = 2; // Настройка начальных позиций черных фигур
                if (i > 4 && (i + j) % 2 == 1)
                    mtx[i][j] = 1; // Настройка начальных позиций белых фигур
            }
        }
        add_history(); // Добавление начального состояния доски в историю
    }

    // Метод для перерисовки всех текстур на доске
    void rerender()
    {
        SDL_RenderClear(ren); // Очистка рендерера
        SDL_RenderCopy(ren, board, NULL, NULL); // Рисуем доску

        // Рисуем фигуры на доске
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (!mtx[i][j]) // Если поле пустое, пропускаем
                    continue;
                int wpos = W * (j + 1) / 10 + W / 120; // Вычисление позиции по X
                int hpos = H * (i + 1) / 10 + H / 120; // Вычисление позиции по Y
                SDL_Rect rect{ wpos, hpos, W / 12, H / 12 }; // Размер и положение фигуры

                SDL_Texture* piece_texture; // Переменная для хранения текстуры фигуры
                if (mtx[i][j] == 1) // Проверка цвета фигуры
                    piece_texture = w_piece; // Белая фигура
                else if (mtx[i][j] == 2)
                    piece_texture = b_piece; // Черная фигура
                else if (mtx[i][j] == 3)
                    piece_texture = w_queen; // Белый ферзь
                else
                    piece_texture = b_queen; // Черный ферзь

                SDL_RenderCopy(ren, piece_texture, NULL, &rect); // Рисуем фигуру
            }
        }

        // Рисуем подсветку
        SDL_SetRenderDrawColor(ren, 0, 255, 0, 0); // Задаем цвет подсветки
        const double scale = 2.5; // Масштаб подсветки
        SDL_RenderSetScale(ren, scale, scale); // Устанавливаем масштаб
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (!is_highlighted_[i][j]) // Проверка на выделение
                    continue;
                SDL_Rect cell{ int(W * (j + 1) / 10 / scale), int(H * (i + 1) / 10 / scale), int(W / 10 / scale),
                              int(H / 10 / scale) }; // Рисуем рамку вокруг выделенной клетки
                SDL_RenderDrawRect(ren, &cell);
            }
        }

        // Рисуем активную клетку
        if (active_x != -1)
        {
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 0); // Красный цвет для активной клетки
            SDL_Rect active_cell{ int(W * (active_y + 1) / 10 / scale), int(H * (active_x + 1) / 10 / scale),
                                 int(W / 10 / scale), int(H / 10 / scale) }; // Параметры активной клетки
            SDL_RenderDrawRect(ren, &active_cell); // Рисуем активную клетку
        }
        SDL_RenderSetScale(ren, 1, 1); // Сбрасываем масштаб

        // Рисуем стрелки на интерфейсе
        SDL_Rect rect_left{ W / 40, H / 40, W / 15, H / 15 }; // Параметры стрелки назад
        SDL_RenderCopy(ren, back, NULL, &rect_left); // Рисуем стрелку назад
        SDL_Rect replay_rect{ W * 109 / 120, H / 40, W / 15, H / 15 }; // Параметры стрелки повтора
        SDL_RenderCopy(ren, replay, NULL, &replay_rect); // Рисуем стрелку повтора

        // Рисуем результат игры, если он есть
        if (game_results != -1) // Если результат известен
        {
            string result_path = draw_path; // Путь к текстуре для результата
            if (game_results == 1)
                result_path = white_path; // Путь для победы белых
            else if (game_results == 2)
                result_path = black_path; // Путь для победы черных
            SDL_Texture* result_texture = IMG_LoadTexture(ren, result_path.c_str()); // Подгружаем текстуру результата
            if (result_texture == nullptr)
            {
                print_exception("IMG_LoadTexture can't load game result picture from " + result_path); // Обработка ошибки
                return;
            }
            SDL_Rect res_rect{ W / 5, H * 3 / 10, W * 3 / 5, H * 2 / 5 }; // Параметры текстуры результата
            SDL_RenderCopy(ren, result_texture, NULL, &res_rect); // Рисуем текстуру результата
            SDL_DestroyTexture(result_texture); // Удаляем текстуру результата после рисования
        }

        SDL_RenderPresent(ren); // Обновляем рендерер
        // Дополнительные строки для macOS
        SDL_Delay(10); // Задержка для управления частотой обновления экрана
        SDL_Event windowEvent; // Структура события окна
        SDL_PollEvent(&windowEvent); // Проверка событий окна
    }

    // Метод для записи исключений в лог дерева
    void print_exception(const string& text) {
        ofstream fout(project_path + "log.txt", ios_base::app); // Открытие файла для записи
        fout << "Error: " << text << ". " << SDL_GetError() << endl; // Запись текста ошибки
        fout.close(); // Закрытие файла
    }

public:
    int W = 0; // Ширина доски
    int H = 0; // Высота доски
    // История состояний доски
    vector<vector<vector<POS_T>>> history_mtx;

private:
    SDL_Window* win = nullptr; // Указатель на окно
    SDL_Renderer* ren = nullptr; // Указатель на рендерер
    // Текстуры фигур и доски
    SDL_Texture* board = nullptr;
    SDL_Texture* w_piece = nullptr;
    SDL_Texture* b_piece = nullptr;
    SDL_Texture* w_queen = nullptr;
    SDL_Texture* b_queen = nullptr;
    SDL_Texture* back = nullptr;
    SDL_Texture* replay = nullptr;
    // Путь к текстурным файлам
    const string textures_path = project_path + "Textures/";
    const string board_path = textures_path + "board.png";
    const string piece_white_path = textures_path + "piece_white.png";
    const string piece_black_path = textures_path + "piece_black.png";
    const string queen_white_path = textures_path + "queen_white.png";
    const string queen_black_path = textures_path + "queen_black.png";
    const string white_path = textures_path + "white_wins.png";
    const string
