# CrackHash C++

Распределённая система для подбора паролей по MD5-хэшу методом brute-force.

### Компоненты

| Компонент | Описание |
|-----------|----------|
| **Manager** | HTTP-сервер, принимает запросы от клиентов, распределяет задачи между воркерами, агрегирует результаты |
| **Worker** | HTTP-сервер, получает задачу, выполняет перебор хэшей в заданном диапазоне, возвращает найденные совпадения |
| **Common** | Общая библиотека: модели данных, MD5-хеширование, комбинаторика, thread-safe map |

## Технологии

- **C++17**
- **CMake** — система сборки
- **Conan** — менеджер зависимостей
- **cpp-httplib** — HTTP-сервер/клиент
- **nlohmann/json** — работа с JSON
- **OpenSSL** — MD5-хеширование
- **spdlog** — логирование
- **libuuid** — генерация UUID
- **Docker & Docker Compose** — контейнеризация

## Запуск через Docker Compose

#### Сборка

```bash
docker-compose build --progress=plain
```

#### Запуск

```bash
docker-compose up
```

#### Очистка кэша

```bash
docker-compose down -v --rmi all
docker builder prune -af
```

#### Примеры использования

**Отправить запрос на взлом хэша:**

```bash
curl -X POST http://localhost:8081/api/hash/crack \
  -H 'Content-Type: application/json' \
  -d '{"hash":"e2fc714c4727ee9395f324cd2e7f331f","maxLength":4}'
```

Ответ:
```json
{"requestId": "<uuid>"}
```

**Проверить статус:**

```bash
curl http://localhost:8081/api/hash/status?requestId=<requestId>
```

Ответы:
```json
// В процессе
{"status": "IN_PROGRESS", "data": []}

// Готово
{"status": "READY", "data": ["abc", "def"]}

// Ошибка
{"status": "ERROR", "data": []}
```

Сервисы будут доступны:
- Менеджер: `http://localhost:8081`
- Worker: `http://localhost:8080` (внутренняя сеть)

## API

### Менеджер

| Метод | Endpoint | Описание |
|-------|----------|----------|
| `POST` | `/api/hash/crack` | Создать задачу на взлом хэша |
| `GET` | `/api/hash/status?requestId=<id>` | Получить статус задачи |
| `PATCH` | `/internal/api/manager/hash/crack/request` | Внутренний API для воркеров (отправка результатов) |

### Worker

| Метод | Endpoint | Описание |
|-------|----------|----------|
| `POST` | `/internal/api/worker/hash/crack/task` | Внутренний API для менеджера (получение задачи) |

### Форматы данных

**CrackRequest:**
```json
{
  "hash": "e2fc714c4727ee9395f324cd2e7f331f",
  "maxLength": 4
}
```

**CrackResponse:**
```json
{
  "requestId": "550e8400-e29b-41d4-a716-446655440000"
}
```

**StatusResponse:**
```json
{
  "status": "READY",
  "data": ["abc", "test"]
}
```

Возможные значения `status`:
- `IN_PROGRESS` — задача выполняется
- `READY` — задача завершена, результаты готовы
- `PARTIALLY_READY` — задача частично готова
- `ERROR` — произошла ошибка или истёк таймаут (300 сек)

## Как это работает

1. **Клиент** отправляет POST-запрос менеджеру с хэшем и максимальной длиной пароля

2. **Manager**:
   - Генерирует уникальный `requestId`
   - Вычисляет общее количество комбинаций
   - Делит пространство перебора на равные части по количеству воркеров
   - Отправляет каждому воркеру `WorkerTask` с указанием диапазона (`partNumber`, `partCount`)

3. **Worker**:
   - Получает задачу
   - В отдельном потоке перебирает строки в своём диапазоне:
     - `getByIndex(alphabet, maxLength, i)` → строка
     - `md5(string)` → хэш
     - Сравнение с искомым хэшем
   - Отправляет найденные совпадения менеджеру через PATCH-запрос

4. **Manager** накапливает результаты от всех воркеров. Когда все воркеры завершили работу - статус становится `READY`

5. **Клиент** опрашивает статус и получает результаты

### Алфавит

По умолчанию используется: `abcdefghijklmnopqrstuvwxyz0123456789` (36 символов)
