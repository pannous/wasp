# Multi-stage build for WASP WebAssembly Programming Language
FROM ubuntu:22.04 AS base

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    clang \
    llvm \
    git \
    curl \
    wget \
    python3 \
    python3-pip \
    libreadline-dev \
    libcurl4-openssl-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install WASI SDK for WebAssembly compilation
FROM base AS wasi-sdk
WORKDIR /opt/wasm
RUN wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-25/wasi-sdk-25.0-linux.tar.gz \
    && tar -xzf wasi-sdk-25.0-linux.tar.gz \
    && mv wasi-sdk-25.0 wasi-sdk \
    && rm wasi-sdk-25.0-linux.tar.gz

# Install WasmEdge runtime for WASM execution
FROM wasi-sdk AS wasmedge
RUN curl -sSf https://raw.githubusercontent.com/WasmEdge/WasmEdge/master/utils/install.sh | bash -s -- -p /opt/wasm/WasmEdge

# Build environment with all dependencies
FROM wasmedge AS build-env

# Set environment variables
ENV PATH="/opt/wasm/wasi-sdk/bin:/opt/wasm/WasmEdge/bin:${PATH}"
ENV WASI_SDK_PATH="/opt/wasm/wasi-sdk"
ENV CMAKE_TOOLCHAIN_FILE="/workspace/wasm.toolchain.cmake"

# Create workspace
WORKDIR /workspace

# Copy build configuration files
COPY CMakeLists.txt wasm.toolchain.cmake ./
COPY clean.sh ./

# Production image with minimal dependencies for running builds
FROM build-env AS production

# Copy source code
COPY source/ ./source/
COPY Frameworks/ ./Frameworks/

# Set default build configuration
ENV CMAKE_BUILD_TYPE=Release
ENV WASM_TARGET=wasp-hosted

# Build command that can be overridden
CMD ["sh", "-c", "mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DWASM=1 -DMY_WASM=1 .. && make ${WASM_TARGET}"]

# Development image with additional tools for debugging
FROM production AS development

# Install additional development tools
RUN apt-get update && apt-get install -y \
    gdb \
    valgrind \
    strace \
    vim \
    nano \
    htop \
    tree \
    && rm -rf /var/lib/apt/lists/*

# Install wabt (WebAssembly Binary Toolkit) for debugging
RUN git clone --recursive https://github.com/WebAssembly/wabt /tmp/wabt \
    && cd /tmp/wabt \
    && git submodule update --init \
    && make gcc-release \
    && cp bin/* /usr/local/bin/ \
    && rm -rf /tmp/wabt

# Set development environment
ENV CMAKE_BUILD_TYPE=Debug
ENV WASM_TARGET=wasp-hosted
WORKDIR /workspace

# Default to bash for development
CMD ["/bin/bash"]