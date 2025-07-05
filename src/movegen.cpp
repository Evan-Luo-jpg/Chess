#include "movegen.h"
#include "bitboard.h"
#include <iostream>

namespace MoveGen {

std::vector<Move> generate_moves(const Board& board) {
    std::vector<Move> moves;
    Color color = board.get_side_to_move();
    
    std::vector<Move> pawn_moves = generate_pawn_moves(board, color);
    moves.insert(moves.end(), pawn_moves.begin(), pawn_moves.end());
    
    std::vector<Move> knight_moves = generate_knight_moves(board, color);
    moves.insert(moves.end(), knight_moves.begin(), knight_moves.end());
    
    std::vector<Move> bishop_moves = generate_bishop_moves(board, color);
    moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
    
    std::vector<Move> rook_moves = generate_rook_moves(board, color);
    moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());
    
    std::vector<Move> queen_moves = generate_queen_moves(board, color);
    moves.insert(moves.end(), queen_moves.begin(), queen_moves.end());
    
    std::vector<Move> king_moves = generate_king_moves(board, color);
    moves.insert(moves.end(), king_moves.begin(), king_moves.end());
    
    std::vector<Move> castling_moves = generate_castling_moves(board, color);
    moves.insert(moves.end(), castling_moves.begin(), castling_moves.end());
    
    return moves;
}

std::vector<Move> generate_captures(const Board& board) {
    std::vector<Move> moves = generate_moves(board);
    std::vector<Move> captures;
    
    for (const Move& move : moves) {
        if (move.is_capture()) {
            captures.push_back(move);
        }
    }
    
    return captures;
}

std::vector<Move> generate_pawn_moves(const Board& board, Color color) {
    std::vector<Move> moves;
    Bitboard pawns = board.get_pieces(color, PAWN);
    Bitboard occupied = board.get_occupied();
    Bitboard enemy_pieces = board.get_all_pieces((color == WHITE) ? BLACK : WHITE);
    Bitboard empty = ~occupied;
    
    // Pawn push direction
    int push_dir = (color == WHITE) ? 8 : -8;
    Bitboard push_targets = (color == WHITE) ? (pawns << 8) & empty : (pawns >> 8) & empty;
    
    // Single pawn pushes
    Bitboard single_pushes = push_targets;
    while (single_pushes) {
        Bitboard push = BitboardUtils::pop_lsb(single_pushes);
        Square to = Square(BitboardUtils::lsb(push));
        Square from = Square(to - push_dir);
        
        // Check for promotion
        if ((color == WHITE && BitboardUtils::rank_of(to) == 7) ||
            (color == BLACK && BitboardUtils::rank_of(to) == 0)) {
            add_promotion_moves(moves, from, to, false);
        } else {
            moves.emplace_back(from, to, Move::NORMAL);
        }
    }
    
    // Double pawn pushes
    Bitboard double_push_targets = (color == WHITE) ? 
        (push_targets << 8) & empty & RANK_BB[3] :
        (push_targets >> 8) & empty & RANK_BB[4];
    
    while (double_push_targets) {
        Bitboard push = BitboardUtils::pop_lsb(double_push_targets);
        Square to = Square(BitboardUtils::lsb(push));
        Square from = Square(to - 2 * push_dir);
        moves.emplace_back(from, to, Move::DOUBLE_PAWN_PUSH);
    }
    
    // Pawn captures
    Bitboard pawns_copy = pawns;
    while (pawns_copy) {
        Bitboard pawn = BitboardUtils::pop_lsb(pawns_copy);
        Square from = Square(BitboardUtils::lsb(pawn));
        Bitboard attacks = PAWN_ATTACKS[color][from] & enemy_pieces;
        
        while (attacks) {
            Bitboard attack = BitboardUtils::pop_lsb(attacks);
            Square to = Square(BitboardUtils::lsb(attack));
            
            // Check for promotion
            if ((color == WHITE && BitboardUtils::rank_of(to) == 7) ||
                (color == BLACK && BitboardUtils::rank_of(to) == 0)) {
                add_promotion_moves(moves, from, to, true);
            } else {
                moves.emplace_back(from, to, Move::CAPTURE);
            }
        }
    }
    
    // En passant captures
    Square ep_square = board.get_state().en_passant_square;
    if (ep_square != SQUARE_NONE) {
        Bitboard ep_attackers = PAWN_ATTACKS[color][ep_square] & pawns;
        while (ep_attackers) {
            Bitboard attacker = BitboardUtils::pop_lsb(ep_attackers);
            Square from = Square(BitboardUtils::lsb(attacker));
            moves.emplace_back(from, ep_square, Move::EN_PASSANT);
        }
    }
    
    return moves;
}

