#!/bin/bash
################################################################################
# Build Script for Customizable Functions Feature Development
#
# This script configures, builds, and tests the customizable functions feature
# in Clang with optimized settings for faster iteration.
#
# Usage:
#   ./custom-functions-dev.sh [command] [options]
#
# Commands:
#   configure [debug|release|minimal]  - Configure CMake build
#   build [target]                     - Build clang (or specific target)
#   test [all|pattern]                 - Run tests (see test section below)
#   clean                              - Clean build directory
#   rebuild                            - Clean and rebuild
#   help                               - Show this help
#
# Test Commands:
#   test all              - Run ALL customizable function tests
#   test codegen          - Run CodeGen tests only
#   test sema             - Run Sema tests only
#   test parse            - Run Parser tests only
#   test [pattern]        - Run tests matching pattern
#
# Examples:
#   ./custom-functions-dev.sh configure debug
#   ./custom-functions-dev.sh build
#   ./custom-functions-dev.sh test all
#   ./custom-functions-dev.sh test codegen
#   ./custom-functions-dev.sh rebuild
################################################################################

set -e  # Exit on error

# Colors for output
RED=$'\033[0;31m'
GREEN=$'\033[0;32m'
YELLOW=$'\033[1;33m'
BLUE=$'\033[0;34m'
CYAN=$'\033[0;36m'
NC=$'\033[0m' # No Color

# Directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLVM_DIR="${SCRIPT_DIR}"
BUILD_DIR="${SCRIPT_DIR}/build-custom-functions"
INSTALL_DIR="${BUILD_DIR}/install"

# Build settings
DEFAULT_BUILD_TYPE="Debug"
NUM_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

################################################################################
# Helper Functions
################################################################################

print_header() {
    echo -e "${BLUE}======================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}======================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${CYAN}ℹ $1${NC}"
}

show_help() {
    cat << EOF
${BLUE}Customizable Functions Build Script${NC}

${GREEN}Usage:${NC}
  ./custom-functions-dev.sh [command] [options]

${GREEN}Commands:${NC}
  ${YELLOW}configure [mode]${NC}      Configure CMake build
                          Modes: debug (default), release, minimal
  ${YELLOW}build [target]${NC}        Build clang or specific target
                          Targets: clang, check-clang, check-clang-sema
  ${YELLOW}test [category]${NC}       Run tests by category (see below)
  ${YELLOW}clean${NC}                 Clean build directory
  ${YELLOW}rebuild${NC}               Clean and rebuild from scratch
  ${YELLOW}info${NC}                  Show build information
  ${YELLOW}help${NC}                  Show this help

${GREEN}Test Categories:${NC}
  ${YELLOW}test all${NC}              Run ALL customizable function tests
  ${YELLOW}test codegen${NC}          Run CodeGenCXX tests:
                          - customizable-functions-basic.cpp
                          - customizable-functions-void.cpp
                          - customizable-functions-constexpr.cpp
                          - customizable-functions-noexcept.cpp
                          - customizable-functions-template.cpp
                          - customizable-functions-overload.cpp
                          - customizable-functions-namespace.cpp
                          - customizable-functions-trailing-return.cpp
                          - customizable-functions-complex-args.cpp
                          - customizable-functions-declaration-only.cpp
                          - customizable-functions-extern-c.cpp
                          - customizable-functions-unaffected.cpp

  ${YELLOW}test sema${NC}             Run SemaCXX tests:
                          - customizable-functions-errors.cpp
                          - customizable-functions-identifier-only.cpp
                          - customizable-functions-external-linkage-only.cpp

  ${YELLOW}test sema-override${NC}    Run Sema ADL override tests:
                          - customizable-functions-sema-override.cpp

  ${YELLOW}test llvm${NC}             Run LLVM transform tests:
                          - basic-override.ll
                          - no-override.ll

  ${YELLOW}test parse${NC}            Run Parser tests (if any)

  ${YELLOW}test [pattern]${NC}        Run tests matching custom pattern

${GREEN}Examples:${NC}
  # Initial setup
  ./custom-functions-dev.sh configure debug
  ./custom-functions-dev.sh build

  # Run all our tests
  ./custom-functions-dev.sh test all

  # Run only codegen tests
  ./custom-functions-dev.sh test codegen

  # Run specific test
  ./custom-functions-dev.sh test template

  # Rebuild and test
  ./custom-functions-dev.sh rebuild
  ./custom-functions-dev.sh test all

${GREEN}Build Modes:${NC}
  ${YELLOW}debug${NC}     - Debug build with assertions (default)
              - Best for development and debugging
  ${YELLOW}release${NC}   - Optimized release build
              - Use for performance testing
  ${YELLOW}minimal${NC}   - Minimal debug build
              - Fastest iteration time

${GREEN}Environment Variables:${NC}
  ${YELLOW}BUILD_JOBS${NC}      Number of parallel build jobs (default: ${NUM_JOBS})
  ${YELLOW}BUILD_TYPE${NC}      Build type override (Debug/Release/RelWithDebInfo)
  ${YELLOW}CC${NC}              C compiler
  ${YELLOW}CXX${NC}             C++ compiler

EOF
}

