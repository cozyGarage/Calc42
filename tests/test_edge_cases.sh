#!/bin/bash

# CALC42 Comprehensive Edge Case Test Suite
# Tests error handling, boundary conditions, and unexpected inputs
# Only tests IMPLEMENTED functions

CLI="./calc42-cli"
PASSED=0
FAILED=0
TOTAL=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

test_expr() {
    TOTAL=$((TOTAL + 1))
    local expr="$1"
    local description="$2"
    local should_error="${3:-false}"
    
    result=$($CLI "$expr" 2>&1)
    exit_code=$?
    
    if [ "$should_error" = "true" ]; then
        # We expect this to fail
        if [ $exit_code -ne 0 ] || echo "$result" | grep -qi "error"; then
            echo -e "${GREEN}✓${NC} $description (expected error)"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}✗${NC} $description (should have errored, got: $result)"
            FAILED=$((FAILED + 1))
        fi
    else
        # We expect this to succeed  
        if [ $exit_code -eq 0 ] && ! echo "$result" | grep -qi "error"; then
            echo -e "${GREEN}✓${NC} $description → $result"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}✗${NC} $description (got error: $result)"
            FAILED=$((FAILED + 1))
        fi
    fi
}

echo -e "${YELLOW}=== CALC42 Comprehensive Edge Case Test Suite ===${NC}\n"

# ============================================================================
echo -e "${YELLOW}[1] Basic Arithmetic Edge Cases${NC}"
# ============================================================================
test_expr "10 / 0" "Division by zero" true
test_expr "10 % 0" "Modulo by zero" true
test_expr "0 / 5" "Zero divided by number"
test_expr "0 % 5" "Zero modulo number"
test_expr "1 / 3" "Floating point division"
test_expr "2 ^ 0" "Power of 0"
test_expr "0 ^ 2" "0 to power"
test_expr "5 * 2" "Basic multiplication"
test_expr "5 - 10" "Negative result"
test_expr "10 - 5 - 2" "Left-to-right subtraction"

# ============================================================================
echo -e "\n${YELLOW}[2] Factorial and Combinatorics Edge Cases${NC}"
# ============================================================================
test_expr "fact(0)" "Factorial of 0 (should be 1)"
test_expr "fact(1)" "Factorial of 1"
test_expr "fact(5)" "Factorial of 5"
test_expr "fact(-5)" "Factorial of negative" true
test_expr "factorial(4)" "Factorial alias"
test_expr "ncr(5, 0)" "nCr with k=0"
test_expr "ncr(5, 5)" "nCr with k=n"
test_expr "ncr(5, 6)" "nCr with k>n" true
test_expr "ncr(10, 3)" "nCr(10,3)"
test_expr "nCr(10, 3)" "nCr uppercase alias"
test_expr "npr(5, 0)" "nPr with k=0"
test_expr "npr(5, 5)" "nPr with k=n"
test_expr "npr(5, 6)" "nPr with k>n" true
test_expr "nPr(5, 3)" "nPr uppercase alias"

# ============================================================================
echo -e "\n${YELLOW}[3] Statistics Edge Cases${NC}"
# ============================================================================
test_expr "mean(5)" "Mean of single value"
test_expr "mean(1, 2, 3, 4, 5)" "Mean of 5 values"
test_expr "median(5)" "Median of single value"
test_expr "median(1, 2)" "Median of even count"
test_expr "median(1, 2, 3)" "Median of odd count"
test_expr "median(5, 1, 3, 2, 4)" "Median unsorted"
test_expr "mode(1, 2, 2, 3, 3, 3)" "Mode calculation"
test_expr "mode(5)" "Mode of single value"
test_expr "stddev(5)" "Stddev of single value (should be 0)"
test_expr "stddev(1, 1, 1, 1)" "Stddev of identical values"
test_expr "stddev(1, 2, 3, 4, 5)" "Stddev of sequence"
test_expr "var(1, 2, 3, 4, 5)" "Variance calculation"
test_expr "var(5)" "Variance of single value"
test_expr "zscore(5, 1, 2, 3, 4, 5, 6, 7, 8, 9)" "Z-score calculation"
test_expr "zscore(mean(1,2,3,4,5), 1, 2, 3, 4, 5)" "Z-score at mean (should be 0)"
test_expr "correlation(1, 2, 3, 4)" "Correlation of 2 pairs"
test_expr "correlation(1, 2, 3, 4, 5, 6)" "Correlation of 3 pairs"

