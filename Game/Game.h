private:
    Response player_turn(const bool color)
    {
        // Функция для обработки хода игрока. 'color' указывает цвет (игрока), который сейчас ходит.

        vector<pair<POS_T, POS_T>> cells; // Вектор для хранения доступных ячеек для игрока
        for (auto turn : logic.turns) // Перебор возможных ходов
        {
            cells.emplace_back(turn.x, turn.y); // Добавление доступных ходов в вектор
        }

        board.highlight_cells(cells); // Подсветка доступных для хода клеток на доске
        move_pos pos = { -1, -1, -1, -1 }; // Инициализация переменной для хранения хода
        POS_T x = -1, y = -1; // Координаты, выбранные игроком

        // Пытаемся сделать первый ход
        while (true)
        {
            auto resp = hand.get_cell(); // Ожидаем ввода от игрока для выбора клетки
            if (get<0>(resp) != Response::CELL) // Если ввод не является выбором клетки
                return get<0>(resp); // Вернуть ответ (QUIT, BACK и т.д.)

            pair<POS_T, POS_T> cell{ get<1>(resp), get<2>(resp) }; // Получаем координаты выбранной клетки

            bool is_correct = false; // Флаг, указывающий на правильность выбора
            for (auto turn : logic.turns) // Проверка, является ли выбранный ход доступным
            {
                if (turn.x == cell.first && turn.y == cell.second)
                {
                    is_correct = true; // Ход является корректным
                    break; // Выходим из цикла
                }
                if (turn == move_pos{ x, y, cell.first, cell.second }) // Проверка, если это полный ход (движение уже инициировано)
                {
                    pos = turn; // Запоминаем полный ход
                    break; // Выход из цикла
                }
            }
            if (pos.x != -1) // Если был полный ход, выходим
                break;

            if (!is_correct) // Если выбранный ход неверный
            {
                if (x != -1) // Проверяем, была ли ранее сделана попытка хода
                {
                    board.clear_active(); // Сбрасываем активную клетку
                    board.clear_highlight(); // Убираем подсветку
                    board.highlight_cells(cells); // Повторная подсветка доступных клеток
                }
                x = -1; // Сбрасываем координаты
                y = -1;
                continue; // Переход к следующему циклу для нового ввода
            }

            // Обновляем координаты выбранной клетки
            x = cell.first;
            y = cell.second;
            board.clear_highlight(); // Убираем подсветку
            board.set_active(x, y); // Устанавливаем активную клетку

            vector<pair<POS_T, POS_T>> cells2; // Вектор для хранения возможных ходов после выбора клетки
            for (auto turn : logic.turns)
            {
                if (turn.x == x && turn.y == y) // Проверка, соответствуют ли координаты
                {
                    cells2.emplace_back(turn.x2, turn.y2); // Добавляем возможные ходы
                }
            }
            board.highlight_cells(cells2); // Подсвечиваем новые доступные для хода клетки
        }

        board.clear_highlight(); // Убираем подсветку перед выполнением хода
        board.clear_active(); // Сбрасываем активную клетку
        board.move_piece(pos, pos.xb != -1); // Перемещение фигуры на доске; передаем информацию о том, был ли сделан ход с побеждением

        if (pos.xb == -1) // Если не было побеждения
            return Response::OK; // Успешный ход без побеждения

        // Продолжаем побеждать, если возможно
        beat_series = 1; // Устанавливаем счетчик серии побежденных фигур
        while (true)
        {
            logic.find_turns(pos.x2, pos.y2); // Поиск возможных побеждающих ходов
            if (!logic.have_beats) // Если больше нет доступных полей для побеждения
                break; // Выход из цикла

            vector<pair<POS_T, POS_T>> cells; // Вектор для подсветки доступных клеток для побеждения
            for (auto turn : logic.turns)
            {
                cells.emplace_back(turn.x2, turn.y2); // Добавление доступных клеток
            }
            board.highlight_cells(cells); // Подсветка клеток
            board.set_active(pos.x2, pos.y2); // Установка активной клетки

            // Пытаемся сделать ход
            while (true)
            {
                auto resp = hand.get_cell(); // Ждем ввода от игрока
                if (get<0>(resp) != Response::CELL) // Если ввод не совпал с CELL
                    return get<0>(resp); // Вернуть соответствующий ответ

                pair<POS_T, POS_T> cell{ get<1>(resp), get<2>(resp) }; // Получаем координаты

                bool is_correct = false; // Флаг правильного выбора
                for (auto turn : logic.turns) // Проверка на корректность выбора
                {
                    if (turn.x2 == cell.first && turn.y2 == cell.second) // Если выбор корректен
                    {
                        is_correct = true; // Отметим, что выбор корректен
                        pos = turn; // Запомним поведение игрока
                        break; // Выход из цикла
                    }
                }
                if (!is_correct) // Если неправильный выбор
                    continue; // Сразу продолжаем ожидание правильного выбора

                board.clear_highlight(); // Сбрасываем подсветку
                board.clear_active(); // Убираем активную подсветку
                beat_series += 1; // Увеличиваем серию побежденных фигур
                board.move_piece(pos, beat_series); // Выполняем движение фигуры
                break; // Выход из внутреннего цикла, если движение прошло успешно
            }
        }

        return Response::OK; // Возврат успешного результата после завершения хода
    }
