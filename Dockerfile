# ============================
# Build & Test Stage
# ============================
FROM ubuntu:24.04 AS build
LABEL authors="Rescyy"

# Install build tools and ASan
RUN apt-get update && apt-get install -y \
    cmake \
    gcc \
    g++ \
    make \
    libasan8 \
    coreutils \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy all source files
COPY . .

# Configure and build with testing enabled
RUN cmake -S . -B build -DENABLE_TESTING=ON \
 && cmake --build build --parallel

# Run tests with ASan and verbose output, timeout after 60s
RUN cd build && \
    ASAN_OPTIONS=verbosity=1:detect_leaks=1:halt_on_error=1 \
    timeout 60s ctest --output-on-failure --verbose \
    || (echo "Tests failed or timed out"; cat Testing/Temporary/LastTest.log || true; exit 1)

# ============================
# Runtime Stage
# ============================
FROM ubuntu:24.04

# Install only runtime ASan dependency
RUN apt-get update && apt-get install -y libasan8 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the final executable and resources
COPY --from=build /app/build/httpserverc ./httpserverc
COPY --from=build /app/resources ./resources
COPY --from=build /app/assets ./assets

EXPOSE 8080

# Run the main application
CMD ["./httpserverc"]
