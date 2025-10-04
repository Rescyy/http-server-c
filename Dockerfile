FROM ubuntu:24.04 AS build
LABEL authors="Rescyy"

RUN apt-get update && apt-get install -y \
    cmake \
    gcc \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Configure and build (with tests enabled)
RUN cmake -S . -B build \
 && cmake --build build

# ----------------------------
# Runtime stage
# ----------------------------
FROM ubuntu:latest

WORKDIR /app

# Copy compiled binary and tests
COPY --from=build /app/build/httpserverc .
#COPY --from=build /app/build/tests ./tests

COPY resources ./resources
COPY assets ./assets

# Expose port if your app listens on one (example: 8080)
EXPOSE 8080

# Default run main application
CMD ["./httpserverc"]