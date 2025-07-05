#include "board.h"
#include "movegen.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// Move constructor
Move::Move(Square from, Square to, Type type, Piece promotion) {
    data = from | (to << 6) | (type << 12);
    if (promotion != PIECE_NONE) {
        data |= (promotion << 12);
    }
}

std::string Move::to_string() const {
    std::string result = BitboardUtils::square_to_string(from()) + BitboardUtils::square_to_string(to());
    if (is_promotion()) {
        const char promo_chars[] = {'q', 'r', 'b', 'n'};
        result += promo_chars[promotion() - 1];
    }
    return result;
}

Move Move::from_string(const std::string& str) {
    if (str.length() < 4) return Move();
    
    Square from = BitboardUtils::string_to_square(str.substr(0, 2));
    Square to = BitboardUtils::string_to_square(str.substr(2, 2));
    
    if (from == SQUARE_NONE || to == SQUARE_NONE) return Move();
    
    Type type = NORMAL;
    Piece promotion = PIECE_NONE;
    
    // Check for double pawn push
    int from_rank = BitboardUtils::rank_of(from);
    int to_rank = BitboardUtils::rank_of(to);
    int from_file = BitboardUtils::file_of(from);
    int to_file = BitboardUtils::file_of(to);
    
    // If it's a pawn moving two squares forward from starting rank, it's a double pawn push
    if (from_file == to_file && abs(to_rank - from_rank) == 2) {
        if ((from_rank == 1 && to_rank == 3) || (from_rank == 6 && to_rank == 4)) {
            type = DOUBLE_PAWN_PUSH;
        }
    }
    
    // Check for castling
    if ((from == E1 && to == G1) || (from == E8 && to == G8)) {
        type = KING_CASTLE;
    } else if ((from == E1 && to == C1) || (from == E8 && to == C8)) {
        type = QUEEN_CASTLE;
    }
    
    if (str.length() > 4) {
        char promo = str[4];
        switch (promo) {
            case 'q': promotion = QUEEN; type = PROMOTION; break;
            case 'r': promotion = ROOK; type = PROMOTION; break;
            case 'b': promotion = BISHOP; type = PROMOTION; break;
            case 'n': promotion = KNIGHT; type = PROMOTION; break;
        }
    }
    
    return Move(from, to, type, promotion);
}

// Board implementation
Board::Board() {
    clear_board();
    reset_to_starting_position();
}

Board::Board(const std::string& fen) {
    clear_board();
    set_fen(fen);
}

void Board::clear_board() {
    for (int c = 0; c < 2; c++) {
        for (int p = 0; p < 6; p++) {
            pieces[c][p] = 0;
        }
    }
    state = GameState();
    move_history.clear();
}

void Board::set_piece(Square sq, Piece piece, Color color) {
    if (piece != PIECE_NONE && color != COLOR_NONE) {
        set_bit(pieces[color][piece], sq);
    }
}

Piece Board::get_piece(Square sq) const {
    for (int p = 0; p < 6; p++) {
        if (test_bit(pieces[WHITE][p], sq)) return Piece(p);
        if (test_bit(pieces[BLACK][p], sq)) return Piece(p);
    }
    return PIECE_NONE;
}

Color Board::get_color(Square sq) const {
    for (int p = 0; p < 6; p++) {
        if (test_bit(pieces[WHITE][p], sq)) return WHITE;
        if (test_bit(pieces[BLACK][p], sq)) return BLACK;
    }
    return COLOR_NONE;
}

Bitboard Board::get_occupied() const {
    Bitboard occupied = 0;
    for (int c = 0; c < 2; c++) {
        for (int p = 0; p < 6; p++) {
            occupied |= pieces[c][p];
        }
    }
    return occupied;
}

Bitboard Board::get_occupied_by_color(Color color) const {
    Bitboard occupied = 0;
    for (int p = 0; p < 6; p++) {
        occupied |= pieces[color][p];
    }
    return occupied;
}

bool Board::is_square_occupied(Square sq) const {
    return get_piece(sq) != PIECE_NONE;
}

