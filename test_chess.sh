#!/bin/sh

# Chess Engine Test Script (sh compatible)
# Tests specific scenarios using FEN notation

ENGINE_PATH="./build/bin/chess_engine"

echo "Chess Engine Test Suite"
echo "======================"

# Function to test a position
test_position() {
    test_name="$1"
    fen="$2"
    move="$3"
    expected="$4"
    
    echo "Testing: $test_name"
    echo "FEN: $fen"
    echo "Move: $move"
    echo "Expected: $expected"
    
    # Create a temporary input file
    cat > /tmp/chess_test.in << EOF
fen $fen
move $move
quit
EOF
    
    # Run the engine with the test input
    output=$(timeout 5s $ENGINE_PATH < /tmp/chess_test.in 2>/dev/null)
    
    # Check result
    if [ "$expected" = "valid" ]; then
        if echo "$output" | grep -q "Move played:"; then
            echo "PASS: Move accepted as expected"
            return 0
        else
            echo "FAIL: Move rejected but should be valid"
            return 1
        fi
    elif [ "$expected" = "invalid" ]; then
        if echo "$output" | grep -q "Invalid move:" || echo "$output" | grep -q "Move found in legal moves: NO"; then
            echo "PASS: Move rejected as expected"
            return 0
        else
            echo "FAIL: Move accepted but should be invalid"
            return 1
        fi
    elif [ "$expected" = "check" ]; then
        if echo "$output" | grep -q "CHECK!"; then
            echo "PASS: Check detected as expected"
            return 0
        else
            echo "FAIL: Check not detected"
            return 1
        fi
    fi
    
    return 1
}

# Initialize counters
passed=0
total=0

# Test 1: Basic castling
echo ""
if test_position "Basic Kingside Castling" \
    "rnbqkb1r/ppp2ppp/3p1n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1" \
    "e1g1" "valid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 2: Castling through check
echo ""
if test_position "Castling When King in Check" \
    "rn1qkbnr/ppp1pppp/8/1b1p4/3BP3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1" \
    "e1g1" "invalid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 3: Bishop moving through pieces (should fail)
echo ""
if test_position "Bishop Moving Through Pieces" \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" \
    "f1d3" "invalid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 4: Queen moving through pieces (should fail)
echo ""
if test_position "Queen Moving Through Pieces" \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" \
    "d1d3" "invalid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 5: Rook moving through pieces (should fail)
echo ""
if test_position "Rook Moving Through Pieces" \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" \
    "a1a3" "invalid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 6: En passant
echo ""
if test_position "En Passant" \
    "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3" \
    "e5d6" "valid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 7: Pawn promotion
echo ""
if test_position "Pawn Promotion" \
    "8/6P1/8/8/8/8/8/1K1k4 w - - 0 1" \
    "g7g8q" "valid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 8: Double pawn push
echo ""
if test_position "Double Pawn Push" \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" \
    "e2e4" "valid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 9: Queen check through pieces (should not be possible)
echo ""
if test_position "Queen Check Through Pieces" \
    "rnb1kbnr/pp1pppp1/7p/qp6/3PP3/2P2N2/PP3PPP/RNBQK2R w KQkq - 2 4" \
    "e1d2" "valid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Test 10: Bishop check through pieces (should not be possible)
echo ""
if test_position "Bishop Check Through Pieces" \
    "rnbqk2r/ppppp1pp/5pn1/8/1b1PP3/2P5/PP3PPP/RNBQKBNR w KQkq - 0 1" \
    "e1d2" "valid"; then
    passed=`expr $passed + 1`
fi
total=`expr $total + 1`

# Clean up
rm -f /tmp/chess_test.in

# Print results
echo ""
echo "=== Test Results ==="
echo "Tests passed: $passed/$total"
percentage=`expr $passed \* 100 / $total`
echo "Success rate: $percentage%"

if [ $passed -eq $total ]; then
    echo ""
    echo "All tests passed!"
    exit 0
else
    echo ""
    echo "Some tests failed!"
    exit 1
fi 