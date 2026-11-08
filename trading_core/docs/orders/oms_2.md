# Recovery в OMS (Order Management System)

В HFT Recovery — это не просто восстановление приложения после падения. Это способность OMS восстановить **корректное состояние всех ордеров, позиций и торговых операций после сбоя**, не потеряв события и не создав дублирующие ордера.

Это одна из наиболее важных частей OMS, поскольку ошибка восстановления может привести к реальным финансовым потерям.

Рассмотрим основные подходы с точки зрения C++ и архитектуры low-latency trading systems.

## 1. Какие проблемы должен решать Recovery

Представим следующую последовательность:

1. OMS отправляет ордер на биржу.
2. Биржа принимает ордер.
3. Биржа отправляет подтверждение (New Ack).
4. OMS получает подтверждение.
5. OMS обновляет внутреннее состояние ордера.
6. OMS неожиданно падает.

После перезапуска возникает вопрос: что произошло с ордером?

Возможны разные ситуации:

| Ситуация | Состояние на бирже | Состояние OMS |
|---|---|---|
| Ордер не отправлен | Отсутствует | Неизвестно |
| Ордер отправлен, но ACK не получен | Может существовать | PendingNew |
| Ордер принят | New | PendingNew или New |
| Ордер частично исполнен | PartiallyFilled | Может содержать старое состояние |
| Ордер полностью исполнен | Filled | Может содержать старое состояние |
| Ордер отменён | Canceled | Может содержать старое состояние |

Самая сложная ситуация — неопределённость между отправкой сообщения и получением подтверждения.

OMS не может просто предположить, что операция не состоялась.

Поэтому Recovery должен решать несколько задач:

- Восстановление состояния ордеров.
- Восстановление позиций и исполнений.
- Восстановление последовательности событий.
- Обнаружение потерянных сообщений.
- Синхронизация с фактическим состоянием биржи.
- Предотвращение повторной отправки уже существующих ордеров.

---

## 2. Event Sourcing / Write-Ahead Log (WAL)

Один из фундаментальных подходов — хранить последовательность событий, позволяющую восстановить состояние системы.

Вместо того чтобы сохранять только текущее состояние ордера, OMS записывает все значимые изменения.

Например:

```text
OrderCreated
     |
     v
OrderValidated
     |
     v
OrderSendRequested
     |
     v
OrderSent
     |
     v
OrderAccepted
     |
     v
OrderPartiallyFilled
     |
     v
OrderFilled
```

Каждое событие записывается в журнал (Event Log).

Условная структура:

```cpp
enum class EventType : std::uint8_t
{
    OrderCreated,
    OrderValidated,
    OrderSent,
    OrderAccepted,
    OrderRejected,
    OrderPartiallyFilled,
    OrderFilled,
    OrderCancelRequested,
    OrderCanceled
};

struct OrderEvent
{
    std::uint64_t sequenceNumber;
    std::uint64_t orderId;

    EventType type;

    std::uint64_t timestamp;
    std::uint64_t price;
    std::uint64_t quantity;
};
```

Восстановление:

```cpp
void recover()
{
    for (const auto& event : eventLog)
    {
        applyEvent(event);
    }
}
```

Здесь `applyEvent()` выполняет детерминированный переход состояния OMS.

Например:

```text
OrderCreated:
    status = PendingNew

OrderAccepted:
    status = New

OrderPartiallyFilled:
    status = PartiallyFilled
    filledQuantity += event.quantity

OrderFilled:
    status = Filled
```

### WAL — Write-Ahead Logging

Важный принцип WAL:

**Изменение состояния не должно считаться надёжно зафиксированным, пока соответствующая запись не попала в устойчивое хранилище.**

Упрощённо:

```text
Incoming Event
      |
      v
   Validate
      |
      v
  Append to WAL
      |
      v
  Commit / Persist
      |
      v
  Update OMS State
```

При этом конкретный порядок зависит от гарантий, которые должна обеспечивать система.

Например, если OMS отправляет ордер на биржу, запись о намерении отправить его должна быть сохранена до фактической отправки.

Иначе возможен сценарий:

```text
1. OMS sends order
2. Exchange accepts order
3. OMS crashes
4. No record of order exists in WAL
```

После перезапуска OMS может даже не знать, что такой ордер существует.

### Преимущества

