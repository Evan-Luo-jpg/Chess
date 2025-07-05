#!/usr/bin/env python3
"""
Chess Engine Test Suite
Tests various chess scenarios using FEN notation
"""

import subprocess
import time
import sys

class ChessTester:
    def __init__(self, engine_path="./build/bin/chess_engine"):
        self.engine_path = engine_path
        self.process = None
        
    def start_engine(self):
        """Start the chess engine process"""
        try:
            self.process = subprocess.Popen(
                [self.engine_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
            # Wait for engine to start
            time.sleep(0.1)
            return True
        except Exception as e:
            print(f"Failed to start engine: {e}")
            return False
    
    def send_command(self, command):
        """Send a command to the engine and return response"""
        if not self.process:
            return None
        
        try:
            self.process.stdin.write(command + "\n")
            self.process.stdin.flush()
            
            # Read response
            response = ""
            time.sleep(0.1)  # Give engine time to respond
            
            # Read available output
            while True:
                try:
                    line = self.process.stdout.readline()
                    if not line:
                        break
                    response += line
                    if "Side to move:" in line or ">" in line:
                        break
                except:
                    break
            
            return response
        except Exception as e:
            print(f"Error sending command '{command}': {e}")
            return None
    
    def test_position(self, test_name, fen, moves_to_test, expected_results):
        """Test a specific position with given moves"""
        print(f"\n=== Testing: {test_name} ===")
        print(f"FEN: {fen}")
        
        # Set position
        response = self.send_command(f"fen {fen}")
        if not response:
            print("❌ Failed to set position")
            return False
        
        # Test each move
        all_passed = True
        for move, expected in moves_to_test:
            print(f"\nTesting move: {move}")
            response = self.send_command(f"move {move}")
            
            if expected == "valid":
                if "Move played:" in response:
                    print("✅ Move accepted (expected)")
                else:
                    print("❌ Move rejected (should be valid)")
                    all_passed = False
            elif expected == "invalid":
                if "Invalid move:" in response or "Move found in legal moves: NO" in response:
                    print("✅ Move rejected (expected)")
                else:
                    print("❌ Move accepted (should be invalid)")
                    all_passed = False
            elif expected == "check":
                if "CHECK!" in response:
                    print("✅ Check detected (expected)")
                else:
                    print("❌ Check not detected (should be check)")
                    all_passed = False
        
        return all_passed
    
    def run_all_tests(self):
        """Run all test scenarios"""
        if not self.start_engine():
            return False
        
        tests_passed = 0
        total_tests = 0
        
        # Test 1: Basic castling (should work)
        test1_passed = self.test_position(
            "Basic Kingside Castling",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("e1g1", "valid")],
            []
        )
        tests_passed += 1 if test1_passed else 0
        total_tests += 1
        
        # Test 2: Castling through check (should fail)
        test2_passed = self.test_position(
            "Castling Through Check",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("e2e4", "valid"), ("d7d5", "valid"), ("e4d5", "valid"), ("e1g1", "invalid")],
            []
        )
        tests_passed += 1 if test2_passed else 0
        total_tests += 1
        
        # Test 3: Castling when king is in check (should fail)
        test3_passed = self.test_position(
            "Castling When King in Check",
            "rnb1kbnr/pp1ppppp/8/qp6/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 2 4",
            [("e1g1", "invalid")],
            []
        )
        tests_passed += 1 if test3_passed else 0
        total_tests += 1
        
        # Test 4: Bishop moving through pieces (should fail)
        test4_passed = self.test_position(
            "Bishop Moving Through Pieces",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("f1d3", "invalid")],  # Bishop trying to move through pawn
            []
        )
        tests_passed += 1 if test4_passed else 0
        total_tests += 1
        
        # Test 5: Queen moving through pieces (should fail)
        test5_passed = self.test_position(
            "Queen Moving Through Pieces",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("d1d3", "invalid")],  # Queen trying to move through pawn
            []
        )
        tests_passed += 1 if test5_passed else 0
        total_tests += 1
        
        # Test 6: Rook moving through pieces (should fail)
        test6_passed = self.test_position(
            "Rook Moving Through Pieces",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("a1a3", "invalid")],  # Rook trying to move through pawn
            []
        )
        tests_passed += 1 if test6_passed else 0
        total_tests += 1
        
        # Test 7: En passant (should work)
        test7_passed = self.test_position(
            "En Passant",
            "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3",
            [("e5d6", "valid")],  # En passant capture
            []
        )
        tests_passed += 1 if test7_passed else 0
        total_tests += 1
        
        # Test 8: Pawn promotion (should work)
        test8_passed = self.test_position(
            "Pawn Promotion",
            "rnbqkbnr/ppppppPp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("g7g8q", "valid")],  # Pawn promotion to queen
            []
        )
        tests_passed += 1 if test8_passed else 0
        total_tests += 1
        
        # Test 9: Check detection
        test9_passed = self.test_position(
            "Check Detection",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("e2e4", "valid"), ("d7d5", "valid"), ("f1b5", "valid")],
            []
        )
        tests_passed += 1 if test9_passed else 0
        total_tests += 1
        
        # Test 10: Double pawn push
        test10_passed = self.test_position(
            "Double Pawn Push",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("e2e4", "valid")],  # Double pawn push
            []
        )
        tests_passed += 1 if test10_passed else 0
        total_tests += 1
        
        # Test 11: Illegal castling through attacked square
        test11_passed = self.test_position(
            "Castling Through Attacked Square",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("e2e4", "valid"), ("d7d5", "valid"), ("e4d5", "valid"), ("e1g1", "invalid")],
            []
        )
        tests_passed += 1 if test11_passed else 0
        total_tests += 1
        
        # Test 12: Queen checking through pieces (should not be possible)
        test12_passed = self.test_position(
            "Queen Check Through Pieces",
            "rnb1kbnr/pp1ppppp/8/qp6/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 2 4",
            [("a5d2", "invalid")],  # Queen trying to move through pawns
            []
        )
        tests_passed += 1 if test12_passed else 0
        total_tests += 1
        
        # Test 13: Bishop check through pieces (should not be possible)
        test13_passed = self.test_position(
            "Bishop Check Through Pieces",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("e2e4", "valid"), ("d7d5", "valid"), ("f1b5", "valid"), ("c6b5", "valid")],
            []
        )
        tests_passed += 1 if test13_passed else 0
        total_tests += 1
        
        # Test 14: Legal captures
        test14_passed = self.test_position(
            "Legal Captures",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("e2e4", "valid"), ("d7d5", "valid"), ("e4d5", "valid")],  # Pawn capture
            []
        )
        tests_passed += 1 if test14_passed else 0
        total_tests += 1
        
        # Test 15: Illegal king moves
        test15_passed = self.test_position(
            "Illegal King Moves",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            [("e1e3", "invalid")],  # King moving like a knight
            []
        )
        tests_passed += 1 if test15_passed else 0
        total_tests += 1
        
        print(f"\n=== Test Results ===")
        print(f"Tests passed: {tests_passed}/{total_tests}")
        print(f"Success rate: {tests_passed/total_tests*100:.1f}%")
        
        if self.process:
            self.process.terminate()
        
        return tests_passed == total_tests

def main():
    tester = ChessTester()
    success = tester.run_all_tests()
    
    if success:
        print("\n🎉 All tests passed!")
        sys.exit(0)
    else:
        print("\n❌ Some tests failed!")
        sys.exit(1)

if __name__ == "__main__":
    main() 