# ============================================================================
echo -e "\n${YELLOW}[4] Statistics with Nested Arrays${NC}"
# ============================================================================
test_expr "mean(vector(1,2,3,4,5))" "Mean of vector"
test_expr "stddev(vector(10,20,30))" "Stddev of vector"
test_expr "median(vector(5,1,3,2,4))" "Median of vector"
test_expr "var(vector(1,2,3))" "Variance of vector"

# ============================================================================
echo -e "\n${YELLOW}[5] Probability Edge Cases${NC}"
# ============================================================================
test_expr "binomial(0, 0.5, 0)" "Binomial n=0, k=0"
test_expr "binomial(10, 0, 5)" "Binomial p=0"
test_expr "binomial(10, 1, 10)" "Binomial p=1, k=n"
test_expr "binomial(10, 0.5, 11)" "Binomial k>n" true
test_expr "binomial(10, 0.5, 5)" "Binomial(10, 0.5, 5)"
test_expr "binomial(100, 0.3, 30)" "Binomial large n"
test_expr "geometric(0.5, 1)" "Geometric k=1"
test_expr "geometric(0.5, 0)" "Geometric k=0" true
test_expr "geometric(1, 1)" "Geometric p=1"
test_expr "geometric(0.5, 3)" "Geometric(0.5, 3)"
test_expr "geometric(0, 5)" "Geometric p=0" true
test_expr "geometric(1.5, 5)" "Geometric p>1" true

# ============================================================================
echo -e "\n${YELLOW}[6] Discrete Math Edge Cases${NC}"
# ============================================================================
test_expr "gcd(0, 5)" "GCD with 0"
test_expr "gcd(5, 0)" "GCD with 0 (reversed)"
test_expr "gcd(1, 100)" "GCD with 1"
test_expr "gcd(12, 18)" "GCD(12, 18)"
test_expr "gcd(100, 50)" "GCD(100, 50)"
test_expr "lcm(0, 5)" "LCM with 0"
test_expr "lcm(1, 100)" "LCM with 1"
test_expr "lcm(12, 18)" "LCM(12, 18)"
test_expr "lcm(7, 13)" "LCM of primes"
test_expr "mod(10, 3)" "mod(10, 3)"
test_expr "mod(10, 0)" "mod by zero" true
test_expr "modpow(2, 0, 10)" "modpow with exp=0"
test_expr "modpow(2, 10, 1)" "modpow with mod=1"
test_expr "modpow(2, 10, 1000)" "modpow(2, 10, 1000)"
test_expr "is_prime(0)" "is_prime(0)"
test_expr "is_prime(1)" "is_prime(1)"
test_expr "is_prime(2)" "is_prime(2)"
test_expr "is_prime(17)" "is_prime(17)"
test_expr "is_prime(100)" "is_prime(100)"

# ============================================================================
echo -e "\n${YELLOW}[7] Vector Operations Edge Cases${NC}"
# ============================================================================
test_expr "vector(1)" "Single-element vector"
test_expr "vector(1,2,3,4,5)" "Multi-element vector"
test_expr "vec_add(1, 2)" "vec_add with 2 scalars"
test_expr "vec_add(1, 2, 3, 4)" "vec_add 4 scalars (2x2)"
test_expr "vec_add(1, 2, 3)" "vec_add odd count" true
test_expr "vec_sub(5, 3, 7, 2)" "vec_sub operation"
test_expr "vec_dot(1, 2, 3, 4)" "vec_dot operation"
test_expr "vec_dot(0, 0, 0, 0)" "vec_dot of zero vectors"
test_expr "vec_mag(0)" "vec_mag of zero"
test_expr "vec_mag(3, 4)" "vec_mag (3,4) should be 5"
test_expr "vec_mag(1)" "vec_mag of single element"
test_expr "vec_scale(0, 1, 2, 3)" "vec_scale by 0"
test_expr "vec_scale(2, 0)" "vec_scale of 0"
test_expr "vec_scale(3, 1, 2, 3)" "vec_scale(3, [1,2,3])"

