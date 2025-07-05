#pragma once
#include "board.h"

namespace Eval {
    // Piece values (in centipawns)
    extern const int PIECE_VALUES[6];
    
    // Positional piece-square tables
    extern const int PAWN_TABLE[64];
    extern const int KNIGHT_TABLE[64];
    extern const int BISHOP_TABLE[64];
    extern const int ROOK_TABLE[64];
    extern const int QUEEN_TABLE[64];
    extern const int KING_TABLE[64];
    extern const int KING_ENDGAME_TABLE[64];
    
    // Evaluation function
    int evaluate(const Board& board);
    
    // Material evaluation
    int evaluate_material(const Board& board);
    
    // Positional evaluation
    int evaluate_position(const Board& board);
    
    // Endgame evaluation
    int evaluate_endgame(const Board& board);
    
    // Utility functions
    bool is_endgame(const Board& board);
    int get_piece_value(Piece piece);
    int get_piece_square_value(Piece piece, Square sq, Color color);
} 