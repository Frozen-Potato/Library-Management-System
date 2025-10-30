FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential cmake git pkg-config wget curl unzip \
    libpq-dev libssl-dev libcurl4-openssl-dev zlib1g-dev \
    protobuf-compiler libprotobuf-dev picojson-dev \
    python3 libasio-dev nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Bcrypt
RUN git clone --depth 1 https://github.com/trusch/libbcrypt.git /tmp/libbcrypt && \
    cd /tmp/libbcrypt && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc) && cmake --install build && \
    ldconfig && \
    rm -rf /tmp/libbcrypt

# Install gRPC v1.76.0 + Protobuf (optimized)
RUN git clone --branch v1.76.0 --depth 1 --recurse-submodules --shallow-submodules \
        https://github.com/grpc/grpc.git /tmp/grpc && \
    cmake -S /tmp/grpc -B /tmp/grpc/cmake/build \
          -DgRPC_INSTALL=ON \
          -DgRPC_BUILD_TESTS=OFF \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_STANDARD=20 \
          -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --build /tmp/grpc/cmake/build -j$(nproc) && \
    cmake --install /tmp/grpc/cmake/build && \
    ldconfig && \
    rm -rf /tmp/grpc

# Install libpqxx v7.10.3 (C++20)
RUN git clone --branch 7.10.3 --depth 1 https://github.com/jtv/libpqxx.git /tmp/libpqxx && \
    cmake -S /tmp/libpqxx -B /tmp/libpqxx/build \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          -DSKIP_BUILD_TEST=ON \
          -DCMAKE_CXX_STANDARD=20 \
          -DCMAKE_BUILD_TYPE=Release && \
    cmake --build /tmp/libpqxx/build -j$(nproc) && \
    cmake --install /tmp/libpqxx/build && \
    ldconfig && \
    rm -rf /tmp/libpqxx

# Install Crow (header-only)
RUN git clone --depth 1 https://github.com/CrowCpp/Crow.git /tmp/crow && \
    cmake -S /tmp/crow -B /tmp/crow/build -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --install /tmp/crow/build && \
    rm -rf /tmp/crow

# Install jwt-cpp (header-only)
RUN git clone --depth 1 https://github.com/Thalhammer/jwt-cpp.git /tmp/jwt-cpp && \
    cmake -S /tmp/jwt-cpp -B /tmp/jwt-cpp/build \
          -DJWT_CPP_BUILD_EXAMPLES=OFF \
          -DJWT_CPP_BUILD_TESTS=OFF && \
    cmake --install /tmp/jwt-cpp/build && \
    rm -rf /tmp/jwt-cpp

# Install MongoDB C driver
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libmongoc-dev libbson-dev && \
    rm -rf /var/lib/apt/lists/*

# Install MongoDB C++ driver
RUN git clone --branch r4.1.4 --depth 1 https://github.com/mongodb/mongo-cxx-driver.git /tmp/mongo-cxx-driver && \
    cd /tmp/mongo-cxx-driver/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && make install && \
    ldconfig && \
    rm -rf /tmp/mongo-cxx-driver

# --- Install latest Protocol Buffers (v33.0) ---
RUN apt-get update && apt-get install -y curl unzip tar && \
    cd /tmp && \
    (curl -fLO https://github.com/protocolbuffers/protobuf/releases/download/v33.0/protoc-33.0-linux-x86_64.zip \
     || curl -fLO https://github.com/protocolbuffers/protobuf/releases/download/v33.0/protoc-33.0-linux-x86_64.tar.gz) && \
    if [ -f protoc-33.0-linux-x86_64.zip ]; then \
        unzip protoc-33.0-linux-x86_64.zip -d /usr/local; \
        rm protoc-33.0-linux-x86_64.zip; \
    else \
        tar -xzf protoc-33.0-linux-x86_64.tar.gz -C /usr/local; \
        rm protoc-33.0-linux-x86_64.tar.gz; \
    fi && \
    protoc --version


# Copy source and build
WORKDIR /app
COPY . .

RUN rm -f /app/proto/*.pb.* && \
    cd /app/proto && \
    protoc --cpp_out=. --grpc_out=. \
      --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` log_service.proto

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 && cmake --build build -j$(nproc)

# ---- Runtime Stage ----
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpq5 libssl3 libcurl4 zlib1g \
    libmongoc-1.0-0 libbson-1.0-0 ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary and assets
COPY --from=builder /app/build/library_server /app/library_server
COPY --from=builder /app/proto /app/proto
RUN if [ -f /app/.env ]; then echo "Copying .env from builder..."; \
    cp /app/.env /app/.env; \
    else echo "No .env file found in builder — skipping."; fi || true
COPY --from=builder /usr/local /usr/local

RUN ldconfig

EXPOSE 8080 50051
CMD ["./library_server"]