################################################################################
# CMake Configuration
################################################################################

configure_build() {
    local build_mode="${1:-debug}"

    print_header "Configuring CMake Build (${build_mode} mode)"

    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"

    local cmake_build_type="${DEFAULT_BUILD_TYPE}"
    local extra_flags=""

    case "${build_mode}" in
        debug)
            cmake_build_type="Debug"
            print_info "Debug build: Assertions enabled"
            ;;
        release)
            cmake_build_type="Release"
            print_info "Release build: Optimizations enabled"
            ;;
        minimal)
            cmake_build_type="Debug"
            extra_flags="-DLLVM_TARGETS_TO_BUILD=X86"
            print_info "Minimal build: Only X86 target"
            ;;
        *)
            print_error "Unknown build mode: ${build_mode}"
            exit 1
            ;;
    esac

    [ -n "${BUILD_TYPE}" ] && cmake_build_type="${BUILD_TYPE}"

    print_info "Build directory: ${BUILD_DIR}"
    print_info "Build type: ${cmake_build_type}"
    print_info "Parallel jobs: ${NUM_JOBS}"

    cmake -G Ninja \
        -DCMAKE_BUILD_TYPE="${cmake_build_type}" \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
        -DLLVM_ENABLE_PROJECTS="clang" \
        -DLLVM_TARGETS_TO_BUILD="X86" \
        -DLLVM_INCLUDE_TESTS=ON \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_DOCS=OFF \
        -DLLVM_ENABLE_ASSERTIONS=ON \
        -DLLVM_ENABLE_WERROR=OFF \
        -DLLVM_OPTIMIZED_TABLEGEN=ON \
        -DLLVM_USE_SPLIT_DWARF=ON \
        -DCLANG_ENABLE_STATIC_ANALYZER=OFF \
        -DCLANG_ENABLE_ARCMT=OFF \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        ${extra_flags} \
        "${LLVM_DIR}/llvm"

    print_success "CMake configuration complete"

    if [ -f "${BUILD_DIR}/compile_commands.json" ]; then
        ln -sf "${BUILD_DIR}/compile_commands.json" "${LLVM_DIR}/compile_commands.json"
        print_success "Created compile_commands.json symlink"
    fi
}

################################################################################
# Build Functions
################################################################################

build_target() {
    local target="${1:-clang}"

    if [ ! -d "${BUILD_DIR}" ]; then
        print_error "Build directory not found. Run 'configure' first."
        exit 1
    fi

    print_header "Building ${target}"
    cd "${BUILD_DIR}"

    local jobs="${BUILD_JOBS:-${NUM_JOBS}}"
    print_info "Building with ${jobs} parallel jobs..."

    local start_time=$(date +%s)

    # When building clang, also build required test infrastructure
    local targets="${target}"
    if [ "${target}" = "clang" ]; then
        targets="clang opt llvm-config FileCheck"
        print_info "Also building test tools: opt, llvm-config, FileCheck"
    fi

    if ninja -j "${jobs}" ${targets}; then
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        print_success "Build complete in ${duration} seconds"

        if [ "${target}" = "clang" ]; then
            print_info "Clang binary: ${BUILD_DIR}/bin/clang"
            print_info "Test tools: llvm-config, FileCheck"
        fi
    else
        print_error "Build failed"
        exit 1
    fi
}

################################################################################
# Test Functions
################################################################################

