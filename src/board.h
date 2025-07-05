#pragma once
#include "bitboard.h"
#include <string>
#include <vector>

// Move structure (16 bits)
struct Move {
    uint16_t data;
    
    // Move types
    enum Type : uint16_t {
        NORMAL = 0,
        DOUBLE_PAWN_PUSH = 1,
        KING_CASTLE = 2,
        QUEEN_CASTLE = 3,
        CAPTURE = 4,
        EN_PASSANT = 5,
        PROMOTION = 8,
        PROMOTION_CAPTURE = 12
    };
    
    // Constructors
    Move() : data(0) {}
    Move(Square from, Square to, Type type = NORMAL, Piece promotion = PIECE_NONE);
    
    // Getters
    Square from() const { return Square(data & 0x3F); }
    Square to() const { return Square((data >> 6) & 0x3F); }
    Type type() const { return Type((data >> 12) & 0xF); }
    Piece promotion() const { return Piece((data >> 12) & 0x7); }
    
    // Utility
    bool is_capture() const { return type() == CAPTURE || type() == EN_PASSANT || type() == PROMOTION_CAPTURE; }
    bool is_promotion() const { return type() >= PROMOTION; }
    bool is_castle() const { return type() == KING_CASTLE || type() == QUEEN_CASTLE; }
    
    // Comparison
    bool operator==(const Move& other) const { return data == other.data; }
    bool operator!=(const Move& other) const { return data != other.data; }
    
    // String conversion
    std::string to_string() const;
    static Move from_string(const std::string& str);
};

// Game state information
struct GameState {
    Color side_to_move;
    Square en_passant_square;
    bool castling_rights[2][2]; // [color][kingside/queenside]
    int halfmove_clock;
    int fullmove_number;
    
    GameState() : side_to_move(WHITE), en_passant_square(SQUARE_NONE), 
                  halfmove_clock(0), fullmove_number(1) {
        castling_rights[WHITE][0] = castling_rights[WHITE][1] = true;
        castling_rights[BLACK][0] = castling_rights[BLACK][1] = true;
    }
};

// Board class
class Board {
private:
    // Piece bitboards [color][piece_type]
    Bitboard pieces[2][6];
    
    // Game state
    GameState state;
    
    // Move history for undoing moves
    struct MoveInfo {
        Move move;
        Piece captured_piece;
        Square captured_square;
        Square en_passant_square;
        bool castling_rights[2][2];
        int halfmove_clock;
    };
    std::vector<MoveInfo> move_history;
    
    // Utility functions
    void clear_board();
    void set_piece(Square sq, Piece piece, Color color);
    Color get_color(Square sq) const;
    Bitboard get_occupied_by_color(Color color) const;
    bool is_square_occupied(Square sq) const;
    bool is_legal_move(const Move& move) const;
    bool is_checkmate(Color color) const;
    bool is_stalemate(Color color) const;
    
public:
    // Constructors
    Board();
    Board(const std::string& fen);
    
    // Board manipulation
    void set_fen(const std::string& fen);
    std::string get_fen() const;
    void make_move(const Move& move);
    void undo_move();
    
    // Move generation
    std::vector<Move> generate_legal_moves() const;
    std::vector<Move> generate_pseudo_legal_moves() const;
    std::vector<Move> generate_captures() const;
    
    // Game state queries
    bool is_game_over() const;
    bool is_check() const;
    bool is_checkmate() const;
    bool is_stalemate() const;
    bool is_draw() const;
    Color get_winner() const;
    bool is_in_check(Color color) const;
    
    // Board queries
    Piece get_piece(Square sq) const;
    Bitboard get_pieces(Color color, Piece piece) const { return pieces[color][piece]; }
    Bitboard get_all_pieces(Color color) const;
    Bitboard get_occupied() const;
    GameState get_state() const { return state; }
    Color get_side_to_move() const { return state.side_to_move; }
    const Bitboard (*get_pieces_array() const)[6] { return pieces; }
    
    // Display
    void print_board() const;
    std::string get_board_string() const;
    
    // Utility
    void reset_to_starting_position();
    int get_piece_count(Color color, Piece piece) const;
    Square get_king_square(Color color) const;
    
    // Move validation
    bool is_valid_move(const std::string& move_str) const;
    Move parse_move(const std::string& move_str) const;
}; 