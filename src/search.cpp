#include "search.h"
#include "movegen.h"
#include <algorithm>
#include <iostream>

namespace Search {

// Global variables for search
static std::chrono::steady_clock::time_point search_start_time;
static int search_max_time_ms;
static bool search_time_up = false;

SearchResult search(const Board& board, const SearchParams& params) {
    SearchResult result;
    search_start_time = std::chrono::steady_clock::now();
    search_max_time_ms = params.max_time_ms;
    search_time_up = false;
    
    std::vector<Move> moves = board.generate_legal_moves();
    if (moves.empty()) {
        return result; // No legal moves
    }
    
    // Iterative deepening
    for (int depth = 1; depth <= params.max_depth && !search_time_up; depth++) {
        Move best_move_at_depth = moves[0];
        int best_score = -1000000;
        int alpha = -1000000;
        int beta = 1000000;
        
        // Search each move at current depth
        for (const Move& move : moves) {
            if (search_time_up) break;
            
            Board temp_board = board;
            temp_board.make_move(move);
            
            int score = -minimax(temp_board, depth - 1, -beta, -alpha, result.nodes_searched);
            
            if (score > best_score) {
                best_score = score;
                best_move_at_depth = move;
            }
            
            alpha = std::max(alpha, score);
            if (alpha >= beta) break;
        }
        
        // Update result if we completed this depth
        if (!search_time_up) {
            result.best_move = best_move_at_depth;
            result.score = best_score;
            result.depth = depth;
        }
    }
    
    result.time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - search_start_time);
    
    return result;
}

int minimax(Board& board, int depth, int alpha, int beta,  int& nodes) {
    nodes++;
    
    // Check for time up
    if (is_time_up(search_start_time, search_max_time_ms)) {
        search_time_up = true;
        return 0;
    }
    
    // Terminal node evaluation
    if (depth == 0) {
        if (search_time_up) return 0;
        return quiescence(board, alpha, beta, nodes);
    }
    
    // Check for game over
    if (board.is_game_over()) {
        if (board.is_checkmate()) {
            return -100000 + depth;
        }
        if (board.is_stalemate() || board.is_draw()) {
            return 0;
        }
    }
    
    std::vector<Move> moves = board.generate_legal_moves();
    order_moves(moves, board);
    
    int best_score  = -1000000;
    for (const Move& move : moves) {
        if (search_time_up) break;
        
        board.make_move(move);
        int eval = -minimax(board, depth - 1, -beta, -alpha, nodes);
        board.undo_move();
        
        best_score = std::max(best_score, eval);
        alpha = std::max(alpha, eval);
        if (beta <= alpha) break; // Beta cutoff
    }
    return best_score;
}

int quiescence(Board& board, int alpha, int beta, int& nodes) {
    nodes++;
    
    // Check for time up
    if (is_time_up(search_start_time, search_max_time_ms)) {
        search_time_up = true;
        return 0;
    }
    
    // Static evaluation
    int stand_pat = Eval::evaluate(board);
    
    if (stand_pat >= beta) return beta;

    // Delta pruning - if even capturing the most valuable piece 
    // can't raise alpha, skip quiescence search
    const int DELTA_MARGIN = 900; // Roughly queen value
    if (stand_pat < alpha - DELTA_MARGIN) {
        return alpha;
    }

    if (stand_pat > alpha) alpha = stand_pat;
    
    // Generate only capture moves
    std::vector<Move> captures = board.generate_captures();
    order_moves(captures, board);
    
    int best_score = stand_pat;
    for (const Move& move : captures) {
        if (search_time_up) break;
        
        board.make_move(move);
        int eval = -quiescence(board, -beta, -alpha, nodes);
        board.undo_move();
        
        best_score = std::max(best_score, eval);
        alpha = std::max(alpha, eval);
        if (beta <= alpha) break;
    }
    return best_score;
}

void order_moves(std::vector<Move>& moves, const Board& board) {
    std::vector<std::pair<int, Move>> scored_moves;
    
    for (const Move& move : moves) {
        int score = 0;
        
        // Captures get high priority
        if (move.is_capture()) {
            Piece captured_piece = board.get_piece(move.to());
            Piece moving_piece = board.get_piece(move.from());
            score += Eval::get_piece_value(captured_piece) * 10 - Eval::get_piece_value(moving_piece);
        }
        
        // Promotions get high priority
        if (move.is_promotion()) {
            score += Eval::get_piece_value(move.promotion()) * 8;
        }
        
        // Castling gets medium priority
        if (move.is_castle()) {
            score += 50;
        }
        
        // Pawn pushes to center get some priority
        if (board.get_piece(move.from()) == PAWN) {
            int to_rank = BitboardUtils::rank_of(move.to());
            int to_file = BitboardUtils::file_of(move.to());
            if (to_file >= 2 && to_file <= 5 && to_rank >= 3 && to_rank <= 4) {
                score += 10;
            }
        }
        
        scored_moves.emplace_back(score, move);
    }
    
    // Sort by score (highest first)
    std::sort(scored_moves.begin(), scored_moves.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Extract moves in sorted order
    moves.clear();
    for (const auto& scored_move : scored_moves) {
        moves.push_back(scored_move.second);
    }
}

bool is_time_up(const std::chrono::steady_clock::time_point& start_time, int max_time_ms) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
    return elapsed.count() >= max_time_ms;
}

Move get_best_move(const Board& board, int depth) {
    SearchParams params;
    params.max_depth = depth;
    params.max_time_ms = 10000; // 10 seconds
    
    SearchResult result = search(board, params);
    return result.best_move;
}

} // namespace Search 