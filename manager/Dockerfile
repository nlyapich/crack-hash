# Стадия сборки
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install conan

RUN conan profile detect --force

WORKDIR /app

COPY conanfile.py .

RUN conan install . --output-folder=build --build=missing -s build_type=Release

COPY common common
COPY manager manager
COPY CMakeLists.txt .

RUN cd build \
    && cmake .. -DCMAKE_TOOLCHAIN_FILE=./build/build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release \
    && cmake --build .

# Стадия выполнения
FROM ubuntu:22.04

WORKDIR /app
COPY --from=builder /app/build/bin/manager /app/manager

EXPOSE ${MANAGER_PORT}

CMD ["/app/manager"]
