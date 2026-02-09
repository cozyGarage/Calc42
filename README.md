# CALC42 - Multi-Mode Calculator

A robust, secure, and feature-rich calculator suite built in pure C with GTK4 GUI and CLI interfaces.

## Features

### Core Capabilities

- **Expression Engine**: Full expression parsing with operator precedence using shunting-yard algorithm
- **Multiple Number Bases**: Binary (`0b1010`), Octal (`0o12`), Decimal (`10`), Hexadecimal (`0xFF`)
- **CLI Calculator**: Interactive REPL and one-shot evaluation modes
- **Programmer Operations**: Bitwise AND, OR, XOR, NOT, left/right shifts
- **Memory Safe**: Zero memory leaks (valgrind verified), comprehensive bounds checking
- **Structured Logging**: JSON logging for all operations, errors, and mode switches

### Calculator Modes

- **Standard**: Basic arithmetic with full operator precedence and unary ops (`neg`, `not`)
- **Programmer**: Bitwise operations, base conversion, shifts, bit masks
- **Statistics**: Mean, median, mode, variance, stddev, z-score, correlation
- **Probability**: Combinations, permutations, factorial, binomial & geometric distributions
- **Discrete Math**: GCD, LCM, modular arithmetic, primality, set operations, logic ops
- **Linear Algebra**: Vectors and Matrices (add, sub, mul, det, transpose).

## Installation

### Prerequisites

```bash
# macOS
brew install gtk4 pkg-config

# Ubuntu/Debian
sudo apt-get install libgtk-4-dev pkg-config

# Fedora
sudo dnf install gtk4-devel pkg-config
```

### Build

```bash
# Clone the repository
cd Calc42

# Build everything (CLI + GUI)
make full

# Build just the CLI
make cli

# Run automated test suite
make test
./tests/test_extended.sh
```

## Usage

### One-Shot Mode

```bash
# Basic arithmetic
./calc42-cli "3 + 4 * 2"
# Output: 11

# Parentheses
./calc42-cli "(3 + 4) * 2"
# Output: 14

# Division
./calc42-cli "100 / 4"
# Output: 25

# Modulo
./calc42-cli "10 % 3"
# Output: 1

# Hexadecimal
./calc42-cli "0xFF + 1"
# Output: 256

# Binary
./calc42-cli "0b1010 + 0b0101"
# Output: 15

# Bitwise operations
./calc42-cli "0xFF & 0x0F"
# Output: 15

# Function calls - Discrete Math
./calc42-cli "gcd(12, 18)"
# Output: 6

# Function calls - Probability
./calc42-cli "ncr(10, 3)"
# Output: 120

# Linear Algebra (Vectors)
./calc42-cli "vec_add(1, 2, 3, 4)"
# Output: [4, 6]

# Linear Algebra (Matrices)
./calc42-cli "mat_det(matrix(2, 2, 1, 2, 3, 4))"
# Output: -2
```

### Interactive Mode (REPL)

```bash
./calc42-cli

[standard] > 3 + 4 * 2
= 11

[standard] > (3 + 4) * 2
= 14

[standard] > :mode programmer
Switched to programmer mode

[programmer] > :base 16
Base set to 16

[programmer] > 0xFF & 0x0F
= 0xF

[programmer] > 1 << 4
= 0x10

[programmer] > :help
# Shows help message

[programmer] > :quit
Goodbye!
```

### REPL Commands

- `:help` or `:h` - Display help message
- `:mode <mode>` - Switch calculator mode (standard, programmer, statistics, etc.)
- `:base <n>` - Set number base for programmer mode (2, 8, 10, 16)
- `:quit` or `:q` - Exit the calculator

## Architecture

### System Overview

```
┌─────────────────┐
│  GUI (GTK4)     │
│  - Buttons      │
│  - Panels       │
│  - History      │
└────────┬────────┘
         │
         │ expressions / commands
         ▼
┌─────────────────┐      ┌──────────────────┐
│  CLI            │      │  Logging Layer   │
│  - REPL         │─────▶│  (JSON, stdout)  │
│  - Scripts      │      │                  │
└────────┬────────┘      └──────────────────┘
         │
         │
         ▼
┌──────────────────────────────────┐
│   Expression Engine Core         │
│  ┌─────────────────────────────┐ │
│  │  Tokenizer                  │ │
│  │  Parser                     │ │
│  │  Evaluator                  │ │
│  │  Error handling             │ │
│  └─────────────────────────────┘ │
└────────┬─────────────────────────┘
         │
         ▼
┌────────────────┬──────────────┬─────────────────┐
│ Programmer Ops │  Statistics  │ Linear Algebra  │
│ - bitwise      │  - mean      │ - vectors       │
│ - shifts       │  - variance  │ - dot, mul      │
└────────────────┴──────────────┴─────────────────┘
```

### Computation Engine Design

The engine is **pure and stateless**, with no global variables. All state is passed explicitly through context structures.

**Components**:

1. **Tokenizer** (`src/engine/tokenizer.c`)
   - Breaks input into tokens (numbers, operators, functions, brackets)
   - Supports multiple number formats (decimal, hex, binary, octal)
   - Bounded token length prevents buffer overflows

2. **Parser** (`src/engine/parser.c`)
   - Converts infix expressions to Abstract Syntax Tree (AST)
   - Uses shunting-yard algorithm for operator precedence
   - Expression depth limit prevents stack overflow
   - Returns structured AST for evaluation

3. **Evaluator** (`src/engine/engine.c`)
   - Recursively evaluates AST nodes
   - Type-safe value system (double, int64, vectors, matrices)
   - Mode-specific operation dispatch
   - Stack overflow protection, division by zero checks

### Parser Design

The parser implements the **shunting-yard algorithm** in two phases:

