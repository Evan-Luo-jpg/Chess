#include "bitboard.h"
#include <cstring>
#include <stdexcept>

// Global lookup tables
Bitboard FILE_BB[8];
Bitboard RANK_BB[8];
Bitboard SQUARE_BB[64];
Bitboard RAYS[8][64];
Bitboard PAWN_ATTACKS[2][64];
Bitboard KNIGHT_ATTACKS[64];
Bitboard KING_ATTACKS[64];
// Magic bitboard arrays (deprecated - using step-by-step approach)
// Bitboard BISHOP_ATTACKS[64][512];
// Bitboard ROOK_ATTACKS[64][4096];
// Bitboard BISHOP_MAGIC[64];
// Bitboard ROOK_MAGIC[64];

// Magic numbers for sliding piece move generation (deprecated - using step-by-step approach)
// static const Bitboard BishopMagicNumbers[64] = { ... };
// static const Bitboard RookMagicNumbers[64] = { ... };

namespace BitboardUtils {

void init() {
    // Initialize square bitboards
    for (int sq = 0; sq < 64; sq++) {
        SQUARE_BB[sq] = 1ULL << sq;
    }
    
    // Initialize file and rank bitboards
    for (int f = 0; f < 8; f++) {
        FILE_BB[f] = 0x0101010101010101ULL << f;
    }
    for (int r = 0; r < 8; r++) {
        RANK_BB[r] = 0xFFULL << (r * 8);
    }
    
    // Initialize rays for sliding pieces
    for (int sq = 0; sq < 64; sq++) {
        int file = file_of(Square(sq));
        int rank = rank_of(Square(sq));
        
        // Initialize all rays to empty
        for (int dir = 0; dir < 8; dir++) {
            RAYS[dir][sq] = 0;
        }
        
        // North
        for (int r = rank + 1; r < 8; r++) {
            set_bit(RAYS[0][sq], make_square(File(file), Rank(r)));
        }
        
        // South
        for (int r = rank - 1; r >= 0; r--) {
            set_bit(RAYS[1][sq], make_square(File(file), Rank(r)));
        }
        
        // East
        for (int f = file + 1; f < 8; f++) {
            set_bit(RAYS[2][sq], make_square(File(f), Rank(rank)));
        }
        
        // West
        for (int f = file - 1; f >= 0; f--) {
            set_bit(RAYS[3][sq], make_square(File(f), Rank(rank)));
        }
        
        // Northeast, Northwest, Southeast, Southwest
        for (int i = 1; i < 8; i++) {
            if (file + i < 8 && rank + i < 8) {
                set_bit(RAYS[4][sq], make_square(File(file + i), Rank(rank + i)));
            }
            if (file - i >= 0 && rank + i < 8) {
                set_bit(RAYS[5][sq], make_square(File(file - i), Rank(rank + i)));
            }
            if (file + i < 8 && rank - i >= 0) {
                set_bit(RAYS[6][sq], make_square(File(file + i), Rank(rank - i)));
            }
            if (file - i >= 0 && rank - i >= 0) {
                set_bit(RAYS[7][sq], make_square(File(file - i), Rank(rank - i)));
            }
        }
    }
    
    // Initialize pawn attack tables
    for (int sq = 0; sq < 64; sq++) {
        int file = file_of(Square(sq));
        int rank = rank_of(Square(sq));
        
        // White pawn attacks (captures to the left and right)
        if (rank < 7) {
            if (file > 0) set_bit(PAWN_ATTACKS[WHITE][sq], make_square(File(file - 1), Rank(rank + 1)));
            if (file < 7) set_bit(PAWN_ATTACKS[WHITE][sq], make_square(File(file + 1), Rank(rank + 1)));
        }
        
        // Black pawn attacks (captures to the left and right)
        if (rank > 0) {
            if (file > 0) set_bit(PAWN_ATTACKS[BLACK][sq], make_square(File(file - 1), Rank(rank - 1)));
            if (file < 7) set_bit(PAWN_ATTACKS[BLACK][sq], make_square(File(file + 1), Rank(rank - 1)));
        }
    }
    
    // Initialize knight attack table
    for (int sq = 0; sq < 64; sq++) {
        int file = file_of(Square(sq));
        int rank = rank_of(Square(sq));
        
        const int knight_dx[] = {-2, -2, -1, -1, 1, 1, 2, 2};
        const int knight_dy[] = {-1, 1, -2, 2, -2, 2, -1, 1};
        
        for (int i = 0; i < 8; i++) {
            int new_file = file + knight_dx[i];
            int new_rank = rank + knight_dy[i];
            if (new_file >= 0 && new_file < 8 && new_rank >= 0 && new_rank < 8) {
                set_bit(KNIGHT_ATTACKS[sq], make_square(File(new_file), Rank(new_rank)));
            }
        }
    }
    
    // Initialize king attack table
    for (int sq = 0; sq < 64; sq++) {
        int file = file_of(Square(sq));
        int rank = rank_of(Square(sq));
        
        for (int df = -1; df <= 1; df++) {
            for (int dr = -1; dr <= 1; dr++) {
                if (df == 0 && dr == 0) continue;
                int new_file = file + df;
                int new_rank = rank + dr;
                if (new_file >= 0 && new_file < 8 && new_rank >= 0 && new_rank < 8) {
                    set_bit(KING_ATTACKS[sq], make_square(File(new_file), Rank(new_rank)));
                }
            }
        }
    }
    
    // Magic number initialization (deprecated - using step-by-step approach)
    // for (int i = 0; i < 64; i++) {
    //     BISHOP_MAGIC[i] = BishopMagicNumbers[i];
    //     ROOK_MAGIC[i] = RookMagicNumbers[i];
    // }
    
    // Sliding piece attack table initialization (deprecated - using step-by-step approach)
    // for (int sq = 0; sq < 64; sq++) {
    //     // Bishop attacks initialization code...
    //     // Rook attacks initialization code...
    // }
}

int popcount(Bitboard bb) {
    return __builtin_popcountll(bb);
}

int lsb(Bitboard bb) {
    return __builtin_ctzll(bb);
}

int msb(Bitboard bb) {
    return 63 - __builtin_clzll(bb);
}

Bitboard pop_lsb(Bitboard& bb) {
    Bitboard result = bb & -bb;
    bb &= bb - 1;
    return result;
}

Square make_square(File file, Rank rank) {
    return Square(rank * 8 + file);
}

File file_of(Square sq) {
    return File(sq & 7);
}

Rank rank_of(Square sq) {
    return Rank(sq >> 3);
}

Square relative_square(Color c, Square sq) {
    return c == WHITE ? sq : Square(sq ^ 56);
}

std::string square_to_string(Square sq) {
    if (sq == SQUARE_NONE) return "-";
    char file = 'a' + file_of(sq);
    char rank = '1' + rank_of(sq);
    return std::string(1, file) + std::string(1, rank);
}

Square string_to_square(const std::string& str) {
    if (str.length() != 2) return SQUARE_NONE;
    int file = str[0] - 'a';
    int rank = str[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return SQUARE_NONE;
    return make_square(File(file), Rank(rank));
}

bool is_attacked(Square sq, Color by_color, const Bitboard pieces[2][6]) {
    Bitboard occupied = pieces[WHITE][0] | pieces[WHITE][1] | pieces[WHITE][2] | 
                       pieces[WHITE][3] | pieces[WHITE][4] | pieces[WHITE][5] |
                       pieces[BLACK][0] | pieces[BLACK][1] | pieces[BLACK][2] | 
                       pieces[BLACK][3] | pieces[BLACK][4] | pieces[BLACK][5];
    
    // Check pawn attacks
    if (PAWN_ATTACKS[!by_color][sq] & pieces[by_color][PAWN]) return true;
    
    // Check knight attacks
    if (KNIGHT_ATTACKS[sq] & pieces[by_color][KNIGHT]) return true;
    
    // Check bishop attacks using step-by-step approach
    Bitboard bishops = pieces[by_color][BISHOP];
    while (bishops) {
        Bitboard bishop = pop_lsb(bishops);
        Square from = Square(lsb(bishop));
        if (get_attacks(BISHOP, from, by_color, occupied) & square_bb(sq)) {
            return true;
        }
    }
    
    // Check rook attacks using step-by-step approach
    Bitboard rooks = pieces[by_color][ROOK];
    while (rooks) {
        Bitboard rook = pop_lsb(rooks);
        Square from = Square(lsb(rook));
        if (get_attacks(ROOK, from, by_color, occupied) & square_bb(sq)) {
            return true;
        }
    }
    
    // Check queen attacks using step-by-step approach
    Bitboard queens = pieces[by_color][QUEEN];
    while (queens) {
        Bitboard queen = pop_lsb(queens);
        Square from = Square(lsb(queen));
        if (get_attacks(QUEEN, from, by_color, occupied) & square_bb(sq)) {
            return true;
        }
    }
    
    // Check king attacks
    if (KING_ATTACKS[sq] & pieces[by_color][KING]) return true;
    
    return false;
}

Bitboard get_attacks(Piece piece, Square sq, Color color, Bitboard occupied) {
    switch (piece) {
        case PAWN:
            return PAWN_ATTACKS[color][sq];
        case KNIGHT:
            return KNIGHT_ATTACKS[sq];
        case BISHOP: {
            Bitboard attacks = 0;
            int file = file_of(sq);
            int rank = rank_of(sq);
            
            // Check all four diagonal directions
            const int bishop_dx[] = {-1, -1, 1, 1};
            const int bishop_dy[] = {-1, 1, -1, 1};
            
            for (int i = 0; i < 4; i++) {
                int new_file = file + bishop_dx[i];
                int new_rank = rank + bishop_dy[i];
                
                while (new_file >= 0 && new_file < 8 && new_rank >= 0 && new_rank < 8) {
                    Square target = make_square(File(new_file), Rank(new_rank));
                    set_bit(attacks, target);
                    
                    // If this square is occupied, stop
                    if (test_bit(occupied, target)) {
                        break;
                    }
                    
                    new_file += bishop_dx[i];
                    new_rank += bishop_dy[i];
                }
            }
            return attacks;
        }
        case ROOK: {
            Bitboard attacks = 0;
            int file = file_of(sq);
            int rank = rank_of(sq);
            
            // Check all four orthogonal directions
            const int rook_dx[] = {-1, 1, 0, 0};
            const int rook_dy[] = {0, 0, -1, 1};
            
            for (int i = 0; i < 4; i++) {
                int new_file = file + rook_dx[i];
                int new_rank = rank + rook_dy[i];
                
                while (new_file >= 0 && new_file < 8 && new_rank >= 0 && new_rank < 8) {
                    Square target = make_square(File(new_file), Rank(new_rank));
                    set_bit(attacks, target);
                    
                    // If this square is occupied, stop
                    if (test_bit(occupied, target)) {
                        break;
                    }
                    
                    new_file += rook_dx[i];
                    new_rank += rook_dy[i];
                }
            }
            return attacks;
        }
        case QUEEN: {
            Bitboard attacks = 0;
            int file = file_of(sq);
            int rank = rank_of(sq);
            
            // Check all eight directions (orthogonal + diagonal)
            const int queen_dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
            const int queen_dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
            
            for (int i = 0; i < 8; i++) {
                int new_file = file + queen_dx[i];
                int new_rank = rank + queen_dy[i];
                
                while (new_file >= 0 && new_file < 8 && new_rank >= 0 && new_rank < 8) {
                    Square target = make_square(File(new_file), Rank(new_rank));
                    set_bit(attacks, target);
                    
                    // If this square is occupied, stop
                    if (test_bit(occupied, target)) {
                        break;
                    }
                    
                    new_file += queen_dx[i];
                    new_rank += queen_dy[i];
                }
            }
            return attacks;
        }
        case KING:
            return KING_ATTACKS[sq];
        default:
            return 0;
    }
}

} // namespace BitboardUtils 