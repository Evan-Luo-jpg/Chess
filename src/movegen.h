#pragma once
#include "board.h"
#include <vector>

namespace MoveGen {
    // Generate all pseudo-legal moves
    std::vector<Move> generate_moves(const Board& board);
    
    // Generate only capture moves
    std::vector<Move> generate_captures(const Board& board);
    
    // Generate moves for specific piece types
    std::vector<Move> generate_pawn_moves(const Board& board, Color color);
    std::vector<Move> generate_knight_moves(const Board& board, Color color);
    std::vector<Move> generate_bishop_moves(const Board& board, Color color);
    std::vector<Move> generate_rook_moves(const Board& board, Color color);
    std::vector<Move> generate_queen_moves(const Board& board, Color color);
    std::vector<Move> generate_king_moves(const Board& board, Color color);
    
    // Generate castling moves
    std::vector<Move> generate_castling_moves(const Board& board, Color color);
    
    // Utility functions
    void add_pawn_moves(std::vector<Move>& moves, Square from, Bitboard targets, Color color);
    void add_promotion_moves(std::vector<Move>& moves, Square from, Square to, bool is_capture);
    void add_castling_move(std::vector<Move>& moves, Square king_from, Square king_to, Square rook_from, Square rook_to);
} 