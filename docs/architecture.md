---
layout: default
title: Architecture
nav_order: 6
---

# Source architecture

Applications keep including `ESPDashboardPlus.h`. It exposes the same card types,
fields, callbacks, default arguments and dashboard methods. Implementations now
live in `.cpp` files, so consumers must build the complete `src` directory rather
than copy just the former standalone header.

## Responsibilities and dependencies

| Component | Responsibility |
|-----------|----------------|
| `ESPDashboardPlus.h/.cpp` | Public facade and composition of the internal services |
| `dashboard/Types.h/.cpp` | Public enums, callback aliases, data structures, enum names and timestamps |
| `dashboard/DashboardCard.h/.cpp` | Polymorphic card contract and common JSON fields/layout helpers |
| `dashboard/cards/*.h/.cpp` | One card's state, serialization and input callbacks per pair of files |
| `dashboard/CardRegistry.h/.cpp` | Card ownership, lookup, removal and group membership |
| `dashboard/JsonProtocol.h/.cpp` | JSON envelopes for initialization, updates, removal, heartbeat and logs |
| `dashboard/MessageRouter.h/.cpp` | Incoming JSON parsing and dispatch to cards, commands or OTA |
| `dashboard/AsyncDashboardTransport.h/.cpp` | ESPAsyncWebServer HTTP/WebSocket integration, frame assembly and upkeep |
| `dashboard/OtaSession.h/.cpp` | Shared start/chunk/end sequence, decoding and upload state |
| `dashboard/FirmwareUpdate.h/.cpp` | Small firmware-writing port and the ESP32 `Update` adapter |

Internal services use `espdashboardplus::detail`. Existing public classes and
types retain their global names. The public facade owns its implementation through
an opaque `std::unique_ptr`; instances are explicitly noncopyable. They own card
state and must outlive the server callbacks registered by `begin()`. The server
still owns its registered WebSocket handler. Returned card pointers are observers;
the registry owns the cards. Card IDs must remain unique, as required by the API.

## SOLID and DRY choices

The **Facade** keeps application code unchanged while each service has one
reason to change: a new JSON envelope does not require editing the network adapter,
and a firmware-writing detail does not belong in a card class. **PImpl** keeps
private service definitions out of the public header.

`DashboardCard` retains its virtual serialization and action contract. Each card
overrides those operations, while `beginJson()` and `writeSize()` centralize the
repeated common fields. The legacy `OTACard` and `ConsoleCard` types are retained,
including their existing serialized fields. Enum conversions and log timestamp
formatting also have one implementation.

`OtaSession` depends on the small `FirmwareUpdate` interface. The concrete
**Adapter**, `ArduinoFirmwareUpdate`, supplies the ESP32 calls; tests can supply
an in-memory writer without flashing hardware. Both card-based and tab-based OTA
requests reach the same session, removing the duplicated upload sequence.

The router receives a reply callback instead of an AsyncWebSocket client. This
keeps its dependency on the operation it needs, and permits independent tests.
No generic service container or extra virtual transport hierarchy is needed.

The facade shares typed card creation and update helpers internally. Public
factory and update methods remain explicit, including both chart overloads.
Scalar update patches retain the existing wire values, even where a card clamps
its local state. Card property setters retain their existing notification behavior.

This is a structural refactoring. Existing authentication policy, OTA feature flags,
frame-buffer scope, HTML caching and timing are not changed by this split.

## Validation

`tests/native/expected_messages.jsonl` was captured from the pre-refactor header.
The native suite compares the new serialized messages against that fixture and
checks card behavior, callbacks, groups, initial connection, fragmented input,
HTTP/gzip headers, update patches, log messages and both OTA entry points. Simulated
OTA checks include successful uploads, padded base64 chunks, short writes, size
mismatches, empty chunks, failed starts and failed completion. A separate test
injects a firmware backend directly into the session.

After building a PlatformIO environment to install ArduinoJson, run:

```sh
python scripts/test_native.py --environment esp32dev
```

The runner uses MSVC C++ Build Tools on Windows or a C++17 compiler (`CXX`, or
`c++`) on Linux. Native stubs replace Arduino and the server only in these tests;
ArduinoJson itself is the installed library. CI runs this suite after each board
build. The full example builds on `esp32dev`, `esp32-s3` and `esp32-c3` additionally
validate linkage with the real Arduino framework and ESPAsyncWebServer.

The library manifest exports nested sources. Package contents must include
`src/ESPDashboardPlus.cpp`, `src/dashboard/*.cpp` and `src/dashboard/cards/*.cpp`.
Native simulations establish software behavior, not successful flashing or OTA on
physical hardware.
