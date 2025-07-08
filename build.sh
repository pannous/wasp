#!/bin/bash
set -e

# WASP Build Script - Docker-based build automation
# Usage: ./build.sh [target] [options]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Default values
TARGET="wasp-hosted"
BUILD_TYPE="Release"
PROFILE="production"
DOCKER_CMD="docker compose"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

usage() {
    echo "Usage: $0 [TARGET] [OPTIONS]"
    echo ""
    echo "Targets:"
    echo "  wasp-hosted    Build hosted WASM version (default)"
    echo "  wasp-runtime   Build runtime-only version"
    echo "  wasp           Build native version"
    echo "  test           Run tests"
    echo "  clean          Clean build artifacts"
    echo ""
    echo "Options:"
    echo "  --debug        Build in debug mode"
    echo "  --dev          Use development environment"
    echo "  --native       Build natively (no Docker)"
    echo "  --help, -h     Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Build wasp-hosted in release mode"
    echo "  $0 --debug           # Build wasp-hosted in debug mode"
    echo "  $0 test              # Run tests"
    echo "  $0 wasp-runtime      # Build runtime-only version"
    echo "  $0 clean             # Clean build artifacts"
}

log() {
    echo -e "${BLUE}[BUILD]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
    exit 1
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if Docker is available
check_docker() {
    if ! command -v docker &> /dev/null; then
        error "Docker is not installed or not in PATH"
    fi
    
    if ! docker compose version &> /dev/null; then
        if command -v docker-compose &> /dev/null; then
            DOCKER_CMD="docker-compose"
        else
            error "Docker Compose is not available"
        fi
    fi
}

# Parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            wasp-hosted|wasp-runtime|wasp|test|clean)
                TARGET="$1"
                shift
                ;;
            --debug)
                BUILD_TYPE="Debug"
                PROFILE="development"
                shift
                ;;
            --dev)
                PROFILE="development"
                shift
                ;;
            --native)
                NATIVE_BUILD=true
                shift
                ;;
            --help|-h)
                usage
                exit 0
                ;;
            *)
                error "Unknown option: $1"
                ;;
        esac
    done
}

# Build natively without Docker
native_build() {
    log "Building natively without Docker"
    
    # Check for required tools
    if ! command -v cmake &> /dev/null; then
        error "CMake is not installed"
    fi
    
    if ! command -v clang &> /dev/null; then
        error "Clang is not installed"
    fi
    
    # Create build directory
    mkdir -p build
    cd build
    
    case $TARGET in
        wasp-hosted)
            log "Building WASM hosted version"
            cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DWASM=1 -DMY_WASM=1 ..
            make wasp-hosted
            ;;
        wasp-runtime)
            log "Building WASM runtime-only version"
            cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DWASM=1 -DRUNTIME_ONLY=1 ..
            make wasp-runtime
            ;;
        wasp)
            log "Building native version"
            cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..
            make wasp
            ;;
        test)
            log "Running tests"
            cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DWASM=1 -DMY_WASM=1 ..
            make wasp-hosted
            ctest --verbose
            ;;
        clean)
            log "Cleaning build artifacts"
            cd ..
            rm -rf build
            ./clean.sh 2>/dev/null || true
            ;;
        *)
            error "Unknown target: $TARGET"
            ;;
    esac
}

# Build using Docker
docker_build() {
    log "Building using Docker with profile: $PROFILE"
    
    case $TARGET in
        clean)
            log "Cleaning Docker build artifacts"
            $DOCKER_CMD down --volumes
            docker system prune -f
            rm -rf build
            ./clean.sh 2>/dev/null || true
            ;;
        test)
            log "Running tests in Docker"
            $DOCKER_CMD --profile test up --build wasp-test
            ;;
        wasp-runtime)
            log "Building runtime-only version in Docker"
            $DOCKER_CMD --profile runtime up --build wasp-runtime
            ;;
        *)
            log "Building $TARGET in Docker"
            if [[ "$PROFILE" == "development" ]]; then
                $DOCKER_CMD --profile development up --build wasp-dev
            else
                $DOCKER_CMD --profile production up --build wasp-build
            fi
            ;;
    esac
}

# Main execution
main() {
    log "WASP Build System"
    log "Target: $TARGET, Build Type: $BUILD_TYPE, Profile: $PROFILE"
    
    parse_args "$@"
    
    if [[ "$TARGET" == "clean" ]]; then
        if [[ "$NATIVE_BUILD" == "true" ]]; then
            native_build
        else
            check_docker
            docker_build
        fi
        success "Clean completed"
        exit 0
    fi
    
    if [[ "$NATIVE_BUILD" == "true" ]]; then
        native_build
    else
        check_docker
        docker_build
    fi
    
    success "Build completed successfully"
    
    # Show output files
    if [[ -f "build/$TARGET" ]] || [[ -f "build/${TARGET}.wasm" ]]; then
        log "Output files:"
        ls -la build/"$TARGET"* 2>/dev/null || true
        ls -la build/*.wasm 2>/dev/null || true
    fi
}

# Run main function with all arguments
main "$@"