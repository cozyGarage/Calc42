#!/bin/bash

echo "======================================"
echo "CALC42 Extended Features Test Suite"
echo "======================================"
echo ""

# Test counter
PASS=0
FAIL=0

# Test function
test_expr() {
    local expr="$1"
    local expected="$2"
    local description="$3"
    
    result=$(./calc42-cli "$expr" 2>&1 | head -1)
    
    if [[ "$result" == "$expected" ]]; then
        echo "✓ $description"
        ((PASS++))
    else
        echo "✗ $description"
        echo "  Expected: $expected"
        echo "  Got:      $result"
        ((FAIL++))
    fi
}

echo "== Basic Arithmetic =="
test_expr "3 + 4 * 2" "11" "Operator precedence"
test_expr "(3 + 4) * 2" "14" "Parentheses"
test_expr "10 / 2" "5" "Division"
test_expr "10 % 3" "1" "Modulo"
echo ""

echo "== Number Bases =="
test_expr "0xFF" "255" "Hexadecimal"
test_expr "0b1010" "10" "Binary"
test_expr "0xFF + 1" "256" "Hex arithmetic"
echo ""

echo "== Bitwise Operations =="
test_expr "0xFF & 0x0F" "15" "Bitwise AND"
test_expr "0x0F | 0xF0" "255" "Bitwise OR"
test_expr "0xFF ^ 0xAA" "85" "Bitwise XOR"
test_expr "1 << 4" "16" "Left shift"
test_expr "16 >> 2" "4" "Right shift"
echo ""

echo "== Statistics Functions =="
test_expr "mean(10, 20, 30)" "20" "mean(10, 20, 30)"
test_expr "median(10, 20, 30)" "20" "median(10, 20, 30)"
test_expr "median(1, 2, 3, 4)" "2.5" "median(1, 2, 3, 4)"
test_expr "stddev(2, 4, 4, 4, 5, 5, 7, 9)" "2" "stddev(2, 4, ..., 9)"
echo ""

echo "== Discrete Math Functions =="
test_expr "gcd(12, 18)" "6" "GCD(12, 18)"
test_expr "lcm(12, 18)" "36" "LCM(12, 18)"
test_expr "mod(10, 3)" "1" "mod(10, 3)"
test_expr "is_prime(7)" "1" "is_prime(7)"
test_expr "is_prime(4)" "0" "is_prime(4)"
echo ""

echo "== Probability Functions =="
test_expr "ncr(10, 3)" "120" "nCr(10, 3)"
test_expr "npr(10, 3)" "720" "nPr(10, 3)"
test_expr "fact(5)" "120" "fact(5)"
echo ""

echo "== Unary Operators =="
test_expr "neg(5)" "-5" "neg(5)"
test_expr "not(0)" "1" "not(0)"
test_expr "not(5)" "0" "not(5)"
test_expr "bnot(0)" "-1" "bnot(0)"
echo ""

echo "== Logic Operators =="
test_expr "and(1, 1)" "1" "and(1, 1)"
test_expr "and(1, 0)" "0" "and(1, 0)"
test_expr "or(0, 0)" "0" "or(0, 0)"
test_expr "or(1, 0)" "1" "or(1, 0)"
test_expr "xor(1, 1)" "0" "xor(1, 1)"
test_expr "xor(1, 0)" "1" "xor(1, 0)"
echo ""

echo "== Set Operations =="
test_expr "set_union(1, 2, 2, 3)" "[1, 2, 3]" "set_union({1,2}, {2,3})"
test_expr "set_intersect(1, 2, 2, 3)" "[2]" "set_intersect({1,2}, {2,3})"
test_expr "set_diff(1, 2, 2, 3)" "[1]" "set_diff({1,2}, {2,3})"
echo ""

echo "== Linear Algebra =="
test_expr "vec_add(1, 2, 3, 4)" "[4, 6]" "vec_add([1,2], [3,4])"
test_expr "vec_dot(1, 2, 3, 4)" "11" "vec_dot([1,2], [3,4])"
test_expr "vec_mag(3, 4)" "5" "vec_mag([3,4])"
test_expr "vec_scale(2, 1, 2)" "[2, 4]" "vec_scale(2, [1,2])"
test_expr "mat_det(matrix(2, 2, 1, 2, 3, 4))" "-2" "mat_det(2x2)"
test_expr "mat_mul(matrix(2, 2, 1, 0, 0, 1), matrix(2, 2, 5, 6, 7, 8))" "[5, 6, 7, 8]" "mat_mul(I, A)"
echo ""

echo "== Advanced Stats & Prob =="
test_expr "binomial(10, 0.5, 5)" "0.24609375" "binomial(10, 0.5, 5)"
test_expr "geometric(0.5, 3)" "0.125" "geometric(0.5, 3)"
test_expr "zscore(20, 10, 20, 30)" "0" "zscore(20, {10,20,30})"
echo ""

echo "== Error Handling =="
# We check if the result contains "Error:" since that's what we output for ERR_*
test_expr "10 / 0" "Error: Division by zero" "Division by zero"
test_expr "1 << 65" "Error: Invalid shift count" "Invalid shift count"
test_expr "gcd(1)" "Error: gcd requires 2 arguments" "Wrong arg count"
echo ""

echo "======================================="
echo "Results: $PASS passed, $FAIL failed"
echo "======================================="

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi
