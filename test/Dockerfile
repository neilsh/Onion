FROM ubuntu:22.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies (mirrors CI workflow)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsdl1.2-dev \
    libsdl-image1.2-dev \
    libsdl-ttf2.0-dev \
    libsqlite3-dev \
    libgtest-dev \
    cppcheck \
    && rm -rf /var/lib/apt/lists/*

# Build and install gtest (mirrors CI workflow)
RUN cd /usr/src/gtest \
    && cmake CMakeLists.txt \
    && make \
    && cp lib/*.a /usr/lib/ \
    && ln -sf /usr/lib/libgtest.a /usr/local/lib/libgtest.a \
    && ln -sf /usr/lib/libgtest_main.a /usr/local/lib/libgtest_main.a

# Set working directory
WORKDIR /workspace

# Set gtest include path
ENV GTEST_INCLUDE_DIR=/usr/src/gtest/include

# Default command: run C integration tests
CMD ["make", "run-c-tests"]
