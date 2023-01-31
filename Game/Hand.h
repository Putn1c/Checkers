#pragma once
#include "../Models/Models.h"
#include "Board.h"

#include <SDL2/SDL.h>

class Hand {
public:
    Hand(Board* board): board(board) {}

    pair<int, int> get_cell() {
        SDL_Event windowEvent;
        bool quit = false;
        int x = -1, y = -1;
        while(true) {
            if ( SDL_PollEvent( &windowEvent ) )
            {
                switch(windowEvent.type) {
                  case SDL_QUIT:
                        quit = true;
                        break;
                  case SDL_MOUSEBUTTONDOWN:
                        x = windowEvent.motion.x;
                        y = windowEvent.motion.y;
                        break;
                  case SDL_WINDOWEVENT:
                        if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                            board->reset_window_size();
                            break;
                        }
                        
                }
                if (x != -1 || quit) break;
            }
        }
        if (quit) return {-1, -1};
        return {max(y / (board->H / 10) - 1, 0), max(x / (board->W / 10) - 1, 0)};
    }
    
    void wait() {
        SDL_Event windowEvent;
        bool quit = false;
        while(true) {
            if ( SDL_PollEvent( &windowEvent ) )
            {
                switch(windowEvent.type) {
                  case SDL_QUIT:
                        quit = true;
                        break;
                  case SDL_WINDOWEVENT_SIZE_CHANGED:
                        board->reset_window_size();
                        break;
                }
                if (quit) break;
            }
        }
    }

private:
    Board* board;
};