- Возможность восстановить состояние с нуля.
- История всех изменений.
- Возможность аудита.
- Удобная диагностика проблем.
- Возможность воспроизводить последовательность событий.

### Недостатки

- Дополнительные операции записи.
- Увеличение объёма журнала.
- Необходимость контролировать целостность WAL.
- Вопрос latency, особенно при синхронном flush.

Для HFT обычно приходится искать баланс между durability и latency.

---

## 3. Snapshot + Event Log

Полное воспроизведение Event Log при каждом запуске может стать слишком дорогим.

Представим OMS, которая работает несколько месяцев и обрабатывает миллиарды событий.

Восстанавливать всё состояние с самого начала — не очень хорошая идея.

Для решения этой проблемы используется комбинация:

- Event Log — последовательность событий.
- Snapshot — периодически сохранённое состояние системы.

Например:

```text
Event Log
    |
    v
[Event 1]
[Event 2]
[Event 3]
...
[Event 1000000]
[Event 1000001]
...
[Event 2000000]

         Snapshot
             |
             v
      State at Event 1000000
```

Восстановление:

1. Загружаем последний Snapshot.
2. Определяем последний обработанный sequence number.
3. Читаем Event Log начиная со следующего события.
4. Последовательно применяем события.
5. Получаем актуальное состояние OMS.

Пример:

```cpp
struct Snapshot
{
    std::uint64_t lastSequenceNumber;

    std::vector<Order> orders;
    std::vector<Position> positions;
};
```

```cpp
void recover()
{
    Snapshot snapshot = loadSnapshot();

    restoreState(snapshot);

    for (const auto& event : eventLog.readFrom(snapshot.lastSequenceNumber + 1))
    {
        applyEvent(event);
    }
}
```

### Важный нюанс

Snapshot должен представлять согласованное состояние системы на определённый момент времени.

Нельзя просто сохранить `orders`, затем отдельно `positions`, а потом записать sequence number, если в это время продолжают обрабатываться события.

Иначе можно получить Snapshot, в котором:

- ордер уже исполнен;
- позиция ещё не обновлена;
- sequence number уже указывает на обработанное событие.

Для решения применяют:
- остановку обработки на время создания Snapshot;
- consistent checkpoint;
- copy-on-write;
- отдельный поток snapshotting с использованием согласованного состояния.

В HFT обычно стараются минимизировать влияние snapshotting на основной trading path.

---

## 4. Exchange Reconciliation

Это, пожалуй, один из наиболее важных механизмов Recovery именно для OMS.

**Exchange Reconciliation — процесс сверки внутреннего состояния OMS с фактическим состоянием на бирже.**

Почему это необходимо?

Потому что локальный Event Log не обязательно содержит абсолютно все события, которые произошли на бирже.

Например:

```text
OMS                         Exchange
 |                              |
 |-------- New Order ---------->|
 |                              |
 |                         Order Accepted
 |                              |
 |                         Partial Fill
 |                              |
 |          CRASH               |
 X                              |
                                |
                           Order Filled
```

После перезапуска OMS может восстановить состояние `New`, хотя на бирже ордер уже `Filled`.

Даже если WAL и Snapshot реализованы идеально, остаётся необходимость сверки с внешней системой.

### Как происходит reconciliation

Упрощённая последовательность:

```text
              OMS Restart
                   |
                   v
           Restore Local State
                   |
                   v
          Connect to Exchange
                   |
                   v
          Request Open Orders
                   |
                   v
         Request Recent Fills
                   |
                   v
          Compare Local State
                   |
                   v
         Reconciliation Logic
                   |
                   v
          Trading Enabled
```

OMS получает от биржи:

1. Список активных ордеров (Open Orders).
2. Историю исполнений (Trade History / Fills).
3. Состояние позиций (если предоставляется).
4. Другую доступную информацию о торговых операциях.

После этого сравнивает её с локальным состоянием.

Например:

| Order ID | OMS | Exchange | Результат |
|---|---|---|---|
| 1001 | New | New | OK |
| 1002 | PartiallyFilled | Filled | Update OMS |
| 1003 | New | Not Found | Investigate |
| 1004 | Unknown | New | Recover Order |

Особенно интересны последние два случая.

**OMS знает об ордере, но биржа его не находит.**

Возможные причины:
- Ордер был отменён.
- Ордер полностью исполнен.
- Ордер отклонён.
- История биржи уже недоступна.
- OMS использует неправильный идентификатор.

