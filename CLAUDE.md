# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Architecture

This is a Tauri desktop application that combines multiple technologies:

- **Frontend**: React 19 + TypeScript with Vite as the build tool
- **Backend**: Rust with Tauri framework
- **Native Integration**: C++ code compiled and linked via Rust's FFI (Foreign Function Interface)

### Key Architecture Components

1. **Frontend (src/)**: React application that communicates with the backend via Tauri's `invoke()` API
2. **Rust Backend (src-tauri/src/)**: Tauri application that serves as a bridge between frontend and C++ code
3. **C++ Integration (src-tauri/cpp/)**: Native C++ functions compiled as static library and called from Rust
4. **Build System**:
   - `build.rs` compiles C++ code using `cc` crate
   - Vite handles frontend development server
   - Tauri handles the overall application build and packaging

## Development Commands

### Frontend Development
```bash
# Start development server (frontend + Tauri)
pnpm tauri dev

# Frontend only (if needed)
pnpm dev
```

### Building
```bash
# Build frontend
pnpm build

# Build complete Tauri application
pnpm tauri build
```

### Package Management
```bash
# Install dependencies
pnpm install

# Tauri CLI commands
pnpm tauri [command]
```

## C++ Integration Workflow

The project uses a specific pattern for C++ integration:

1. **C++ Functions**: Defined in `src-tauri/cpp/hello.cpp` with `extern "C"` linkage
2. **Build Process**: `build.rs` compiles C++ to static library using `cc::Build`
3. **Rust FFI Wrapper**: `src-tauri/src/lib.rs` declares external C++ functions and creates safe Rust wrappers
4. **Tauri Commands**: Rust wrappers exposed as Tauri commands for frontend consumption
5. **Frontend Calls**: React components use `invoke()` to call backend commands

When modifying C++ code:
- Edit functions in `src-tauri/cpp/hello.cpp`
- The build system automatically detects changes and recompiles
- Add new function declarations in `src-tauri/src/lib.rs` with `extern "C"`
- Create safe Rust wrappers and expose as Tauri commands

## Project Structure Notes

- Frontend runs on `http://localhost:1420` during development
- Tauri window configured to 800x600 pixels
- C++ compiled to static library named `hello_cpp`
- All C++ strings use `static std::string` to ensure memory safety across FFI boundaries
- Project uses `pnpm` as package manager (configured in tauri.conf.json)