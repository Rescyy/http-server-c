FROM ubuntu:24.04 AS build
LABEL authors="Rescyy"

# Install build tools and ASan library for compilation
RUN apt-get update && apt-get install -y \
    cmake \
    gcc \
    g++ \
    make \
    libasan8 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

# Configure and build (ASan flags already set in CMakeLists.txt)
RUN cmake -S . -B build \
 && cmake --build build --parallel

# ----------------------------
# Runtime stage
# ----------------------------
FROM ubuntu:24.04

# Install only the runtime ASan library (needed by instrumented binary)
RUN apt-get update && apt-get install -y libasan8 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary
COPY --from=build /app/build/httpserverc .

# Copy resources
COPY resources ./resources
COPY assets ./assets

EXPOSE 8080

CMD ["./httpserverc"]