void Board::reset_to_starting_position() {
    set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::set_fen(const std::string& fen) {
    clear_board();
    
    std::istringstream iss(fen);
    std::string board_part, turn_part, castle_part, ep_part, halfmove_part, fullmove_part;
    
    iss >> board_part >> turn_part >> castle_part >> ep_part >> halfmove_part >> fullmove_part;
    
    // Parse board
    int rank = 7, file = 0;
    for (char c : board_part) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += c - '0';
        } else {
            Color color = isupper(c) ? WHITE : BLACK;
            Piece piece;
            switch (tolower(c)) {
                case 'p': piece = PAWN; break;
                case 'n': piece = KNIGHT; break;
                case 'b': piece = BISHOP; break;
                case 'r': piece = ROOK; break;
                case 'q': piece = QUEEN; break;
                case 'k': piece = KING; break;
                default: continue;
            }
            set_piece(BitboardUtils::make_square(File(file), Rank(rank)), piece, color);
            file++;
        }
    }
    
    // Parse turn
    state.side_to_move = (turn_part == "w") ? WHITE : BLACK;
    
    // Parse castling rights
    state.castling_rights[WHITE][1] = (castle_part.find('K') != std::string::npos);
    state.castling_rights[WHITE][0] = (castle_part.find('Q') != std::string::npos);
    state.castling_rights[BLACK][1] = (castle_part.find('k') != std::string::npos);
    state.castling_rights[BLACK][0] = (castle_part.find('q') != std::string::npos);
    
    // Parse en passant square
    state.en_passant_square = (ep_part == "-") ? SQUARE_NONE : BitboardUtils::string_to_square(ep_part);
    
    // Parse move counters
    state.halfmove_clock = halfmove_part.empty() ? 0 : std::stoi(halfmove_part);
    state.fullmove_number = fullmove_part.empty() ? 1 : std::stoi(fullmove_part);
}

std::string Board::get_fen() const {
    std::ostringstream oss;
    
    // Board part
    for (int rank = 7; rank >= 0; rank--) {
        int empty_count = 0;
        for (int file = 0; file < 8; file++) {
            Square sq = BitboardUtils::make_square(File(file), Rank(rank));
            Piece piece = get_piece(sq);
            Color color = get_color(sq);
            
            if (piece == PIECE_NONE) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    oss << empty_count;
                    empty_count = 0;
                }
                char piece_char;
                switch (piece) {
                    case PAWN: piece_char = 'p'; break;
                    case KNIGHT: piece_char = 'n'; break;
                    case BISHOP: piece_char = 'b'; break;
                    case ROOK: piece_char = 'r'; break;
                    case QUEEN: piece_char = 'q'; break;
                    case KING: piece_char = 'k'; break;
                    default: piece_char = '?'; break;
                }
                oss << (color == WHITE ? (char)toupper(piece_char) : piece_char);
            }
        }
        if (empty_count > 0) {
            oss << empty_count;
        }
        if (rank > 0) oss << '/';
    }
    
    // Turn
    oss << (state.side_to_move == WHITE ? " w " : " b ");
    
    // Castling rights
    bool has_castling = false;
    if (state.castling_rights[WHITE][1]) { oss << 'K'; has_castling = true; }
    if (state.castling_rights[WHITE][0]) { oss << 'Q'; has_castling = true; }
    if (state.castling_rights[BLACK][1]) { oss << 'k'; has_castling = true; }
    if (state.castling_rights[BLACK][0]) { oss << 'q'; has_castling = true; }
    if (!has_castling) oss << '-';
    
    // En passant
    oss << ' ' << (state.en_passant_square == SQUARE_NONE ? "-" : BitboardUtils::square_to_string(state.en_passant_square));
    
    // Move counters
    oss << ' ' << state.halfmove_clock << ' ' << state.fullmove_number;
    
    return oss.str();
}