run_tests() {
    local category="${1:-all}"

    if [ ! -d "${BUILD_DIR}" ]; then
        print_error "Build directory not found. Run 'configure' first."
        exit 1
    fi

    if [ ! -f "${BUILD_DIR}/bin/llvm-lit" ]; then
        print_error "llvm-lit not found. Build first."
        exit 1
    fi

    cd "${BUILD_DIR}"

    print_header "Running Customizable Functions Tests"

    case "${category}" in
        all)
            print_info "Running ALL customizable function tests"
            echo ""

            print_info "CodeGen tests..."
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-basic.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-void.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-constexpr.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-noexcept.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-template.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-overload.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-namespace.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-trailing-return.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-complex-args.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-declaration-only.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-extern-c.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-unaffected.cpp"

            echo ""
            print_info "Sema tests..."
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/clang/test/SemaCXX/customizable-functions-errors.cpp" \
                "${LLVM_DIR}/clang/test/SemaCXX/customizable-functions-identifier-only.cpp" \
                "${LLVM_DIR}/clang/test/SemaCXX/customizable-functions-external-linkage-only.cpp"

            echo ""
            print_info "Sema override tests..."
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-sema-override.cpp"

            echo ""
            print_info "LLVM transform tests..."
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/llvm/test/Transforms/CustomizableFunctions/"

            echo ""
            print_info "Integration tests..."
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/llvm/test/Other/customizable-functions-pipeline.ll" \
                "${LLVM_DIR}/clang/test/Driver/customizable-functions.cpp"
            ;;

        codegen)
            print_info "Running CodeGen tests only"
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-basic.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-void.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-constexpr.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-noexcept.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-template.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-overload.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-namespace.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-trailing-return.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-complex-args.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-declaration-only.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-extern-c.cpp" \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-unaffected.cpp"
            ;;

        sema)
            print_info "Running Sema tests only"
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/clang/test/SemaCXX/customizable-functions-errors.cpp" \
                "${LLVM_DIR}/clang/test/SemaCXX/customizable-functions-identifier-only.cpp" \
                "${LLVM_DIR}/clang/test/SemaCXX/customizable-functions-external-linkage-only.cpp"
            ;;

        sema-override|override)
            print_info "Running Sema override/ADL tests"
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-sema-override.cpp"
            ;;

        llvm)
            print_info "Running LLVM transform tests only"
            ./bin/llvm-lit -v \
                "${LLVM_DIR}/llvm/test/Transforms/CustomizableFunctions/"
            ;;

        parse|parser)
            print_info "Running Parser tests (if any exist)"
            ./bin/llvm-lit -v "${LLVM_DIR}/clang/test/Parser" --filter="customizable" || true
            ;;

        *)
            print_info "Running tests matching pattern: ${category}"
            ./bin/llvm-lit -v "${LLVM_DIR}/clang/test" --filter="customizable-functions.*${category}"
            ;;
    esac

    print_success "Test run complete"
}

################################################################################
# Utility Functions
################################################################################

clean_build() {
    print_header "Cleaning Build Directory"

    if [ -d "${BUILD_DIR}" ]; then
        print_warning "Removing ${BUILD_DIR}..."
        rm -rf "${BUILD_DIR}"
        print_success "Build directory cleaned"
    else
        print_info "Build directory does not exist"
    fi
}

rebuild_all() {
    print_header "Rebuilding from Scratch"
    clean_build
    configure_build "${1:-debug}"
    build_target "clang"
    print_success "Rebuild complete"
}

show_info() {
    print_header "Build Information"

    if [ -d "${BUILD_DIR}" ]; then
        echo -e "${GREEN}Build Directory:${NC} ${BUILD_DIR}"

        if [ -f "${BUILD_DIR}/CMakeCache.txt" ]; then
            local build_type=$(grep "CMAKE_BUILD_TYPE:" "${BUILD_DIR}/CMakeCache.txt" | cut -d'=' -f2)
            echo -e "${GREEN}Build Type:${NC} ${build_type}"
        fi

        if [ -f "${BUILD_DIR}/bin/clang" ]; then
            echo -e "${GREEN}Clang Binary:${NC} ${BUILD_DIR}/bin/clang"
            echo -e "${GREEN}Clang Version:${NC}"
            "${BUILD_DIR}/bin/clang" --version | head -1
        fi

        echo ""
        echo -e "${GREEN}Test Files:${NC}"
        echo -e "  ${CYAN}CodeGenCXX:${NC}"
        ls -1 "${LLVM_DIR}/clang/test/CodeGenCXX/customizable-functions-"*.cpp 2>/dev/null | sed 's|.*/||' | sed 's/^/    /' || echo "    (none found)"
        echo -e "  ${CYAN}SemaCXX:${NC}"
        ls -1 "${LLVM_DIR}/clang/test/SemaCXX/customizable-functions-"*.cpp 2>/dev/null | sed 's|.*/||' | sed 's/^/    /' || echo "    (none found)"
    else
        print_warning "Build not configured yet"
        echo -e "Run: ${YELLOW}./custom-functions-dev.sh configure${NC}"
    fi
}

################################################################################
# Main Script
################################################################################

main() {
    local command="${1:-help}"
    shift || true

    case "${command}" in
        configure|config)
            configure_build "$@"
            ;;
        build)
            build_target "$@"
            ;;
        test)
            run_tests "$@"
            ;;
        clean)
            clean_build
            ;;
        rebuild)
            rebuild_all "$@"
            ;;
        info)
            show_info
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            print_error "Unknown command: ${command}"
            echo ""
            show_help
            exit 1
            ;;
    esac
}

main "$@"
