#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic
{
public:
    Logic(Board* board, Config* config)
    {
        rand_eng = std::default_random_engine(
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0);
        scoring_mode = (*config)("Bot", "BotScoringType");
        optimization = (*config)("Bot", "Optimization");
    }

    vector<move_pos> find_best_turns(const bool color);

private:
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const;

    double calc_score(const vector<vector<POS_T>>& mtx, const bool first_bot_color) const;

    // Удалены реализации данных методов:
    // double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
    //                             double alpha = -1);

    // double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,
    //                            double beta = INF + 1, const POS_T x = -1, const POS_T y = -1);

public:
    void find_turns(const bool color);

    void find_turns(const POS_T x, const POS_T y);

private:
    void find_turns(const bool color, const vector<vector<POS_T>>& mtx);

    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>>& mtx);

public:
    vector<move_pos> turns;
    bool have_beats;
    int Max_depth;

private:
    default_random_engine rand_eng;
    string scoring_mode;
    string optimization;
    vector<move_pos> next_move;
    vector<int> next_best_state;
    Board* board;
    Config* config;
};
