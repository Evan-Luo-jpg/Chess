# C++ Chess Engine

A highly optimized chess engine written in C++ using bitboards and magic bitboards for fast move generation and evaluation. This engine is inspired by the "Programming a Chess Engine in C" YouTube tutorial series but implemented in modern C++ with additional optimizations.

## Features

### Core Engine
- **Bitboard Representation**: Uses 64-bit integers to represent the chess board for ultra-fast operations
- **Magic Bitboards**: Pre-computed lookup tables for sliding piece move generation (bishops and rooks)
- **Fast Move Generation**: Optimized move generation using bitwise operations
- **Alpha-Beta Pruning**: Minimax search with alpha-beta pruning for efficient tree exploration
- **Quiescence Search**: Capture-only search to avoid horizon effect
- **Iterative Deepening**: Progressive depth search with time management
- **Move Ordering**: Intelligent move ordering for better pruning efficiency

### Evaluation
- **Material Evaluation**: Standard piece values with bonus for material advantage
- **Positional Evaluation**: Piece-square tables for positional scoring
- **Endgame Detection**: Special evaluation for endgame positions
- **King Safety**: Basic king safety evaluation

### User Interface
- **Terminal Interface**: Clean ASCII board display
- **Interactive Commands**: Easy-to-use command system
- **FEN Support**: Load positions from FEN strings
- **Move Validation**: Automatic move legality checking
- **Game State Display**: Shows check, checkmate, stalemate, and draw conditions

## Building the Engine

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)
- Make or Ninja build system

### Build Instructions

1. **Clone and navigate to the project directory:**
   ```bash
   cd Chess
   ```

2. **Create build directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure and build:**
   ```bash
   cmake ..
   make -j$(nproc)
   ```

4. **Run the engine:**
   ```bash
   ./bin/chess_engine
   ```

### Build Options

The engine is configured with aggressive optimizations by default:
- `-O3` optimization level
- `-march=native` for CPU-specific optimizations
- Debug builds available with `-O0 -g -Wall -Wextra`

## Usage

### Basic Commands

| Command | Description | Example |
|---------|-------------|---------|
| `move <from><to>` | Make a move | `move e2e4` |
| `move <from><to><promo>` | Make a promotion move | `move e7e8q` |
| `go` | Let engine make a move | `go` |
| `depth <n>` | Set search depth (1-10) | `depth 6` |
| `time <ms>` | Set max search time | `time 3000` |
| `fen <string>` | Set position from FEN | `fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1` |
| `reset` | Reset to starting position | `reset` |
| `eval` | Show position evaluation | `eval` |
| `legal` | Show all legal moves | `legal` |
| `help` | Show help | `help` |
| `quit` | Exit the program | `quit` |

### Example Game Session

```
=== C++ Chess Engine ===
A highly optimized chess engine using bitboards and magic bitboards

Starting position:
  +---+---+---+---+---+---+---+---+
8 | r | n | b | q | k | b | n | r |
  +---+---+---+---+---+---+---+---+
7 | p | p | p | p | p | p | p | p |
  +---+---+---+---+---+---+---+---+
6 |   |   |   |   |   |   |   |   |
  +---+---+---+---+---+---+---+---+
5 |   |   |   |   |   |   |   |   |
  +---+---+---+---+---+---+---+---+
4 |   |   |   |   |   |   |   |   |
  +---+---+---+---+---+---+---+---+
3 |   |   |   |   |   |   |   |   |
  +---+---+---+---+---+---+---+---+
2 | P | P | P | P | P | P | P | P |
  +---+---+---+---+---+---+---+---+
1 | R | N | B | Q | K | B | N | R |
  +---+---+---+---+---+---+---+---+
    a   b   c   d   e   f   g   h

> move e2e4
Move played: e2e4

Engine is thinking...
Engine plays: e7e5 (depth: 4, score: 15, time: 245ms)
```

## Technical Details

### Bitboard Implementation

The engine uses bitboards for efficient board representation:
- Each piece type and color has its own bitboard
- Fast bitwise operations for move generation
- Magic bitboards for sliding pieces (bishops, rooks, queens)

### Search Algorithm

- **Minimax with Alpha-Beta Pruning**: Standard adversarial search
- **Iterative Deepening**: Progressive depth search
- **Quiescence Search**: Capture-only search to avoid horizon effect
- **Move Ordering**: Captures, promotions, and castling prioritized
- **Time Management**: Configurable time limits

### Evaluation Function

- **Material**: Standard piece values (pawn=100, knight=320, bishop=330, rook=500, queen=900)
- **Position**: Piece-square tables for positional scoring
- **Endgame**: Special evaluation for king tropism and endgame patterns

### Performance Optimizations

- **Magic Bitboards**: Pre-computed attack tables for sliding pieces
- **Bitwise Operations**: Fast bit manipulation using built-in functions
- **Move Ordering**: Intelligent move ordering for better pruning
- **Memory Efficiency**: Compact move representation (16 bits per move)
- **CPU Optimizations**: Compiler optimizations and CPU-specific instructions

## Architecture

```
src/
├── main.cpp          # Main application and UI
├── bitboard.h/cpp    # Bitboard utilities and lookup tables
├── board.h/cpp       # Board representation and game state
├── movegen.h/cpp     # Move generation using bitboards
├── eval.h/cpp        # Position evaluation
└── search.h/cpp      # Search algorithm (minimax + alpha-beta)
```

## Performance

The engine is designed for high performance:
- **Move Generation**: ~1-2 million moves/second on modern hardware
- **Search Speed**: ~100k-500k positions/second depending on position complexity
- **Memory Usage**: Minimal memory footprint (~1MB for lookup tables)

## Future Improvements

Potential enhancements for future versions:
- **Opening Book**: Pre-computed opening moves
- **Transposition Table**: Cache for repeated positions
- **Null Move Pruning**: Additional search pruning techniques
- **Late Move Reduction**: Advanced move ordering
- **Parallel Search**: Multi-threaded search
- **UCI Protocol**: Standard chess engine protocol support

## License

This project is open source and available under the MIT License.

## Acknowledgments

- Inspired by the "Programming a Chess Engine in C" YouTube tutorial series
- Uses standard chess evaluation techniques and algorithms
- Built with modern C++ best practices and optimizations # Chess
# Chess
