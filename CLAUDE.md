# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Tauri desktop application that implements a mathematical expression calculator using C++ stack-based algorithms. The project combines multiple technologies to create a cross-platform desktop application with native C++ performance.

- **Frontend**: React 19 + TypeScript with Vite as the build tool
- **Backend**: Rust with Tauri framework serving as FFI bridge
- **Native Integration**: C++ stack-based expression calculator compiled and linked via Rust's FFI

## Architecture Overview

The application follows a three-tier architecture:

1. **Frontend (src/)**: React calculator interface that sends expressions to backend via Tauri's `invoke()` API
2. **Rust FFI Layer (src-tauri/src/lib.rs)**: Safe Rust wrappers around C++ functions, exposed as Tauri commands
3. **C++ Calculator Engine (src-tauri/cpp/)**: Native stack-based expression evaluation with thread-safe global stacks

### Key Components

- **Stack Template** (`src-tauri/cpp/stack.hpp`): Generic stack implementation with dynamic resizing
- **Expression Calculator** (`src-tauri/cpp/hello.cpp`): Stack-based algorithm evaluating mathematical expressions with operator precedence
- **Tauri Commands**: `init_stack_command()` and `calculate_expression()` exposed to frontend
- **Thread Safety**: Global stacks protected by `std::mutex` for safe concurrent access

## Development Commands

```bash
# Install dependencies
pnpm install

# Start full development server (frontend + Tauri + C++ compilation)
pnpm tauri dev

# Frontend development only (for UI changes)
pnpm dev

# Build frontend TypeScript
pnpm build

# Build complete Tauri application with C++ integration
pnpm tauri build

# Other Tauri commands
pnpm tauri [command]
```

## C++ Integration Architecture

The project uses a specific C++ FFI integration pattern:

### Build System (`src-tauri/build.rs`)
- Compiles C++ code using `cc` crate into static library `hello_cpp`
- Automatically rebuilds when C++ files change via `cargo:rerun-if-changed`
- Handles C++ compilation flags and include paths

### C++ Layer (`src-tauri/cpp/`)
- **Functions**: Exported with `extern "C"` linkage for FFI compatibility
- **Thread Safety**: Global stacks protected by `std::mutex`
- **Error Handling**: Returns integer error codes mapped to Rust Result types
- **Memory Management**: Uses smart pointers and RAII for safety

### Rust FFI Bridge (`src-tauri/src/lib.rs`)
- **Declarations**: `extern "C"` blocks declaring C++ function signatures
- **Safe Wrappers**: Convert unsafe C++ calls to safe Rust Result types
- **Error Mapping**: Maps C++ error codes to descriptive Rust error messages
- **Tauri Commands**: Exposes safe wrappers as `#[tauri::command]` functions

### Frontend Integration (`src/App.tsx`)
- **Initialization**: Calls `init_stack_command` on app startup
- **Expression Evaluation**: Uses `calculate_expression` command for user input
- **Error Handling**: Displays backend error messages to users

## Expression Calculator Algorithm

The C++ calculator uses two-stack algorithm:
1. **Number Stack**: Stores integer operands
2. **Operator Stack**: Stores operators with precedence handling
3. **Algorithm**: Processes infix expressions with proper operator precedence and parentheses
4. **Supported Operations**: +, -, *, / with proper precedence and parentheses handling

## Error Codes

C++ backend returns specific error codes:
- `0`: Success
- `-1`: Stack not initialized
- `-2`: Empty input
- `-3`: Division by zero
- `-4`: Unknown operator
- `-5`: Invalid expression (insufficient numbers)
- `-6`: No result available

## Project Configuration

- **Development Server**: `http://localhost:1420`
- **Window Size**: 800x600 pixels (configurable in `tauri.conf.json`)
- **Package Manager**: pnpm (configured in tauri.conf.json)
- **C++ Standard**: Modern C++ with template metaprogramming
- **Thread Safety**: Mutex-protected global state