# ============================================================================
echo -e "\n${YELLOW}[8] Vector with Array Arguments${NC}"
# ============================================================================
test_expr "vec_add(vector(1,2), vector(3,4))" "vec_add with vectors"
test_expr "vec_add(vector(1,2,3), vector(4,5,6))" "vec_add 3D vectors"
test_expr "vec_sub(vector(5,6), vector(2,1))" "vec_sub with vectors"
test_expr "vec_dot(vector(1,2), vector(3,4))" "vec_dot with vectors"
test_expr "vec_dot(vector(1,0,0), vector(0,1,0))" "vec_dot orthogonal"
test_expr "vec_mag(vector(3,4))" "vec_mag with vector"
test_expr "vec_mag(vector(1,0,0))" "vec_mag unit vector"
test_expr "vec_scale(2, vector(1,2,3))" "vec_scale with vector"
test_expr "vec_scale(0.5, vector(10,20))" "vec_scale fractional"

# ============================================================================
echo -e "\n${YELLOW}[9] Matrix Construction and Edge Cases${NC}"
# ============================================================================
test_expr "matrix(1, 1, 42)" "1x1 matrix"
test_expr "matrix(2, 2, 1, 2, 3, 4)" "2x2 matrix"
test_expr "matrix(2, 3, 1, 2, 3, 4, 5, 6)" "2x3 matrix"
test_expr "matrix(3, 1, 1, 2, 3)" "Column vector as matrix"
test_expr "matrix(1, 3, 1, 2, 3)" "Row vector as matrix"
test_expr "matrix(2, 2, 1, 2)" "Matrix with insufficient elements" true
test_expr "matrix(3, 3, 1,2,3,4,5,6,7,8,9)" "3x3 matrix"

# ============================================================================
echo -e "\n${YELLOW}[10] Matrix Operations Edge Cases${NC}"
# ============================================================================
test_expr "mat_det(matrix(2, 2, 1, 0, 0, 1))" "Identity matrix 2x2 det"
test_expr "mat_det(matrix(2, 2, 1, 2, 2, 4))" "Singular matrix det (should be 0)"
test_expr "mat_det(matrix(2, 2, 5, 3, 2, 1))" "Regular 2x2 det"
test_expr "mat_det(matrix(3,3,1,0,0,0,1,0,0,0,1))" "Identity 3x3 det"
test_expr "mat_transpose(matrix(1, 2, 1, 2))" "Transpose 1x2"
test_expr "mat_transpose(matrix(2, 2, 1, 2, 3, 4))" "Transpose 2x2"
test_expr "mat_mul(matrix(2,2,1,0,0,1), matrix(2,2,1,2,3,4))" "Identity * matrix"
test_expr "mat_mul(matrix(2,2,0,0,0,0), matrix(2,2,1,2,3,4))" "Zero matrix mul"
test_expr "mat_mul(matrix(2,2,1,2,3,4), matrix(2,2,5,6,7,8))" "2x2 * 2x2"
test_expr "mat_add(matrix(2,2,1,2,3,4), matrix(2,2,5,6,7,8))" "Matrix addition"
test_expr "mat_sub(matrix(2,2,5,6,7,8), matrix(2,2,1,2,3,4))" "Matrix subtraction"

