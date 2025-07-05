#pragma once
#include "board.h"
#include "eval.h"
#include <vector>
#include <chrono>

namespace Search {
    // Search result structure
    struct SearchResult {
        Move best_move;
        int score;
        int depth;
        int nodes_searched;
        std::chrono::milliseconds time_taken;
        
        SearchResult() : best_move(), score(0), depth(0), nodes_searched(0), time_taken(0) {}
    };
    
    // Search parameters
    struct SearchParams {
        int max_depth;
        int max_time_ms;
        bool use_quiescence;
        
        SearchParams() : max_depth(4), max_time_ms(5000), use_quiescence(true) {}
    };
    
    // Main search function
    SearchResult search(const Board& board, const SearchParams& params = SearchParams());
    
    // Minimax with alpha-beta pruning
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizing, int& nodes);
    
    // Quiescence search (capture-only search)
    int quiescence(Board& board, int alpha, int beta, bool maximizing, int& nodes);
    
    // Move ordering for better alpha-beta pruning
    void order_moves(std::vector<Move>& moves, const Board& board);
    
    // Utility functions
    bool is_time_up(const std::chrono::steady_clock::time_point& start_time, int max_time_ms);
    Move get_best_move(const Board& board, int depth);
} 