void Board::make_move(const Move& move) {
    // Save current state
    MoveInfo info;
    info.move = move;
    info.en_passant_square = state.en_passant_square;
    info.halfmove_clock = state.halfmove_clock;
    for (int c = 0; c < 2; c++) {
        for (int s = 0; s < 2; s++) {
            info.castling_rights[c][s] = state.castling_rights[c][s];
        }
    }
    
    Square from = move.from();
    Square to = move.to();
    Piece piece = get_piece(from);
    Color color = get_color(from);
    
    // Handle capture
    if (is_square_occupied(to)) {
        info.captured_piece = get_piece(to);
        info.captured_square = to;
        clear_bit(pieces[get_color(to)][info.captured_piece], to);
    } else {
        info.captured_piece = PIECE_NONE;
        info.captured_square = SQUARE_NONE;
    }
    
    // Move piece
    clear_bit(pieces[color][piece], from);
    set_bit(pieces[color][piece], to);
    
    // Handle special moves
    switch (move.type()) {
        case Move::DOUBLE_PAWN_PUSH: {
            state.en_passant_square = BitboardUtils::make_square(BitboardUtils::file_of(from), 
                                                               Rank(BitboardUtils::rank_of(from) + (color == WHITE ? 1 : -1)));
            break;
        }
            
        case Move::EN_PASSANT: {
            Square ep_square = state.en_passant_square;
            clear_bit(pieces[!color][PAWN], ep_square);
            info.captured_piece = PAWN;
            info.captured_square = ep_square;
            break;
        }
            
        case Move::KING_CASTLE: {
            // Move rook
            Square rook_from = (color == WHITE) ? H1 : H8;
            Square rook_to = (color == WHITE) ? F1 : F8;
            clear_bit(pieces[color][ROOK], rook_from);
            set_bit(pieces[color][ROOK], rook_to);
            break;
        }
            
        case Move::QUEEN_CASTLE: {
            // Move rook
            Square rook_from_q = (color == WHITE) ? A1 : A8;
            Square rook_to_q = (color == WHITE) ? D1 : D8;
            clear_bit(pieces[color][ROOK], rook_from_q);
            set_bit(pieces[color][ROOK], rook_to_q);
            break;
        }
            
        case Move::PROMOTION:
        case Move::PROMOTION_CAPTURE: {
            clear_bit(pieces[color][PAWN], to);
            set_bit(pieces[color][move.promotion()], to);
            break;
        }
    }
    
    // Update castling rights
    if (piece == KING) {
        state.castling_rights[color][0] = state.castling_rights[color][1] = false;
    } else if (piece == ROOK) {
        if (from == (color == WHITE ? A1 : A8)) state.castling_rights[color][0] = false;
        if (from == (color == WHITE ? H1 : H8)) state.castling_rights[color][1] = false;
    }
    
    // Update en passant square
    if (piece != PAWN && !move.is_capture()) {
        state.en_passant_square = SQUARE_NONE;
    }
    
    // Update move counters
    if (piece == PAWN || move.is_capture()) {
        state.halfmove_clock = 0;
    } else {
        state.halfmove_clock++;
    }
    
    if (state.side_to_move == BLACK) {
        state.fullmove_number++;
    }
    
    // Switch sides
    state.side_to_move = (state.side_to_move == WHITE) ? BLACK : WHITE;
    
    // Save move info
    move_history.push_back(info);
}

void Board::undo_move() {
    if (move_history.empty()) return;
    
    MoveInfo info = move_history.back();
    move_history.pop_back();
    
    // Restore state
    state.side_to_move = (state.side_to_move == WHITE) ? BLACK : WHITE;
    state.en_passant_square = info.en_passant_square;
    state.halfmove_clock = info.halfmove_clock;
    for (int c = 0; c < 2; c++) {
        for (int s = 0; s < 2; s++) {
            state.castling_rights[c][s] = info.castling_rights[c][s];
        }
    }
    
    Move move = info.move;
    Square from = move.from();
    Square to = move.to();
    Piece piece = get_piece(to);
    Color color = get_color(to);
    
    // Restore piece to original square
    clear_bit(pieces[color][piece], to);
    set_bit(pieces[color][piece], from);
    
    // Handle special moves
    switch (move.type()) {
        case Move::KING_CASTLE: {
            // Restore rook
            Square rook_from = (color == WHITE) ? H1 : H8;
            Square rook_to = (color == WHITE) ? F1 : F8;
            clear_bit(pieces[color][ROOK], rook_to);
            set_bit(pieces[color][ROOK], rook_from);
            break;
        }
            
        case Move::QUEEN_CASTLE: {
            // Restore rook
            Square rook_from_q = (color == WHITE) ? A1 : A8;
            Square rook_to_q = (color == WHITE) ? D1 : D8;
            clear_bit(pieces[color][ROOK], rook_to_q);
            set_bit(pieces[color][ROOK], rook_from_q);
            break;
        }
            
        case Move::PROMOTION:
        case Move::PROMOTION_CAPTURE: {
            clear_bit(pieces[color][move.promotion()], from);
            set_bit(pieces[color][PAWN], from);
            break;
        }
    }
    
    // Restore captured piece
    if (info.captured_piece != PIECE_NONE) {
        Color captured_color = (color == WHITE) ? BLACK : WHITE;
        set_bit(pieces[captured_color][info.captured_piece], info.captured_square);
    }
}