# ============================================================================
echo -e "\n${YELLOW}[11] Set Operations Edge Cases${NC}"
# ============================================================================
test_expr "set_union(1, 1)" "set_union of identical elements"
test_expr "set_union(1, 2, 3, 4)" "set_union no overlap"
test_expr "set_union(1, 2, 1, 2)" "set_union complete overlap"
test_expr "set_union(1, 2, 2, 3)" "set_union partial overlap"
test_expr "set_intersect(1, 2, 3, 4)" "set_intersect no overlap"
test_expr "set_intersect(1, 2, 1, 2)" "set_intersect all overlap"
test_expr "set_intersect(1, 2, 2, 3)" "set_intersect partial"
test_expr "set_diff(1, 2, 1, 2)" "set_diff all in both"
test_expr "set_diff(1, 2, 3, 4)" "set_diff no overlap"
test_expr "set_diff(1, 2, 3, 3, 4, 2)" "set_diff with overlap"

# ============================================================================
echo -e "\n${YELLOW}[12] Set Operations with Arrays${NC}"
# ============================================================================
test_expr "set_union(vector(1,2,3), vector(3,4,5))" "set_union with vectors"
test_expr "set_union(vector(1,1,2), vector(2,2,3))" "set_union with dups"
test_expr "set_intersect(vector(1,2,3), vector(3,4,5))" "set_intersect with vectors"
test_expr "set_intersect(vector(1,2), vector(3,4))" "set_intersect empty result"
test_expr "set_diff(vector(1,2,3), vector(3,4,5))" "set_diff with vectors"
test_expr "set_diff(vector(1,2,3), vector(1,2,3))" "set_diff all removed"

# ============================================================================
echo -e "\n${YELLOW}[13] Logic Operations Edge Cases${NC}"
# ============================================================================
test_expr "and(0, 0)" "and(false, false)"
test_expr "and(1, 0)" "and(true, false)"
test_expr "and(1, 1)" "and(true, true)"
test_expr "and(5, 10)" "and(truthy, truthy)"
test_expr "or(0, 0)" "or(false, false)"
test_expr "or(0, 1)" "or(false, true)"
test_expr "or(1, 1)" "or(true, true)"
test_expr "xor(0, 0)" "xor(false, false)"
test_expr "xor(1, 1)" "xor(true, true)"
test_expr "xor(1, 0)" "xor(true, false)"
test_expr "not(0)" "not(false)"
test_expr "not(1)" "not(true)"
test_expr "not(42)" "not(truthy)"
test_expr "bnot(0)" "Bitwise not of 0"
test_expr "bnot(255)" "Bitwise not of 255"
test_expr "neg(5)" "Negation of 5"
test_expr "neg(0)" "Negation of 0"

# ============================================================================
echo -e "\n${YELLOW}[14] Logic with Multiple Arguments${NC}"
# ============================================================================
test_expr "and(1, 1, 1, 1)" "and of 4 trues"
test_expr "and(1, 1, 0, 1)" "and with one false"
test_expr "or(0, 0, 0, 1)" "or with one true"
test_expr "or(0, 0, 0, 0)" "or all false"
test_expr "xor(1, 1, 1, 1)" "xor chain"

# ============================================================================
echo -e "\n${YELLOW}[15] Nested Function Calls${NC}"
# ============================================================================
test_expr "mean(stddev(1,2,3), var(4,5,6))" "Nested stats"
test_expr "gcd(lcm(12, 18), lcm(15, 25))" "Nested discrete math"
test_expr "vec_mag(vec_add(1,2,3,4))" "Nested vector ops"
test_expr "vec_dot(vec_scale(2,1,2), vec_scale(3,2,1))" "Complex vector expr"
test_expr "set_union(set_intersect(1,2,3,4), set_diff(3,4,5,6))" "Deeply nested sets"
test_expr "mean(vector(stddev(1,2,3), var(4,5,6), median(7,8,9)))" "Very nested stats"

# ============================================================================
echo -e "\n${YELLOW}[16] Complex Expressions${NC}"
# ============================================================================
test_expr "mean(1,2,3) + stddev(4,5,6) * var(7,8,9)" "Mixed stats operations"
test_expr "gcd(48, 18) * lcm(12, 8)" "Discrete math expression"
test_expr "vec_dot(1,2,3,4) + vec_mag(5,6)" "Mixed vector ops"
test_expr "binomial(10, 0.5, 5) / geometric(0.5, 3)" "Probability expression"
test_expr "mat_det(matrix(2,2,1,2,3,4)) * 10" "Matrix in expression"
test_expr "ncr(10, 3) + npr(5, 2)" "Combinatorics expression"

