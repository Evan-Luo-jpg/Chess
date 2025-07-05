#!/bin/sh

# Specific test for castling through check issue

ENGINE_PATH="./build/bin/chess_engine"

echo "Testing Castling Through Check Issue"
echo "===================================="

# Test the specific position where castling should be illegal
echo ""
echo "Test: Castling when king is in check"
echo "FEN: rnb1kbnr/pp1ppppp/8/qp6/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 2 4"
echo "Move: e1g1"
echo "Expected: INVALID (king is in check)"

# Create test input
cat > /tmp/castling_test.in << EOF
fen rnb1kbnr/pp1ppppp/8/qp6/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 2 4
move e1g1
quit
EOF

# Run the engine
output=$(timeout 5s $ENGINE_PATH < /tmp/castling_test.in 2>/dev/null)

echo ""
echo "Engine output:"
echo "$output"

# Check if castling was rejected
if echo "$output" | grep -q "Invalid move:" || echo "$output" | grep -q "Move found in legal moves: NO"; then
    echo ""
    echo "PASS: Castling correctly rejected when king is in check"
    rm -f /tmp/castling_test.in
    exit 0
else
    echo ""
    echo "FAIL: Castling was allowed when king is in check!"
    echo "This is the bug that needs to be fixed."
    rm -f /tmp/castling_test.in
    exit 1
fi 