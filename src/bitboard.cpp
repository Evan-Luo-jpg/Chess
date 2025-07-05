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
Bitboard BISHOP_ATTACKS[64][512];
Bitboard ROOK_ATTACKS[64][4096];
Bitboard BISHOP_MAGIC[64];
Bitboard ROOK_MAGIC[64];

// Magic numbers for sliding piece move generation
static const Bitboard BishopMagicNumbers[64] = {
    0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL,
    0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
    0x4050404440404ULL, 0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL,
    0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL, 0x801010402020200ULL,
    0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL, 0x4002801a02003ULL,
    0x85040820080400ULL, 0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL,
    0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL, 0x20080002081110ULL,
    0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL, 0x1c004001012080ULL,
    0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
    0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
    0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL, 0x1840060a44020800ULL,
    0x90080104000041ULL, 0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL,
    0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL, 0x300000261044000aULL,
    0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL, 0x4010801011c04ULL,
    0xa010109502200ULL, 0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL,
    0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL, 0x4010011029020020ULL
};

static const Bitboard RookMagicNumbers[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
    0x200020010080420ULL, 0x3001c0012010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
    0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
    0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
    0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
    0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
    0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
    0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
    0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
    0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
    0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
    0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
    0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
    0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL, 0x148020010100400ULL, 0x1244444044128000ULL, 0x2001310220000ULL
};

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
    
    // Initialize magic numbers
    for (int i = 0; i < 64; i++) {
        BISHOP_MAGIC[i] = BishopMagicNumbers[i];
        ROOK_MAGIC[i] = RookMagicNumbers[i];
    }
    
    // Initialize sliding piece attack tables
    for (int sq = 0; sq < 64; sq++) {
        // Bishop attacks
        Bitboard bishop_mask = (RAYS[4][sq] | RAYS[5][sq] | RAYS[6][sq] | RAYS[7][sq]) & ~(FILE_BB[0] | FILE_BB[7] | RANK_BB[0] | RANK_BB[7]);
        int bishop_bits = popcount(bishop_mask);
        
        for (Bitboard subset = 0; subset < (1ULL << bishop_bits); subset++) {
            Bitboard occupied = 0;
            Bitboard temp = bishop_mask;
            for (int i = 0; i < bishop_bits; i++) {
                int bit = lsb(temp);
                if (subset & (1ULL << i)) {
                    set_bit(occupied, Square(bit));
                }
                clear_bit(temp, Square(bit));
            }
            
            Bitboard attacks = 0;
            for (int dir = 4; dir < 8; dir++) {
                Bitboard ray = RAYS[dir][sq];
                Bitboard blockers = ray & occupied;
                if (blockers) {
                    // Find the first blocker
                    Square blocker = Square(lsb(blockers));
                    // Include squares up to and including the first blocker
                    attacks |= ray & ~(ray & (FULL_BB << (blocker + 1)));
                } else {
                    attacks |= ray;
                }
            }
            
            int index = (occupied * BISHOP_MAGIC[sq]) >> (64 - 9);
            BISHOP_ATTACKS[sq][index] = attacks;
        }
        
        // Rook attacks
        Bitboard rook_mask = (RAYS[0][sq] | RAYS[1][sq] | RAYS[2][sq] | RAYS[3][sq]) & ~(FILE_BB[0] | FILE_BB[7] | RANK_BB[0] | RANK_BB[7]);
        int rook_bits = popcount(rook_mask);
        
        for (Bitboard subset = 0; subset < (1ULL << rook_bits); subset++) {
            Bitboard occupied = 0;
            Bitboard temp = rook_mask;
            for (int i = 0; i < rook_bits; i++) {
                int bit = lsb(temp);
                if (subset & (1ULL << i)) {
                    set_bit(occupied, Square(bit));
                }
                clear_bit(temp, Square(bit));
            }
            
            Bitboard attacks = 0;
            for (int dir = 0; dir < 4; dir++) {
                Bitboard ray = RAYS[dir][sq];
                Bitboard blockers = ray & occupied;
                if (blockers) {
                    // Find the first blocker
                    Square blocker = Square(lsb(blockers));
                    // Include squares up to and including the first blocker
                    attacks |= ray & ~(ray & (FULL_BB << (blocker + 1)));
                } else {
                    attacks |= ray;
                }
            }
            
            int index = (occupied * ROOK_MAGIC[sq]) >> (64 - 12);
            ROOK_ATTACKS[sq][index] = attacks;
        }
    }
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