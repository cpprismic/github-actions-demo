# Этап 1: сборка
FROM ubuntu:24.04 AS builder

LABEL stage=builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      cmake \
      git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .
COPY src/ ./src/
COPY tests/ ./tests/

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF && \
    make -j$(nproc)

# Этап 2: минимальный финальный образ
FROM ubuntu:24.04

ARG BUILD_DATE
ARG VCS_REF
ARG VERSION=1.0.0

LABEL org.opencontainers.image.title="System Monitor" \
      org.opencontainers.image.description="Linux system resource monitor" \
      org.opencontainers.image.version="${VERSION}" \
      org.opencontainers.image.created="${BUILD_DATE}" \
      org.opencontainers.image.revision="${VCS_REF}" \
      org.opencontainers.image.source="https://github.com/cpprismic/github-actions-demo"

RUN useradd -r -s /bin/false monitor

COPY --from=builder /app/build/bin/system_monitor /usr/local/bin/

USER monitor

HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD ["/usr/local/bin/system_monitor"]

ENTRYPOINT ["/usr/local/bin/system_monitor"]
CMD []