std::vector<Move> Board::generate_legal_moves() const {
    std::vector<Move> pseudo_legal = generate_pseudo_legal_moves();
    std::vector<Move> legal;
    
    for (const Move& move : pseudo_legal) {
        // Make move temporarily
        Board temp_board = *this;
        temp_board.make_move(move);

        //If the king is currently in check, check if king isn't after the move
        // rnb1kbnr/pp1pp1pp/2p2p2/q7/3PP3/5N2/PPPQ1PPP/RNB1KB1R b KQkq - 3 4
        if (this->is_in_check(state.side_to_move) && !temp_board.is_in_check(state.side_to_move)){
            legal.push_back(move);
        }else if(!temp_board.is_in_check(state.side_to_move)) {
            // Check if king is in check after move
            legal.push_back(move);
        }
    }
    
    return legal;
}

std::vector<Move> Board::generate_pseudo_legal_moves() const {
    return MoveGen::generate_moves(*this);
}

std::vector<Move> Board::generate_captures() const {
    return MoveGen::generate_captures(*this);
}

bool Board::is_in_check(Color color) const {
    Square king_square = get_king_square(color);
    if (king_square == SQUARE_NONE) {
        // If king is not found, something is wrong with the position
        return false;
    }
    return BitboardUtils::is_attacked(king_square, (color == WHITE) ? BLACK : WHITE, pieces);
}

bool Board::is_checkmate(Color color) const {
    return is_in_check(color) && generate_legal_moves().empty();
}

bool Board::is_stalemate(Color color) const {
    return !is_in_check(color) && generate_legal_moves().empty();
}

bool Board::is_game_over() const {
    return is_checkmate(state.side_to_move) || is_stalemate(state.side_to_move) || is_draw();
}

bool Board::is_check() const {
    return is_in_check(state.side_to_move);
}

bool Board::is_checkmate() const {
    return is_checkmate(state.side_to_move);
}

bool Board::is_stalemate() const {
    return is_stalemate(state.side_to_move);
}

bool Board::is_draw() const {
    // Simple draw detection - can be expanded
    return state.halfmove_clock >= 50;
}

Color Board::get_winner() const {
    if (is_checkmate(WHITE)) return BLACK;
    if (is_checkmate(BLACK)) return WHITE;
    return COLOR_NONE;
}

Bitboard Board::get_all_pieces(Color color) const {
    Bitboard all = 0;
    for (int p = 0; p < 6; p++) {
        all |= pieces[color][p];
    }
    return all;
}

void Board::print_board() const {
    std::cout << get_board_string() << std::endl;
}

std::string Board::get_board_string() const {
    std::ostringstream oss;
    oss << "\n  +---+---+---+---+---+---+---+---+\n";
    
    for (int rank = 7; rank >= 0; rank--) {
        oss << (rank + 1) << " |";
        for (int file = 0; file < 8; file++) {
            Square sq = BitboardUtils::make_square(File(file), Rank(rank));
            Piece piece = get_piece(sq);
            Color color = get_color(sq);
            
            char piece_char;
            switch (piece) {
                case PAWN: piece_char = 'p'; break;
                case KNIGHT: piece_char = 'n'; break;
                case BISHOP: piece_char = 'b'; break;
                case ROOK: piece_char = 'r'; break;
                case QUEEN: piece_char = 'q'; break;
                case KING: piece_char = 'k'; break;
                default: piece_char = ' '; break;
            }
            
            if (color == WHITE) piece_char = toupper(piece_char);
            oss << " " << piece_char << " |";
        }
        oss << "\n  +---+---+---+---+---+---+---+---+\n";
    }
    oss << "    a   b   c   d   e   f   g   h\n";
    
    return oss.str();
}