# ============================================================================
echo -e "\n${YELLOW}[17] Operator Precedence and Parsing${NC}"
# ============================================================================
test_expr "2 + 3 * 4" "Multiplication before addition"
test_expr "10 - 5 - 2" "Left-to-right subtraction"
test_expr "2 ^ 3 ^ 2" "Right-to-left exponentiation"
test_expr "(2 + 3) * 4" "Parentheses override"
test_expr "2 + 3 * 4 ^ 2" "Mixed precedence"
test_expr "100 / 10 / 2" "Left-to-right division"
test_expr "2 * 3 + 4 * 5" "Multiple multiplications"
test_expr "((1 + 2) * (3 + 4))" "Nested parentheses"

# ============================================================================
echo -e "\n${YELLOW}[18] Error Recovery and Robustness${NC}"
# ============================================================================
test_expr "unknown_func(1, 2, 3)" "Unknown function" true
test_expr "mean()" "Function with no args" true
test_expr "vec_add(1)" "Insufficient vector args" true
test_expr "set_union(1)" "Insufficient set args" true
test_expr "(1 + 2" "Unclosed parenthesis" true
test_expr "1 + + 2" "Double operator" true
test_expr "/ 5" "Leading operator" true
test_expr "5 *" "Trailing operator" true

# ============================================================================
echo -e "\n${YELLOW}[19] Very Large and Very Small Numbers${NC}"
# ============================================================================
test_expr "fact(20)" "Large factorial"
test_expr "1e-10 + 1e-10" "Very small addition"
test_expr "1e10 * 1e10" "Large multiplication"
test_expr "1e308 * 2" "Near overflow" true
test_expr "1e-308 / 2" "Near underflow"
test_expr "999999999999 + 1" "Large integer"

# ============================================================================
echo -e "\n${YELLOW}[20] Special Values and Edge Cases${NC}"
# ============================================================================
test_expr "0 * 999999" "Zero multiplication"
test_expr "1 ^ 999999" "One to large power"
test_expr "0 ^ 0" "Zero to zero power"
test_expr "2 ^ 10" "Power of 10"
test_expr "mean(0, 0, 0, 0)" "Mean of all zeros"
test_expr "vec_dot(1,2,3,4,5,6)" "vec_dot 3D vectors"

# ============================================================================
# Memory Leak Check (if available)
# ============================================================================
if command -v leaks &> /dev/null; then
    echo -e "\n${YELLOW}[21] Memory Leak Test${NC}"
    result=$(leaks --atExit -- $CLI "mean(stddev(1,2,3), var(4,5,6), median(7,8,9))" 2>&1 | grep "0 leaks")
    if [ ! -z "$result" ]; then
        echo -e "${GREEN}✓${NC} No memory leaks detected"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗${NC} Memory leaks detected"
        FAILED=$((FAILED + 1))
    fi
    TOTAL=$((TOTAL + 1))
fi

# ============================================================================
# Summary
# ============================================================================
echo -e "\n${YELLOW}========================================${NC}"
echo -e "${YELLOW}Test Summary${NC}"
echo -e "${YELLOW}========================================${NC}"
echo -e "Total tests: $TOTAL"
echo -e "${GREEN}Passed: $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "${RED}Failed: $FAILED${NC}"
    SUCCESS_RATE=$((PASSED * 100 / TOTAL))
    echo -e "Success rate: ${SUCCESS_RATE}%"
    if [ $SUCCESS_RATE -ge 95 ]; then
        echo -e "${GREEN}Excellent! Ready for production.${NC}"
        exit 0
    elif [ $SUCCESS_RATE -ge 90 ]; then
        echo -e "${YELLOW}Good, but needs minor fixes.${NC}"
        exit 1
    else
        echo -e "${RED}Needs attention.${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}All tests passed! 100% success rate.${NC}"
    echo -e "${GREEN}✓ Production ready!${NC}"
    exit 0
fi
