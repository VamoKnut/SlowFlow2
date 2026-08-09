# SlowFlow

SlowFlow is an MVP and production prototype for active control of roof drainage
systems using remotely managed drain nodes.

The project includes:

* Arduino Portenta H7 firmware
* ASP.NET Core and Blazor backend and web application
* MQTT communication
* SQL Server persistence
* Integration and Automation REST APIs

The authoritative functional and technical specification is:

[`docs/SlowFlow_Requirements_Specification_v1.md`](docs/SlowFlow_Requirements_Specification_v1.md)

Development and agent-specific working instructions are defined in:

[`AGENTS.md`](AGENTS.md)

The project is implemented incrementally according to the phases defined in the requirements specification.

## Repository layout

```text
SlowFlow.sln
src/
|-- SlowFlow.Core/
|-- SlowFlow.Infrastructure/
`-- SlowFlow.Web/
tests/
|-- SlowFlow.Core.Tests/
|-- SlowFlow.Infrastructure.Tests/
`-- SlowFlow.Web.Tests/
firmware/
|-- include/
|-- lib/
|-- src/
`-- test/
docs/
```

The backend is one modular monolith. `SlowFlow.Core` has no infrastructure
dependency, `SlowFlow.Infrastructure` depends on Core, and `SlowFlow.Web`
composes both.

## Prerequisites

* .NET SDK selected by [`global.json`](global.json)
* PlatformIO Core 6.1.19 or the PlatformIO VS Code extension

## Build and test

```powershell
dotnet restore SlowFlow.sln
dotnet build SlowFlow.sln --no-restore
dotnet test SlowFlow.sln --no-build
pio run --project-dir firmware
pio test --project-dir firmware --environment portenta_h7_m7 --without-uploading --without-testing
```

The last command compiles the embedded test image without requiring connected
hardware. Hardware test execution belongs to the relevant firmware phase.

Architecture boundaries, terminology, phase status, and accepted limitations
are recorded under [`docs/`](docs/).