**Биржа знает об ордере, но OMS его не знает.**

Возможные причины:
- OMS упала после отправки ордера.
- Локальная запись не была сохранена.
- Произошла ошибка восстановления.
- Ордер был отправлен другим компонентом.

Такие ситуации нельзя автоматически разрешать одним универсальным правилом. Нужна reconciliation logic с учётом протокола конкретной биржи.

### FIX Protocol

Если используется FIX, полезно знать следующие сообщения:

| Message | Назначение |
|---|---|
| ExecutionReport (35=8) | Обновление состояния ордера |
| OrderCancelRequest (35=F) | Запрос отмены |
| OrderCancelReject (35=9) | Отказ в отмене |
| OrderStatusRequest (35=H) | Запрос состояния ордера |
| ResendRequest (35=2) | Запрос повторной передачи сообщений |
| SequenceReset (35=4) | Коррекция последовательности сообщений |

В FIX также есть механизм восстановления пропущенных сообщений через `MsgSeqNum` и `ResendRequest`.

Это позволяет восстановить последовательность FIX-сообщений при разрыве соединения, если контрагент поддерживает соответствующую историю.

Но важно понимать: FIX session recovery и OMS recovery — это разные уровни восстановления.

FIX восстанавливает последовательность сообщений с контрагентом, а OMS — согласованное состояние торговой системы.

---

## 5. Idempotency и Duplicate Detection

Ещё одна фундаментальная проблема — повторная обработка событий.

Представим:

```text
1. Exchange sends ExecutionReport
2. OMS receives ExecutionReport
3. OMS updates Order
4. OMS crashes before persisting the event
5. OMS restarts
6. Exchange resends ExecutionReport
```

Если обработать сообщение повторно, можно дважды учесть исполнение.

Например:

```text
Original Quantity: 10 BTC

First Processing:
    Filled Quantity = 5 BTC

Duplicate Processing:
    Filled Quantity = 10 BTC
```

Хотя реально было исполнено только 5 BTC.

### Решение: уникальные идентификаторы событий

Для каждого execution report необходимо иметь идентификатор, позволяющий обнаружить дубликат.

В FIX для этого используется `ExecID` (tag 17).

Условная структура:

```cpp
struct ExecutionReport
{
    std::uint64_t orderId;
    std::uint64_t executionId;

    std::uint64_t price;
    std::uint64_t quantity;
};
```

При обработке:

```cpp
void processExecution(const ExecutionReport& report)
{
    if (processedExecutions.contains(report.executionId))
    {
        return;
    }

    processedExecutions.insert(report.executionId);

    updateOrder(report);
    updatePosition(report);
}
```

Это иллюстрация логики, а не готовая production-реализация.

В реальной системе проверка дубликата, запись события и изменение состояния должны быть согласованы с точки зрения durability.

Например, если `processedExecutions` обновился, а WAL не сохранил execution, после рестарта проблема повторится.

Поэтому обычно используют атомарную фиксацию события и его идентификатора в журнале.

### А что насчёт повторной отправки ордера?

Здесь используется другой механизм — уникальный идентификатор клиентского ордера.

В FIX это `ClOrdID` (tag 11).

Например:

```text
ClOrdID = STRATEGY_01_000012345
```

При восстановлении OMS может использовать этот идентификатор для поиска ранее отправленного ордера.

Но есть важный нюанс:

**Нельзя считать, что повторная отправка ордера с тем же ClOrdID автоматически безопасна.**

Поведение при повторном использовании идентификатора зависит от биржи и её протокола.

Правильная стратегия при неопределённом результате отправки:

1. Не отправлять ордер повторно вслепую.
2. Найти его по Client Order ID, если биржа поддерживает такой поиск.
3. Получить состояние ордера и историю исполнений.
4. Только после сверки принять решение о дальнейших действиях.

Это одна из причин, почему гарантировать exactly-once delivery между OMS и внешней биржей практически невозможно.

Обычно строят систему с at-least-once delivery и idempotent processing, дополняя её reconciliation.

---

## 6. High Availability (HA) и Active-Passive Architecture

До этого мы рассматривали восстановление после полного падения OMS.

Но в торговых системах часто требуется сократить время восстановления.

Для этого применяют High Availability.

Один из распространённых вариантов — Active-Passive.