int Board::get_piece_count(Color color, Piece piece) const {
    return BitboardUtils::popcount(pieces[color][piece]);
}

Square Board::get_king_square(Color color) const {
    Bitboard king_bb = pieces[color][KING];
    if (king_bb == 0) return SQUARE_NONE;
    return Square(BitboardUtils::lsb(king_bb));
}

bool Board::is_valid_move(const std::string& move_str) const {
    if (move_str.length() < 4) return false;
    
    Square from = BitboardUtils::string_to_square(move_str.substr(0, 2));
    Square to = BitboardUtils::string_to_square(move_str.substr(2, 2));
    
    if (from == SQUARE_NONE || to == SQUARE_NONE) return false;
    
    // Check if the piece at 'from' belongs to the side to move
    Color piece_color = get_color(from);
    if (piece_color != state.side_to_move) return false;
    
    // Determine move type based on the position
    Move::Type type = Move::NORMAL;
    Piece promotion = PIECE_NONE;
    
    // Check for captures
    Color target_color = get_color(to);
    if (target_color != COLOR_NONE && target_color != state.side_to_move) {
        type = Move::CAPTURE;
    }
    
    // Check for double pawn push
    Piece piece = get_piece(from);
    if (piece == PAWN) {
        int from_rank = BitboardUtils::rank_of(from);
        int to_rank = BitboardUtils::rank_of(to);
        int from_file = BitboardUtils::file_of(from);
        int to_file = BitboardUtils::file_of(to);
        
        if (from_file == to_file && abs(to_rank - from_rank) == 2) {
            if ((from_rank == 1 && to_rank == 3) || (from_rank == 6 && to_rank == 4)) {
                type = Move::DOUBLE_PAWN_PUSH;
            }
        }
        
        // Check for promotion
        if ((state.side_to_move == WHITE && to_rank == 7) ||
            (state.side_to_move == BLACK && to_rank == 0)) {
            if (move_str.length() > 4) {
                char promo = move_str[4];
                switch (promo) {
                    case 'q': promotion = QUEEN; break;
                    case 'r': promotion = ROOK; break;
                    case 'b': promotion = BISHOP; break;
                    case 'n': promotion = KNIGHT; break;
                    default: return false;
                }
                type = (type == Move::CAPTURE) ? Move::PROMOTION_CAPTURE : Move::PROMOTION;
            }
        }
    }
    
    // Check for en passant
    if (piece == PAWN && to == state.en_passant_square) {
        type = Move::EN_PASSANT;
    }
    
    // Check for castling
    if (piece == KING) {
        if ((state.side_to_move == WHITE && from == E1 && to == G1) ||
            (state.side_to_move == BLACK && from == E8 && to == G8)) {
            type = Move::KING_CASTLE;
            std::cout << "DEBUG: Detected KING_CASTLE move\n";
        } else if ((state.side_to_move == WHITE && from == E1 && to == C1) ||
                   (state.side_to_move == BLACK && from == E8 && to == C8)) {
            type = Move::QUEEN_CASTLE;
            std::cout << "DEBUG: Detected QUEEN_CASTLE move\n";
        }
    }
    
    Move move(from, to, type, promotion);
    std::cout << "DEBUG: Created move with type: " << type << "\n";
    
    std::vector<Move> legal_moves = generate_legal_moves();
    std::cout << "DEBUG: Generated " << legal_moves.size() << " legal moves\n";
    
    // Check if our move is in the legal moves
    bool found = false;
    for (const Move& legal_move : legal_moves) {
        if (legal_move.from() == move.from() && legal_move.to() == move.to() && 
            legal_move.type() == move.type()) {
            found = true;
            break;
        }
    }
    std::cout << "DEBUG: Move found in legal moves: " << (found ? "YES" : "NO") << "\n";
    
    return std::find(legal_moves.begin(), legal_moves.end(), move) != legal_moves.end();
}

Move Board::parse_move(const std::string& move_str) const {
    return Move::from_string(move_str);
} 