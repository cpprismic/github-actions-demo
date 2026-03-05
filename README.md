# System Monitor — демонстрационный проект

## О проекте

Этот репозиторий является **учебно-демонстрационным проектом**, созданным с целью изучения и практики построения современного CI/CD-пайплайна на базе **GitHub Actions**. Само приложение — монитор системных ресурсов Linux на C++17 — выступает в роли реального, но намеренно несложного рабочего кода, на примере которого выстраивается полноценный процесс разработки.

## Что демонстрирует проект

### CI/CD с GitHub Actions

- **Многоэтапный пайплайн** — сборка, тестирование, сборка Docker-образа и сканирование безопасности как независимые jobs с явными зависимостями
- **Кэширование артефактов сборки** — повторное использование результатов CMake между jobs через `actions/cache` и `actions/upload-artifact`
- **Multi-arch Docker-образы** — сборка под `linux/amd64` и `linux/arm64` в одном job через Docker Buildx и QEMU без дублирования тегов
- **Контроль качества** — Trivy сканирует файловую систему на уязвимости, результаты публикуются в GitHub Security
- **Production-деплой** — ручной запуск через `workflow_dispatch` с подтверждением, проверкой статуса CI, SSH-деплоем на VPS, health-check и записью в GitHub Deployments

### C++ и сборка

- Структура проекта с **функциональной группировкой** исходников (`src/core/`, `src/utils/`)
- Сборка через **CMake** с поддержкой Debug/Release конфигураций
- Юнит-тесты на **Google Test**, запускаемые через CTest

### Docker

- **Multi-stage Dockerfile** — отдельная стадия сборки (builder) и минимальный финальный образ
- Запуск от непривилегированного пользователя
- Монтирование `/proc`, `/sys`, `/etc` хоста в контейнер для чтения системных метрик
- `HEALTHCHECK` и ограничения ресурсов через `deploy.resources`

## Структура репозитория

```
.
├── src/
│   ├── core/               # Класс SystemMonitor — чтение /proc
│   ├── utils/              # Вспомогательные утилиты
│   └── main.cpp
├── tests/                  # Google Test юнит-тесты
├── .github/workflows/
│   ├── ci.yaml             # Build → Test → Docker → Security
│   └── deploy-production.yaml  # Ручной деплой на VPS
├── dockerfile              # Multi-stage сборка
└── docker-compose.yml      # Запуск на сервере
```

## Что делает приложение

Читает данные из файловой системы `/proc` и выводит:

- Имя хоста и время работы системы
- Загрузку процессора (дифференциальный расчёт между двумя чтениями `/proc/stat`)
- Использование памяти (`/proc/meminfo`)

Поддерживает работу внутри Docker-контейнера с примонтированным `/host/proc`.

```
=== System Information ===
Timestamp:      2025-01-15 14:32:01
Hostname:       my-server
Uptime:         2d 7h 14m
CPU Usage:      12.4%
Memory Usage:   61.3% (4953 MB / 8074 MB)
==========================
```

## Сборка и запуск

### Локально (Linux)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

./bin/system_monitor          # однократный вывод
./bin/system_monitor -c 3     # непрерывный режим, интервал 3 сек
```

### Тесты

```bash
cd build
ctest --output-on-failure
# или напрямую:
./bin/unit_tests
```

### Docker

```bash
# Сборка образа
docker build -t system-monitor .

# Запуск с доступом к метрикам хоста
docker run --rm \
  -v /proc:/host/proc:ro \
  -v /sys:/host/sys:ro \
  -v /etc:/host/etc:ro \
  system-monitor -c 5

# Через docker-compose (использует готовый образ с Docker Hub)
docker-compose up -d
docker-compose logs -f
docker-compose down
```

## Используемые технологии

| Категория | Инструменты |
|-----------|-------------|
| Язык | C++17 |
| Сборка | CMake 3.16+, GCC/Clang |
| Тесты | Google Test 1.14 |
| CI/CD | GitHub Actions |
| Контейнеризация | Docker, Docker Compose, Buildx |
| Безопасность | Trivy |
| Документация | Doxygen |