std::vector<Move> generate_knight_moves(const Board& board, Color color) {
    std::vector<Move> moves;
    Bitboard knights = board.get_pieces(color, KNIGHT);
    Bitboard occupied = board.get_occupied();
    Bitboard own_pieces = board.get_all_pieces(color);
    Bitboard enemy_pieces = board.get_all_pieces((color == WHITE) ? BLACK : WHITE);
    
    while (knights) {
        Bitboard knight = BitboardUtils::pop_lsb(knights);
        Square from = Square(BitboardUtils::lsb(knight));
        Bitboard attacks = KNIGHT_ATTACKS[from] & ~own_pieces;
        
        while (attacks) {
            Bitboard attack = BitboardUtils::pop_lsb(attacks);
            Square to = Square(BitboardUtils::lsb(attack));
            Move::Type type = (enemy_pieces & attack) ? Move::CAPTURE : Move::NORMAL;
            moves.emplace_back(from, to, type);
        }
    }
    
    return moves;
}

std::vector<Move> generate_bishop_moves(const Board& board, Color color) {
    std::vector<Move> moves;
    Bitboard bishops = board.get_pieces(color, BISHOP);
    Bitboard occupied = board.get_occupied();
    Bitboard own_pieces = board.get_all_pieces(color);
    Bitboard enemy_pieces = board.get_all_pieces((color == WHITE) ? BLACK : WHITE);
    
    while (bishops) {
        Bitboard bishop = BitboardUtils::pop_lsb(bishops);
        Square from = Square(BitboardUtils::lsb(bishop));
        Bitboard attacks = BitboardUtils::get_attacks(BISHOP, from, color, occupied) & ~own_pieces;
        
        while (attacks) {
            Bitboard attack = BitboardUtils::pop_lsb(attacks);
            Square to = Square(BitboardUtils::lsb(attack));
            Move::Type type = (enemy_pieces & attack) ? Move::CAPTURE : Move::NORMAL;
            moves.emplace_back(from, to, type);
        }
    }
    
    return moves;
}

std::vector<Move> generate_rook_moves(const Board& board, Color color) {
    std::vector<Move> moves;
    Bitboard rooks = board.get_pieces(color, ROOK);
    Bitboard occupied = board.get_occupied();
    Bitboard own_pieces = board.get_all_pieces(color);
    Bitboard enemy_pieces = board.get_all_pieces((color == WHITE) ? BLACK : WHITE);
    
    while (rooks) {
        Bitboard rook = BitboardUtils::pop_lsb(rooks);
        Square from = Square(BitboardUtils::lsb(rook));
        Bitboard attacks = BitboardUtils::get_attacks(ROOK, from, color, occupied) & ~own_pieces;
        
        while (attacks) {
            Bitboard attack = BitboardUtils::pop_lsb(attacks);
            Square to = Square(BitboardUtils::lsb(attack));
            Move::Type type = (enemy_pieces & attack) ? Move::CAPTURE : Move::NORMAL;
            moves.emplace_back(from, to, type);
        }
    }
    
    return moves;
}