```text
             Trading Strategies
                     |
                     v
              +-------------+
              |   Active    |
              |     OMS     |
              +-------------+
                     |
                     v
                Event Log
                     |
              +------+------+
              |             |
              v             v
         Persistent     Standby OMS
          Storage        (Passive)
```

В нормальном режиме:

- Active OMS обрабатывает торговые операции.
- Passive OMS получает события и поддерживает актуальную копию состояния.
- Passive OMS не отправляет торговые ордера.

При отказе Active:

```text
       Active OMS
            |
          CRASH
            |
            v
      Failure Detection
            |
            v
      Passive Promotion
            |
            v
      State Validation
            |
            v
      Exchange Reconciliation
            |
            v
      Trading Enabled
```

### Split-Brain Problem

Здесь возникает серьёзная проблема.

Представим, что Active OMS не упала, а просто потеряла связь с Passive OMS.

Passive OMS может решить, что Active умерла, и начать самостоятельно отправлять ордера.

В результате обе системы одновременно управляют одной торговой инфраструктурой.

Это называется Split-Brain.

Возможные решения:

- Leader Election.
- Distributed Consensus.
- Fencing Tokens.
- Lease-based Leadership.

Особенно важен механизм Fencing.

Например, каждая активная инстанция получает уникальный generation ID:

```text
Active OMS:
    Generation = 100

Passive OMS:
    Generation = 101
```

Компоненты, отвечающие за отправку ордеров, должны отклонять команды от устаревшего поколения.

Важно: одного локального generation ID недостаточно. Проверка fencing должна выполняться в точке, которая действительно контролирует отправку ордеров.

---

## 7. Journal / Message Replay

Есть ещё один интересный подход, особенно актуальный для low-latency C++ систем.

Вместо сохранения сложного состояния отдельных объектов можно сохранять последовательность входящих сообщений и повторно проигрывать их после рестарта.

Например:

```text
Market Data
    |
    v
Trading Strategy
    |
    v
OMS Commands
    |
    v
Message Journal
    |
    v
OMS Processing
```

В журнал попадают:

- OrderNew.
- OrderCancel.
- OrderModify.
- ExecutionReport.
- OrderReject.
- RiskCheckResult.

При восстановлении система повторно обрабатывает сообщения в том же порядке.

Это близко к Event Sourcing, но акцент здесь на replay входных сообщений и детерминированном восстановлении.

Особенно важно сохранять порядок событий.

Например:

```text
Sequence 1001: OrderCreated
Sequence 1002: OrderSent
Sequence 1003: OrderAccepted
Sequence 1004: PartialFill
Sequence 1005: CancelRequested
Sequence 1006: CancelRejected
Sequence 1007: Filled
```

Если воспроизвести события в неправильном порядке, можно получить некорректное состояние.

Для распределённой OMS также важно понимать, что глобальный порядок всех событий не всегда необходим. Часто достаточно гарантировать порядок внутри конкретного order stream или partition.

---

## 8. Как это реализовать в C++ с учётом Low Latency

Здесь начинается самое интересное с точки зрения системного программирования.

Не все операции OMS одинаково критичны по latency.

Я бы разделил систему на два условных контура.

### Trading Path

Критичный путь обработки торговых операций:

```text
Strategy
   |
   v
Order Validation
   |
   v
Risk Checks
   |
   v
Order State Update
   |
   v
Exchange Connector
```

Основные требования:

- Минимум динамических аллокаций.
- Минимум блокировок.
- Предсказуемая latency.
- Preallocated memory.
- Эффективная передача сообщений между потоками.

### Recovery / Persistence Path

Отвечает за сохранение состояния и восстановление:

```text
             Trading Path
                  |
                  v
             Event Queue
                  |
                  v
             WAL Writer
                  |
                  v
           Persistent Storage
                  |
                  v
              Snapshots
```

Здесь можно использовать отдельный поток для записи WAL.

Но возникает принципиальный вопрос:

**Можно ли отправлять ордер на биржу, не дожидаясь записи соответствующего события в WAL?**

Это компромисс между latency и durability.

Рассмотрим два варианта.

**Вариант A — Synchronous Persistence**

```text
Create Order
     |
     v
Write WAL
     |
     v
Persist
     |
     v
Send to Exchange
```

