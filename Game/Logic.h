// Первая перегрузка функции find_turns.
// Эта версия принимает цвет игрока и находит все доступные ходы на текущей доске для данного цвета.
public:
    void find_turns(const bool color)
    {
        find_turns(color, board->get_board()); // Вызов приватной версии с текущей доской
    }

    // Вторая перегрузка функции find_turns.
    // Эта версия принимает координаты клетки и находит все доступные ходы для фигуры, находящейся в этой клетке.
public:
    void find_turns(const POS_T x, const POS_T y)
    {
        find_turns(x, y, board->get_board()); // Вызов приватной версии с текущей доской
    }

    // Приватная версия функции find_turns, которая находит доступные ходы для всех фигур заданного цвета на доске.
private:
    void find_turns(const bool color, const vector<vector<POS_T>>& mtx)
    {
        vector<move_pos> res_turns; // Вектор для хранения найденных возможностей ходов
        bool have_beats_before = false; // Флаг, указывающий, есть ли потенциальные побеждения

        // Проход по всем клеткам доски
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                // Если текущая клетка содержит фигуру, которая не принадлежит данному цвету
                if (mtx[i][j] && mtx[i][j] % 2 != color)
                {
                    // Ищем возможные ходы для фигуры в клетке (i, j)
                    find_turns(i, j, mtx);
                    // Если были найденные побеждения, сбрасываем порядок ходов и отмечаем, что существуют побеждения
                    if (have_beats && !have_beats_before)
                    {
                        have_beats_before = true;
                        res_turns.clear(); // Очищаем вектор ходов
                    }

                    // Если есть побеждения, добавляем их в результат или просто добавляем текущие ходы
                    if ((have_beats_before && have_beats) || !have_beats_before)
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }
        turns = res_turns; // Обновляем доступные ходы
        shuffle(turns.begin(), turns.end(), rand_eng); // Перемешиваем ходы для случайности
        have_beats = have_beats_before; // Обновляем информацию о возможности побеждений
    }

    // Приватная версия функции find_turns, которая ищет доступные ходы для фигуры по заданным координатам.
private:
    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>>& mtx)
    {
        turns.clear(); // Очищаем прошлый список ходов
        have_beats = false; // Сбрасываем флаг побеждений
        POS_T type = mtx[x][y]; // Получаем тип фигуры из указанной клетки

        // Проверка на возможность побеждения
        switch (type)
        {
        case 1:
        case 2:
            // Проверка для обычных фигур
            for (POS_T i = x - 2; i <= x + 2; i += 4)
            {
                for (POS_T j = y - 2; j <= y + 2; j += 4)
                {
                    // Проверяем границы доски
                    if (i < 0 || i > 7 || j < 0 || j > 7)
                        continue;

                    // Вычисляем клетку, которую можно побеждать
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2;
                    // Проверяем правильность хода
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)
                        continue;

                    // Добавляем возможный ход
                    turns.emplace_back(x, y, i, j, xb, yb);
                }
            }
            break;
        default:
            // Проверка для ферзей
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1; // Для хранения клетки, которую можно побеждать
                    // Проходим по диагоналям
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        // Проверяем, встречаем ли мы другие фигуры
                        if (mtx[i2][j2])
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break; // Если встретили нашу фигуру, прерываем цикл
                            }
                            xb = i2; // Запоминаем место побеждаемой фигуры
                            yb = j2;
                        }
                        // Если мы можем побеждать
                        if (xb != -1 && xb != i2)
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb);
                        }
                    }
                }
            }
            break;
        }
        // Проверка на другие ходы (не побеждающие)
        if (!turns.empty())
        {
            have_beats = true; // Обновляем флаг, если есть надежные ходы
            return; // Завершаем, если есть побеждения
        }
        switch (type)
        {
        case 1:
        case 2:
            // Проверка для обычных фигур
        {
            POS_T i = ((type % 2) ? x - 1 : x + 1);
            for (POS_T j = y - 1; j <= y + 1; j += 2)
            {
                // Проверяем, можем ли двигаться
                if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                    continue;
                turns.emplace_back(x, y, i, j); // Добавляем ход
            }
            break;
        }
        default:
            // Проверка для ферзей
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        // Проверяем свободные клетки учитывая границы
                        if (mtx[i2][j2])
                            break; // Прерываем, если встречаем фигуру
                        turns.emplace_back(x, y, i2, j2); // Добавляем возможный ход
                    }
                }
            }
            break;
        }
    }