std::vector<Move> generate_queen_moves(const Board& board, Color color) {
    std::vector<Move> moves;
    Bitboard queens = board.get_pieces(color, QUEEN);
    Bitboard occupied = board.get_occupied();
    Bitboard own_pieces = board.get_all_pieces(color);
    Bitboard enemy_pieces = board.get_all_pieces((color == WHITE) ? BLACK : WHITE);
    
    while (queens) {
        Bitboard queen = BitboardUtils::pop_lsb(queens);
        Square from = Square(BitboardUtils::lsb(queen));
        Bitboard attacks = BitboardUtils::get_attacks(QUEEN, from, color, occupied) & ~own_pieces;
        
        while (attacks) {
            Bitboard attack = BitboardUtils::pop_lsb(attacks);
            Square to = Square(BitboardUtils::lsb(attack));
            Move::Type type = (enemy_pieces & attack) ? Move::CAPTURE : Move::NORMAL;
            moves.emplace_back(from, to, type);
        }
    }
    
    return moves;
}

std::vector<Move> generate_king_moves(const Board& board, Color color) {
    std::vector<Move> moves;
    Bitboard kings = board.get_pieces(color, KING);
    Bitboard occupied = board.get_occupied();
    Bitboard own_pieces = board.get_all_pieces(color);
    Bitboard enemy_pieces = board.get_all_pieces((color == WHITE) ? BLACK : WHITE);
    
    while (kings) {
        Bitboard king = BitboardUtils::pop_lsb(kings);
        Square from = Square(BitboardUtils::lsb(king));
        Bitboard attacks = KING_ATTACKS[from] & ~own_pieces;
        
        while (attacks) {
            Bitboard attack = BitboardUtils::pop_lsb(attacks);
            Square to = Square(BitboardUtils::lsb(attack));
            Move::Type type = (enemy_pieces & attack) ? Move::CAPTURE : Move::NORMAL;
            moves.emplace_back(from, to, type);
        }
    }
    
    return moves;
}

std::vector<Move> generate_castling_moves(const Board& board, Color color) {
    std::vector<Move> moves;
    const GameState& state = board.get_state();
    
    // Check if king is in check
    if (board.is_in_check(color)) {
        return moves;
    }
    
    Bitboard occupied = board.get_occupied();
    
    // Kingside castling
    if (state.castling_rights[color][1]) {
        Square king_square = (color == WHITE) ? E1 : E8;
        Square rook_square = (color == WHITE) ? H1 : H8;
        Square king_to = (color == WHITE) ? G1 : G8;
        Square rook_to = (color == WHITE) ? F1 : F8;
        
        // Check if squares are empty and not attacked
        Bitboard castling_squares = square_bb(king_to) | square_bb(rook_to);
        if (!(occupied & castling_squares) && 
            !BitboardUtils::is_attacked(king_to, (color == WHITE) ? BLACK : WHITE, board.get_pieces_array()) &&
            !BitboardUtils::is_attacked(rook_to, (color == WHITE) ? BLACK : WHITE, board.get_pieces_array())) {
            moves.emplace_back(king_square, king_to, Move::KING_CASTLE);
        }
    }
    
    // Queenside castling
    if (state.castling_rights[color][0]) {
        Square king_square = (color == WHITE) ? E1 : E8;
        Square rook_square = (color == WHITE) ? A1 : A8;
        Square king_to = (color == WHITE) ? C1 : C8;
        Square rook_to = (color == WHITE) ? D1 : D8;
        
        // Check if squares are empty and not attacked
        Bitboard castling_squares = square_bb(king_to) | square_bb(rook_to) | square_bb((color == WHITE) ? B1 : B8);
        if (!(occupied & castling_squares) && 
            !BitboardUtils::is_attacked(king_to, (color == WHITE) ? BLACK : WHITE, board.get_pieces_array()) &&
            !BitboardUtils::is_attacked(rook_to, (color == WHITE) ? BLACK : WHITE, board.get_pieces_array())) {
            moves.emplace_back(king_square, king_to, Move::QUEEN_CASTLE);
        }
    }
    
    return moves;
}

void add_promotion_moves(std::vector<Move>& moves, Square from, Square to, bool is_capture) {
    Move::Type base_type = is_capture ? Move::PROMOTION_CAPTURE : Move::PROMOTION;
    
    for (Piece promo = QUEEN; promo >= KNIGHT; promo = Piece(promo - 1)) {
        moves.emplace_back(from, to, base_type, promo);
    }
}

} // namespace MoveGen 