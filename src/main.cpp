#include "board.h"
#include "search.h"
#include "eval.h"
#include "bitboard.h"
#include <iostream>
#include <string>
#include <chrono>

void print_help() {
    std::cout << "\nChess Engine Commands:\n";
    std::cout << "  move <from><to>     - Make a move (e.g., 'move e2e4')\n";
    std::cout << "  move <from><to><promo> - Make a promotion move (e.g., 'move e7e8q')\n";
    std::cout << "  go                  - Let engine make a move\n";
    std::cout << "  depth <n>           - Set search depth (default: 4)\n";
    std::cout << "  time <ms>           - Set max search time in milliseconds (default: 5000)\n";
    std::cout << "  fen <string>        - Set position from FEN string\n";
    std::cout << "  reset               - Reset to starting position\n";
    std::cout << "  eval                - Show current position evaluation\n";
    std::cout << "  legal               - Show all legal moves\n";
    std::cout << "  help                - Show this help\n";
    std::cout << "  quit                - Exit the program\n\n";
}

void print_game_status(const Board& board) {
    std::cout << "\n";
    board.print_board();
    
    std::cout << "\nFEN: " << board.get_fen() << "\n";
    std::cout << "Side to move: " << (board.get_side_to_move() == WHITE ? "White" : "Black") << "\n";
    
    if (board.is_check()) {
        std::cout << "CHECK!\n";
    }
    
    if (board.is_game_over()) {
        if (board.is_checkmate()) {
            Color winner = board.get_winner();
            std::cout << "CHECKMATE! " << (winner == WHITE ? "White" : "Black") << " wins!\n";
        } else if (board.is_stalemate()) {
            std::cout << "STALEMATE! Draw.\n";
        } else if (board.is_draw()) {
            std::cout << "DRAW!\n";
        }
    }
}

int main() {
    std::cout << "=== C++ Chess Engine ===\n";
    std::cout << "A highly optimized chess engine using bitboards and magic bitboards\n\n";
    
    // Initialize bitboard lookup tables
    BitboardUtils::init();
    
    // Create board and search parameters
    Board board;
    Search::SearchParams search_params;
    
    std::cout << "Starting position:\n";
    print_game_status(board);
    print_help();
    
    std::string command;
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, command);
        
        if (command.empty()) continue;
        
        if (command == "quit" || command == "exit") {
            break;
        } else if (command == "help") {
            print_help();
        } else if (command.substr(0, 4) == "move") {
            if (command.length() < 9) {
                std::cout << "Invalid move format. Use 'move <from><to>' or 'move <from><to><promo>'\n";
                continue;
            }
            
            std::string move_str = command.substr(5);
            if (!board.is_valid_move(move_str)) {
                std::cout << "Invalid move: " << move_str << "\n";
                continue;
            }
            
            Move move = board.parse_move(move_str);
            board.make_move(move);
            std::cout << "Move played: " << move.to_string() << "\n";
            print_game_status(board);
            
            // Check if game is over after player's move
            if (board.is_game_over()) {
                continue;
            }
            
            // Engine's turn
            std::cout << "\nEngine is thinking...\n";
            auto start_time = std::chrono::steady_clock::now();
            
            Search::SearchResult result = Search::search(board, search_params);
            
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            if (result.best_move.data != 0) {
                board.make_move(result.best_move);
                std::cout << "Engine plays: " << result.best_move.to_string() 
                         << " (depth: " << result.depth 
                         << ", score: " << result.score 
                         << ", time: " << duration.count() << "ms)\n";
                print_game_status(board);
            } else {
                std::cout << "Engine found no legal moves.\n";
            }
            
        } else if (command == "go") {
            if (board.is_game_over()) {
                std::cout << "Game is over. Use 'reset' to start a new game.\n";
                continue;
            }
            
            std::cout << "Engine is thinking...\n";
            auto start_time = std::chrono::steady_clock::now();
            
            Search::SearchResult result = Search::search(board, search_params);
            
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            if (result.best_move.data != 0) {
                board.make_move(result.best_move);
                std::cout << "Engine plays: " << result.best_move.to_string() 
                         << " (depth: " << result.depth 
                         << ", score: " << result.score 
                         << ", time: " << duration.count() << "ms)\n";
                print_game_status(board);
            } else {
                std::cout << "Engine found no legal moves.\n";
            }
            
        } else if (command.substr(0, 5) == "depth") {
            try {
                int depth = std::stoi(command.substr(6));
                if (depth > 0 && depth <= 10) {
                    search_params.max_depth = depth;
                    std::cout << "Search depth set to " << depth << "\n";
                } else {
                    std::cout << "Depth must be between 1 and 10\n";
                }
            } catch (...) {
                std::cout << "Invalid depth value\n";
            }
            
        } else if (command.substr(0, 4) == "time") {
            try {
                int time_ms = std::stoi(command.substr(5));
                if (time_ms > 0 && time_ms <= 60000) {
                    search_params.max_time_ms = time_ms;
                    std::cout << "Max search time set to " << time_ms << "ms\n";
                } else {
                    std::cout << "Time must be between 1 and 60000ms\n";
                }
            } catch (...) {
                std::cout << "Invalid time value\n";
            }
            
        } else if (command.substr(0, 3) == "fen") {
            std::string fen = command.substr(4);
            try {
                board.set_fen(fen);
                std::cout << "Position set from FEN\n";
                print_game_status(board);
            } catch (...) {
                std::cout << "Invalid FEN string\n";
            }
            
        } else if (command == "reset") {
            board.reset_to_starting_position();
            std::cout << "Board reset to starting position\n";
            print_game_status(board);
            
        } else if (command == "eval") {
            int eval = Eval::evaluate(board);
            std::cout << "Position evaluation: " << eval << " centipawns\n";
            if (eval > 0) {
                std::cout << "White is winning\n";
            } else if (eval < 0) {
                std::cout << "Black is winning\n";
            } else {
                std::cout << "Position is equal\n";
            }
            
        } else if (command == "legal") {
            std::vector<Move> legal_moves = board.generate_legal_moves();
            std::cout << "Legal moves (" << legal_moves.size() << "): ";
            for (size_t i = 0; i < legal_moves.size(); i++) {
                std::cout << legal_moves[i].to_string();
                if (i < legal_moves.size() - 1) std::cout << ", ";
            }
            std::cout << "\n";
            
        } else {
            std::cout << "Unknown command. Type 'help' for available commands.\n";
        }
    }
    
    std::cout << "Goodbye!\n";
    return 0;
} 