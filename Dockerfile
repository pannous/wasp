# Multi-stage build for WASP WebAssembly Programming Language
FROM ubuntu:22.04 AS base

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies

# IF REFRESH
#RUN rm -f /etc/apt/apt.conf.d/docker-clean \
#RUN rm -rf /var/lib/apt/lists/*
RUN echo 'Acquire::AllowUnauthenticated "true";' > /etc/apt/apt.conf.d/allow-insecure
RUN apt-get update || true && apt-get install --reinstall -y ca-certificates gnupg
#RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
#    --mount=target=/var/cache/apt,type=cache,sharing=locked \
#    rm -f /etc/apt/apt.conf.d/docker-clean \
#    && apt-get update  \
RUN apt-get update  \
    && apt-get -y --no-install-recommends install  \
    build-essential \
    cmake \
    clang \
    llvm \
    git \
    curl \
    wget \
    python3 \
    python3-pip \
    libc6-dev \
    libreadline-dev \
    libcurl4-openssl-dev \
    pkg-config \
		ninja-build \
		llvm-15-dev libllvm15

# Create workspace
WORKDIR /workspace

COPY CMakeLists.txt ./
COPY source/ ./source/

# Install WASI SDK for WebAssembly compilation
FROM base AS wasi-sdk
WORKDIR /opt/wasm
COPY wasm.toolchain.cmake ./
#RUN wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-25/wasi-sdk-25.0-x86_64-linux.tar.gz  \
#    && tar -xzf  wasi-sdk-25.0-x86_64-linux.tar.gz  \
RUN wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-25/wasi-sdk-25.0-arm64-linux.tar.gz \
    && tar -xzf wasi-sdk-25.0-arm64-linux.tar.gz \
    && mv wasi-sdk-25.0-arm64-linux wasi-sdk \
    && rm wasi-sdk-25.0-arm64-linux.tar.gz

# Set environment variables
ENV PATH="/opt/wasm/wasi-sdk/bin:${PATH}"
ENV WASI_SDK_PATH="/opt/wasm/wasi-sdk"
ENV CMAKE_TOOLCHAIN_FILE="/workspace/wasm.toolchain.cmake"

FROM wasi-sdk AS wasm-runtime
# OPTIONAL: Build the WASP runtime (wasp-runtime.wasm) for testing
RUN mkdir -p cmake-build-wasm-runtime
RUN cd cmake-build-wasm-runtime && \
    rm CMakeCache.txt CMakeFiles || true && \
    cmake -DCMAKE_TOOLCHAIN_FILE=wasm.toolchain.cmake \
      -DCMAKE_OSX_ARCHITECTURES:STRING= \
      -DCMAKE_C_COMPILER=/opt/wasm/wasi-sdk/bin/clang \
      -DCMAKE_CXX_COMPILER=/opt/wasm/wasi-sdk/bin/clang++ \
      -DWASM=1 -DRUNTIME_ONLY=1 -DNO_TESTS=1 \
      -S .. -B . && make wasp-runtime && ls

RUN cp cmake-build-wasm-runtime/wasp-runtime.wasm .

FROM base AS wasmedge
#RUN curl -sSf https://raw.githubusercontent.com/WasmEdge/WasmEdge/master/utils/install.sh | bash -s -- -p Frameworks/WasmEdge
COPY Frameworks/install_wasmedge_0.14.sh Frameworks/install_wasmedge_0.14.sh
RUN Frameworks/install_wasmedge_0.14.sh -p Frameworks/WasmEdge

FROM wasmedge AS shell
CMD ["/bin/bash"]


FROM wasmedge AS binary
#Build the WASP native binary
RUN apt-get update && apt-get install -y libcurl4-openssl-dev libreadline-dev && rm -rf /var/lib/apt/lists/*
RUN mkdir -p cmake-build-release
COPY readline-config.cmake .
COPY FindReadline.cmake .
COPY source/increase-wasp-version.py source/increase-wasp-version.py

RUN (test -f Frameworks/WasmEdge/lib/libwasmedge.so &&  echo "libwasmedge.so found") || (echo "libwasmedge.so not found" && exit 1)
RUN cd cmake-build-release \
    && ln -s ../wasp-runtime.wasm \
    && cmake -DNO_CONSOLE=1 -DCURL_INCLUDE_DIR=$(pkg-config --cflags-only-I libcurl | sed 's/-I//') \
      -DCURL_LIBRARY=$(pkg-config --libs-only-L libcurl | sed 's/-L//')libcurl.so  -DDEBUG=1 ..  \
    && make && cp wasp ..


# Development image with additional tools for debugging
FROM binary AS development

RUN mkdir -p cmake-build-debug
RUN cd cmake-build-debug && cmake -DDEBUG=1 -DNO_CONSOLE=1 -DCURL_INCLUDE_DIR=$(pkg-config --cflags-only-I libcurl | sed 's/-I//') \
      -DCURL_LIBRARY=$(pkg-config --libs-only-L libcurl | sed 's/-L//')libcurl.so .. && make && cp wasp ..

#FROM development AS test
FROM binary AS test
RUN ./wasp test | grep -q 'CURRENT TESTS PASSED'

# Install additional development tools
#RUN apt-get update && apt-get install -y \
#    gdb \
#    valgrind \
#    strace \
#    vim \
#    nano \
#    htop \
#    tree \
#    && rm -rf /var/lib/apt/lists/*

# Clean up APT when done
RUN rm -f /etc/apt/apt.conf.d/docker-clean \
 && rm -rf /var/lib/apt/lists/*  # Reduziert das finale Image

# Default to bash for development
CMD ["/bin/bash"]