Преимущества:
- Более строгие гарантии сохранности.
- После рестарта известно, какие ордера планировалось отправить.

Недостатки:
- Дополнительная latency.
- Зависимость trading path от storage.

**Вариант B — Asynchronous Persistence**

```text
Create Order
     |
     +------------------+
     |                  |
     v                  v
Send to Exchange    WAL Queue
                        |
                        v
                    WAL Writer
```

Преимущества:
- Меньше latency на trading path.
- Независимая работа persistence thread.

Недостатки:
- Возможна потеря последних событий при аварии.
- После рестарта может потребоваться более сложная reconciliation.

Для HFT это действительно важный архитектурный компромисс.

При этом асинхронная запись не обязательно означает потерю durability: можно использовать batching, group commit и другие механизмы, но необходимо чётко определить момент, после которого операция считается устойчиво зафиксированной.

---

# 9. Какой подход я бы выбрал для HFT OMS

Я бы не рассматривал перечисленные подходы как взаимоисключающие. В production-системе они обычно комбинируются.

Моя отправная точка выглядела бы так:

| Механизм | Назначение |
|---|---|
| Event Log / WAL | Сохранение последовательности событий |
| Snapshot | Ускорение восстановления |
| Exchange Reconciliation | Сверка с реальным состоянием биржи |
| Execution Deduplication | Защита от повторной обработки исполнений |
| Client Order ID | Идентификация отправленных ордеров |
| Sequence Numbers | Контроль порядка и пропущенных сообщений |
| Active-Passive | Быстрое переключение на резервную инстанцию |
| Fencing | Защита от одновременной работы нескольких Active |

Пример общей архитектуры:

```text
                  Trading Strategy
                         |
                         v
                  +-------------+
                  |     OMS     |
                  +-------------+
                         |
                +--------+--------+
                |                 |
                v                 v
           Order State       Event Journal
                |                 |
                v                 v
          Order Router        WAL Writer
                |                 |
                v                 v
         Exchange Connector  Persistent Storage
                |
                v
             Exchange
                |
                v
          Execution Reports
                |
                v
              OMS

         Recovery Components
         -------------------

         Snapshot Manager
         Event Replay
         Execution Deduplication
         Exchange Reconciliation
         State Validation
```

## 10. Несколько важных вопросов для дальнейшего изучения

Если рассматривать OMS как C++ engineering project, я бы особенно рекомендовал разобраться в следующих темах:

| Тема | Что изучать |
|---|---|
| Write-Ahead Log | Append-only log, fsync, group commit |
| Event Sourcing | Event replay, deterministic state transitions |
| Crash Consistency | Atomic writes, torn writes, checksums |
| Sequence Management | Gap detection, message ordering |
| Idempotency | Duplicate execution handling |
| FIX Recovery | ResendRequest, SequenceReset, PossDupFlag |
| Reconciliation | Open Orders, Executions, Positions |
| High Availability | Active-Passive, Leader Election, Fencing |
| Storage | mmap, O_DIRECT, io_uring, persistent memory |

---

### 11. На что я бы обратил особое внимание

Есть три проблемы, которые особенно интересны с инженерной точки зрения.

**1. Crash между отправкой ордера и сохранением результата**

```text
WAL: OrderSendRequested
       |
       v
Exchange: Order Accepted
       |
       v
OMS CRASH
       |
       v
Recovery
```

Как определить, был ли ордер принят?

Это классическая проблема взаимодействия с внешней системой без общей транзакции.

**2. Crash между получением Execution Report и его сохранением**

Как гарантировать, что после восстановления исполнение не будет потеряно или учтено дважды?

Здесь важны атомарность WAL и обновления состояния, а также deduplication по `ExecID`.

**3. Recovery без остановки всей торговой инфраструктуры**

Например, одна биржа доступна, а другая нет.

Можно ли восстановить торговлю на доступной бирже, не дожидаясь полной синхронизации остальных?

Это уже вопрос из области fault isolation и distributed trading architecture.

---

**Мой совет:** если хочется глубоко разобраться в Recovery для HFT, начни с реализации небольшого WAL + Snapshot + Replay для OMS.

Причём я бы специально предусмотрел crash injection в разных точках обработки ордера и проверял, что после каждого рестарта система восстанавливает корректное состояние.

Это даст гораздо больше практического понимания, чем простое изучение Event Sourcing или WAL по отдельности.