# CALC42 Future Roadmap

While CALC42 1.0 is feature-complete and production-ready, there are several exciting directions for future growth. This document outlines the planned improvements and visionary features.

## Phase 1: Engine & Language Enhancements

- **Variables & State**: Support for persistent variables (e.g., `x = 10`, `ans + 5`).
- **User Functions**: Allow users to define their own functions (e.g., `f(x) = x^2 + 2x + 1`).
- **Unary Operator Fixes**: Enhanced parser support for unary minus in parenthetical expressions (e.g., `(-1)^2`).
- **Arbitrary Precision**: Integration of a multi-precision library (like GMP) for thousands of digits of accuracy.
- **Complex Numbers**: Native support for complex arithmetic (`a + bi`).

## Phase 2: Advanced Math Libraries

- **Calculus Suite**:
  - Numerical differentiation.
  - Numerical integration using Simpson's rule or Gaussian quadrature.
- **Linear Algebra 2.0**:
  - Support for matrices larger than 3x3.
  - Matrix inversion and LU decomposition.
  - Eigenvalues and Eigenvectors.
- **Financial Math**:
  - Time value of money (TVM) calculations.
  - Amortization schedules.
  - Investment ROI and NPV.

## Phase 3: GUI & Visualization

- **Function Plotting**: 2D and 3D graphing engine using GTK4 drawing primitives or Cairo.
- **Matrix View**: A dedicated interactive spreadsheet-like view for large matrices.
- **History Management**: Visual and searchable log of all previous calculations.
- **Themes & Customization**: Dynamic switching between glassmorphism, dark mode, and high-contrast accessibility themes.
- **Mobile Port**: Exploring GTK4's mobile capabilities for a Linux-based mobile app (Librem 5 / PinePhone).

## Phase 4: Developer Ecosystem

- **Scripting Support**: The ability to run `.c42` scripts containing blocks of logic.
- **API / Shared Library**: Packaging the calculation engine as a standalone C library (`libcalc42`) for other developers to use.
- **Fuzz Testing**: Continuous automated fuzzing to ensure the parser remains indestructible.
- **CI/CD**: GitHub Actions for automated testing and cross-platform binary builds.

---

_CALC42 is an ever-evolving tool. We welcome ideas and contributions from the community._