1. **Infix to Postfix Conversion**: Uses operator precedence to convert expressions
2. **AST Construction**: Builds tree from postfix notation using a stack

**Operator Precedence** (highest to lowest):

- Bitwise: `&` (4), `^` (5), `|` (6)
- Multiplicative: `*`, `/`, `%` (3)
- Additive: `+`, `-` (2)
- Shift: `<<`, `>>` (1)

### Mode System Design

Calculator modes are implemented through a **context-based dispatch system**:

```c
typedef struct {
    calc_mode_t mode;
    int base;  // For programmer mode
} engine_context_t;
```

Operations check the current mode and execute mode-specific logic.

### Error Handling Strategy

**Layered Error Handling**:

1. **Error Codes**: Enumerated error types (syntax, domain, overflow, etc.)
2. **Error Context**: Position information for precise error reporting
3. **Graceful Degradation**: Never crash, always return an error code
4. **User-Friendly Messages**: Clear, actionable error messages

**Error Propagation**:

- Tokenizer errors → Parser
- Parser errors → Evaluator
- Evaluator errors → CLI/GUI

All allocations are checked, all indices are bounds-checked.

### Memory Safety Strategy

**Principles**:

1. **No Global State**: All data structures are stack or heap allocated with clear ownership
2. **Checked Allocations**: Every malloc/calloc return value is validated
3. **Bounded Operations**: Token lengths, expression depth, array sizes all have limits
4. **Safe Wrappers**: `safe_malloc`, `safe_realloc`, `safe_free` for consistent error handling
5. **RAII-like Pattern**: Clear allocation/deallocation pairs, cleanup on error paths
6. **Valgrind Clean**: Zero leaks, zero invalid reads/writes

**Dynamic Arrays**: Custom implementation with:

- Capacity tracking to prevent overflows
- Safe reallocation with overflow checks
- Automatic doubling growth strategy

## Logging

The system uses **JSON structured logging** for easy parsing and analysis:

```json
{"timestamp":"2026-02-02T01:01:52","type":"expression","expression":"3 + 4 * 2","result":"11"}
{"timestamp":"2026-02-02T01:01:53","type":"mode_switch","from":"standard","to":"programmer"}
{"timestamp":"2026-02-02T01:01:54","type":"error","error":"Division by zero","expression":"10 / 0"}
```

Logs are written to both `calc42.log` and stdout.

## Testing

### Automated Tests

```bash
# Run test suite
make test

# Memory leak detection
make valgrind

# Example output:
# ✓ Basic arithmetic
# ✓ Parentheses
# ✓ Division
# ✓ Modulo
```

### Manual Testing

**Test Cases**:

1. **Arithmetic**: `3 + 4 * 2` = 11
2. **Precedence**: `2 + 3 * 4` = 14
3. **Parentheses**: `(2 + 3) * 4` = 20
4. **Division**: `10 / 2` = 5
5. **Modulo**: `10 % 3` = 1
6. **Hex**: `0xFF` = 255
7. **Binary**: `0b1010` = 10
8. **Bitwise AND**: `0xFF & 0x0F` = 15 (when in programmer mode)
9. **Left Shift**: `1 << 4` = 16 (when in programmer mode)
10. **Error Handling**: `10 / 0` → "Division by zero"

### Fuzzing (Optional)

```bash
# Generate random inputs to test robustness
for i in {1..1000}; do
    echo $(head /dev/urandom | tr -dc 'A-Za-z0-9+\-*/()' | head -c 20) | ./calc42-cli
done
```

## Project Structure

```
Calc42/
├── include/
│   ├── common/
│   │   ├── error.h        # Error handling
│   │   ├── memory.h       # Memory management
│   │   └── logger.h       # Logging system
│   ├── engine/
│   │   ├── tokenizer.h    # Tokenization
│   │   ├── parser.h       # Parsing & AST
│   │   └── engine.h       # Main engine
│   ├── cli/
│   │   └── cli.h          # CLI interface (future)
│   └── gui/
│       └── gui.h          # GUI interface (future)
├── src/
│   ├── common/            # Common utilities
│   ├── engine/            # Core computation engine
│   ├── cli/               # CLI calculator
│   └── gui/               # GTK4 GUI (future)
├── tests/                 # Test suite (future)
├── Makefile              # Build system
└── README.md             # This file
```

## Development

### Compiler Flags

Security-focused compilation:

```makefile
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -O2 -g
CFLAGS += -fstack-protector-strong -D_FORTIFY_SOURCE=2
```

### Code Style

- **Standard**: C11
- **Naming**: `snake_case` for functions, `UPPER_CASE` for macros
- **Documentation**: Comprehensive comments in headers
- **Error Handling**: Always check return values
- **Memory**: Every malloc has a corresponding free

## 🗺️ Future Roadmap

CALC42 1.0 is a solid foundation, but there's more to come. See our [ROADMAP.md](ROADMAP.md) for planned features like plotting and complex numbers.

## 🧪 Testing & Quality

We maintain 100% pass rate across 240+ automated tests. Detailed reports can be found in [TESTING.md](TESTING.md).

## Resources

- [Shunting-yard Algorithm](https://en.wikipedia.org/wiki/Shunting-yard_algorithm)
- [GTK4 Documentation](https://docs.gtk.org/gtk4/)

## AI Usage Disclosure

This project was developed with assistance from AI (Google Antigravity/Gemini) as part of the learning process. The AI helped with:

- Architecture design and code structure
- Implementation of algorithms (shunting-yard, tokenizer)
- Memory safety patterns and error handling strategies
- Documentation and code organization

All code has been reviewed, understood, and customized to meet project requirements and coding standards.

---

**Status**: CALC42 1.0 is stable and feature-complete.
