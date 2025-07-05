#pragma once
#include <cstdint>
#include <string>

// Bitboard type (64-bit unsigned integer)
using Bitboard = uint64_t;

// Piece types
enum Piece : int {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
    PIECE_NONE = 6
};

// Colors
enum Color : int {
    WHITE = 0,
    BLACK = 1,
    COLOR_NONE = 2
};

// Squares (A1 = 0, H8 = 63)
enum Square : int {
    A1 = 0, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQUARE_NONE = 64
};

// Files and ranks
enum File : int {
    FILE_A = 0, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

enum Rank : int {
    RANK_1 = 0, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};

// Bitboard constants
constexpr Bitboard EMPTY_BB = 0ULL;
constexpr Bitboard FULL_BB = 0xFFFFFFFFFFFFFFFFULL;

// File and rank bitboards
extern Bitboard FILE_BB[8];
extern Bitboard RANK_BB[8];

// Square bitboards
extern Bitboard SQUARE_BB[64];

// Direction bitboards for sliding pieces
extern Bitboard RAYS[8][64];

// Pawn attack tables
extern Bitboard PAWN_ATTACKS[2][64];

// Knight and king attack tables
extern Bitboard KNIGHT_ATTACKS[64];
extern Bitboard KING_ATTACKS[64];

// Bishop and rook magic tables (deprecated - using step-by-step approach)
// extern Bitboard BISHOP_ATTACKS[64][512];
// extern Bitboard ROOK_ATTACKS[64][4096];

// Magic numbers for sliding piece move generation (deprecated)
// extern Bitboard BISHOP_MAGIC[64];
// extern Bitboard ROOK_MAGIC[64];

// Utility functions
namespace BitboardUtils {
    // Initialize all lookup tables
    void init();
    
    // Bit manipulation
    int popcount(Bitboard bb);
    int lsb(Bitboard bb);
    int msb(Bitboard bb);
    Bitboard pop_lsb(Bitboard& bb);
    
    // Square utilities
    Square make_square(File file, Rank rank);
    File file_of(Square sq);
    Rank rank_of(Square sq);
    Square relative_square(Color c, Square sq);
    
    // String conversion
    std::string square_to_string(Square sq);
    Square string_to_square(const std::string& str);
    
    // Direction utilities
    bool is_attacked(Square sq, Color by_color, const Bitboard pieces[2][6]);
    Bitboard get_attacks(Piece piece, Square sq, Color color, Bitboard occupied);
}

// Inline functions for performance
inline Bitboard square_bb(Square sq) {
    return SQUARE_BB[sq];
}

inline Bitboard file_bb(File file) {
    return FILE_BB[file];
}

inline Bitboard rank_bb(Rank rank) {
    return RANK_BB[rank];
}

inline bool test_bit(Bitboard bb, Square sq) {
    return (bb & square_bb(sq)) != 0;
}

inline void set_bit(Bitboard& bb, Square sq) {
    bb |= square_bb(sq);
}

inline void clear_bit(Bitboard& bb, Square sq) {
    bb &= ~square_bb(sq);
}

inline void flip_bit(Bitboard& bb, Square sq) {
    bb ^= square_bb(sq);
} 