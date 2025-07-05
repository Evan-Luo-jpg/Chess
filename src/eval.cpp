#include "eval.h"
#include "bitboard.h"

namespace Eval {

// Piece values in centipawns
const int PIECE_VALUES[6] = {
    100,   // PAWN
    320,   // KNIGHT
    330,   // BISHOP
    500,   // ROOK
    900,   // QUEEN
    20000  // KING
};

// Pawn piece-square table (from white's perspective)
const int PAWN_TABLE[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

// Knight piece-square table
const int KNIGHT_TABLE[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

// Bishop piece-square table
const int BISHOP_TABLE[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

// Rook piece-square table
const int ROOK_TABLE[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

// Queen piece-square table
const int QUEEN_TABLE[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

// King piece-square table (middlegame)
const int KING_TABLE[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

// King piece-square table (endgame)
const int KING_ENDGAME_TABLE[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

int evaluate(const Board& board) {
    int score = 0;
    
    // Material and positional evaluation
    score += evaluate_material(board);
    score += evaluate_position(board);
    
    // Endgame evaluation
    if (is_endgame(board)) {
        score += evaluate_endgame(board);
    }
    
    // Return score from white's perspective
    return (board.get_side_to_move() == WHITE) ? score : -score;
}

int evaluate_material(const Board& board) {
    int score = 0;
    
    for (int piece = PAWN; piece <= KING; piece++) {
        score += PIECE_VALUES[piece] * board.get_piece_count(WHITE, Piece(piece));
        score -= PIECE_VALUES[piece] * board.get_piece_count(BLACK, Piece(piece));
    }
    
    return score;
}

int evaluate_position(const Board& board) {
    int score = 0;
    
    // Evaluate piece-square tables for each piece
    for (int piece = PAWN; piece <= KING; piece++) {
        Bitboard white_pieces = board.get_pieces(WHITE, Piece(piece));
        Bitboard black_pieces = board.get_pieces(BLACK, Piece(piece));
        
        while (white_pieces) {
            Bitboard piece_bb = BitboardUtils::pop_lsb(white_pieces);
            Square sq = Square(BitboardUtils::lsb(piece_bb));
            score += get_piece_square_value(Piece(piece), sq, WHITE);
        }
        
        while (black_pieces) {
            Bitboard piece_bb = BitboardUtils::pop_lsb(black_pieces);
            Square sq = Square(BitboardUtils::lsb(piece_bb));
            score -= get_piece_square_value(Piece(piece), sq, BLACK);
        }
    }
    
    return score;
}

int evaluate_endgame(const Board& board) {
    int score = 0;
    
    // King tropism (encourage kings to move toward each other in endgames)
    Square white_king = board.get_king_square(WHITE);
    Square black_king = board.get_king_square(BLACK);
    
    if (white_king != SQUARE_NONE && black_king != SQUARE_NONE) {
        int file_distance = abs(BitboardUtils::file_of(white_king) - BitboardUtils::file_of(black_king));
        int rank_distance = abs(BitboardUtils::rank_of(white_king) - BitboardUtils::rank_of(black_king));
        int king_distance = file_distance + rank_distance;
        
        // Encourage kings to be close in endgames (but not too close)
        if (king_distance > 2) {
            score += (14 - king_distance) * 10;
        }
    }
    
    return score;
}

bool is_endgame(const Board& board) {
    // Simple endgame detection: few pieces remaining
    int total_pieces = 0;
    for (int piece = PAWN; piece <= QUEEN; piece++) {
        total_pieces += board.get_piece_count(WHITE, Piece(piece));
        total_pieces += board.get_piece_count(BLACK, Piece(piece));
    }
    
    return total_pieces <= 12; // Arbitrary threshold
}

int get_piece_value(Piece piece) {
    if (piece >= 0 && piece < 6) {
        return PIECE_VALUES[piece];
    }
    return 0;
}

int get_piece_square_value(Piece piece, Square sq, Color color) {
    if (piece < 0 || piece >= 6 || sq < 0 || sq >= 64) return 0;
    
    // Get the square index from white's perspective
    Square white_sq = (color == WHITE) ? sq : BitboardUtils::relative_square(WHITE, sq);
    
    switch (piece) {
        case PAWN:
            return PAWN_TABLE[white_sq];
        case KNIGHT:
            return KNIGHT_TABLE[white_sq];
        case BISHOP:
            return BISHOP_TABLE[white_sq];
        case ROOK:
            return ROOK_TABLE[white_sq];
        case QUEEN:
            return QUEEN_TABLE[white_sq];
        case KING:
            // Use a simple heuristic: if there are few pieces, it's likely an endgame
            return (white_sq >= 16 && white_sq <= 47) ? KING_ENDGAME_TABLE[white_sq] : KING_TABLE[white_sq];
        default:
            return 0;
    }
}

} // namespace Eval 