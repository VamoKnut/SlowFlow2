# SlowFlow Requirement Specification v1

**Status:** Consolidated MVP draft for code generation  
**Date:** 2026-08-06  
**Language:** English  

## 1. Purpose

SlowFlow is a system for automatic control of roof drains in order to retain rainwater temporarily and reduce peak loading on downstream stormwater infrastructure.

Version 1 shall be implemented as an MVP and simple production prototype. It shall demonstrate the technical concept in real pilot installations and provide a practical foundation for later commercialization and scaling.

The system includes:

- firmware for a drain node based on Arduino Portenta H7;
- an ASP.NET Core backend;
- an authenticated Blazor web application;
- public read-only insight pages;
- MQTT communication between backend and drain nodes;
- SQL Server persistence;
- read-only and automation REST APIs;
- OTA firmware management.

Hardware design is outside the software scope, but the firmware shall support the agreed physical interfaces and safety behavior.

## 2. MVP principles

The implementation shall prefer simple, explicit, maintainable solutions over generalized frameworks and speculative extensibility.

Priority order:

1. Safe physical control of the drain.
2. Robust local operation without backend connectivity.
3. Correct engineering values and persistent configuration.
4. Understandable diagnostics and recovery.
5. Simple code and deployment.
6. Operational web functionality.
7. Future scalability.

The MVP shall avoid:

- microservices;
- CQRS frameworks;
- event sourcing;
- generic plugin frameworks;
- a general-purpose GIS platform;
- full multitenancy;
- enterprise-grade device security;
- abstractions created only for hypothetical future requirements.

Known security and scalability limitations shall be documented as technical debt.

## 3. Technology stack

### 3.1 Backend

- .NET 10
- ASP.NET Core
- Blazor Web App using interactive server rendering
- Entity Framework Core
- Existing external SQL Server
- ASP.NET Core Identity
- Docker deployment for the application
- Existing external MQTT broker

The backend shall be implemented as one modular monolith.

### 3.2 Firmware

- Arduino Portenta H7
- Custom SlowFlow shield
- C++/Arduino-compatible firmware environment
- Persistent local configuration and telemetry buffer
- Wi-Fi and 4G connectivity
- MQTT application protocol

### 3.3 Web security

The authenticated web application and REST APIs shall be exposed over HTTPS, normally through a reverse proxy. Publicly trusted certificates such as Let's Encrypt are recommended for public deployments. An internal CA may be used in controlled internal environments.

## 4. System context

```text
Operator / Viewer / Administrator
              |
              | HTTPS
              v
      Reverse proxy
              |
              v
  SlowFlow ASP.NET Core application
      |          |          |
      |          |          +-- Firmware file storage
      |          +------------- External MQTT broker
      +------------------------ External SQL Server
                                 |
                                 | Wi-Fi / 4G
                                 v
                            Drain nodes
```

The backend may be unavailable without immediately preventing local drain control. Each drain node shall continue using its last valid local configuration, normal mode, and setpoints.

## 5. Terminology

### 5.1 Drain

`Drain` is the physical hydraulic installation, including the telescopic drain mechanism and its local datum.

### 5.2 Drain node

`DrainNode` is the electronic hardware and microcontroller assembly installed at a drain. In Norwegian operational terminology this is a *sluknode*.

### 5.3 Drain datum

The local `0 mm` datum is the roof surface elevation at the drain and the lowest level from which runoff toward the drain can occur.

`DrainDatumElevationNN2000` maps the local datum to absolute roof geometry.

### 5.4 Drain crest position

`DrainCrestPositionMm` is the measured elevation of the telescopic drain crest relative to the drain datum.

It may be negative when the crest is below the roof-surface datum.

### 5.5 Water level

`WaterLevelMm` is measured relative to the same drain datum.

It may be slightly negative where local construction details retain a shallow pool below the runoff datum. Negative values within configured limits are valid.

### 5.6 Desired and actual state

- **Desired state:** mode and setpoints requested by backend or automation.
- **Actual state:** state reported by the drain node.

The web application and APIs shall never treat desired state as confirmed actual state before acknowledgement or status reporting.

## 6. Users and authorization

Version 1 shall use three roles.

### 6.1 System Administrator

May:

- perform all Operator actions;
- manage users and roles;
- manage global system settings;
- manage API clients and credentials;
- manage firmware releases and OTA deployments.

### 6.2 Operator

May:

- manage regions, buildings, roofs, drains, and installations;
- edit geometry;
- change normal operating mode and setpoints;
- issue Flush;
- perform calibration and commissioning;
- edit operational hardware configuration;
- acknowledge alarms.

Operators shall not manage users, global system security, or firmware releases.

### 6.3 Viewer

May view operational data, alarms, trends, assets, and history but may not issue commands or change configuration.

## 7. Asset and geometry domain model

```text
Region
└── Building
    └── Roof
        └── DrainCatchmentArea
            ├── CatchmentVertex
            ├── CatchmentEdge
            ├── CatchmentPlane
            └── Drain
                └── DrainNodeInstallation
                    └── DrainNode
```

### 7.1 Region

```text
Region
- Id
- Name
- Description
- IsActive
```

### 7.2 Building

```text
Building
- Id
- RegionId
- Name
- Address
- Description
- IsActive
```

### 7.3 Roof

```text
Roof
- Id
- BuildingId
- Name
- Description
- CoordinateReferenceSystem
- IsPubliclyVisible
- PublicSlug
- IsActive
```

### 7.4 Drain catchment area

A `DrainCatchmentArea` represents one natural roof catchment where every raindrop drains to exactly one drain at the lowest point.

Cross-catchment overflow caused by raising a telescopic drain is outside the initial calculation model.

```text
DrainCatchmentArea
- Id
- RoofId
- DrainId
- Name
- Description
- HorizontalAreaSquareMetres
- IsActive
```

### 7.5 Catchment vertex

```text
CatchmentVertex
- Id
- DrainCatchmentAreaId
- Name
- CoordinateX
- CoordinateY
- SurfaceElevationNN2000
```

Coordinates are normally in a projected CRS such as UTM. Absolute coordinates are authoritative. Relative coordinates may be accepted as input and converted.

### 7.6 Catchment plane

A `CatchmentPlane` is a triangular roof plane defined by three vertices.

```text
CatchmentPlane
- Id
- DrainCatchmentAreaId
- Name
- Vertex1Id
- Vertex2Id
- Vertex3Id
```

### 7.7 Catchment edge

```text
CatchmentEdge
- Id
- DrainCatchmentAreaId
- StartVertexId
- EndVertexId
- BoundaryType
- StartCrestElevationNN2000
- EndCrestElevationNN2000
```

`BoundaryType`:

- `InternalOpen`
- `OuterOpen`
- `OuterRetaining`

For `OuterRetaining`, start and end crest elevations are stored on the edge. Relative wall heights may be entered but shall be converted to absolute NN2000 crest elevations.

### 7.8 Drain

```text
Drain
- Id
- Name
- Description
- DrainDatumElevationNN2000
- CoordinateX
- CoordinateY
- MinimumDrainCrestPositionMm
- MaximumDrainCrestPositionMm
- IsActive
```

The drain vertex in the catchment geometry shall have the same elevation as `DrainDatumElevationNN2000`.

### 7.9 Drain-node installation

A drain node may be replaced without replacing the hydraulic drain or its history.

```text
DrainNodeInstallation
- Id
- DrainId
- DrainNodeId
- InstalledAtUtc
- RemovedAtUtc
- InstalledByUserId
- RemovalReason
- Comment
```

Rules:

- one drain may have at most one active drain-node installation;
- one drain node may be actively installed at at most one drain;
- installation history shall be retained.

## 8. Geometry and hydraulic calculations

### 8.1 Stored water

Stored-water volume shall be calculated from:

- current or historical `WaterLevelMm`;
- `DrainDatumElevationNN2000`;
- triangular catchment geometry;
- horizontal water surface.

Outputs may include:

- `EstimatedRetainedVolumeLitres`;
- `FloodedHorizontalAreaSquareMetres`;
- `RainfallEquivalentMm`;
- `RemainingStorageCapacityLitres`.

Because 1 litre per square metre equals 1 millimetre, rainfall equivalent is:

```text
RainfallEquivalentMm = RetainedVolumeLitres / HorizontalAreaSquareMetres
```

### 8.2 Geometry history

Formal geometry versioning is not required in v1. Geometry changes shall be validated and audit logged. Historical calculations may use current geometry.

### 8.3 Effective hydraulic head

The effective outlet threshold is the higher of the drain datum and the drain crest.

```text
EffectiveOutletLevelMm = max(0, DrainCrestPositionMm)
EffectiveHeadMm = max(0, WaterLevelMm - EffectiveOutletLevelMm)
```

This ensures zero discharge when `WaterLevelMm <= 0`, even if the drain crest is below the datum.

### 8.4 Discharge model

The backend owns the hydraulic relationship between effective head and discharge. Firmware shall not calculate discharge.

The current prototype uses a V-shaped notch. A specific discharge formula and calibration parameters may be added when the physical geometry has been finalized.

## 9. Drain-node hardware model

### 9.1 Board and shield

```text
BoardType = ArduinoPortentaH7
ShieldType = configured SlowFlow shield profile
```

The custom shield provides interfaces for:

- water-level sensor;
- drain-crest position sensor;
- external temperature sensor;
- actuator enable and direction outputs;
- digital overcurrent input;
- display;
- keypad;
- communication peripherals.

`ShieldType` determines concrete pin mappings, buses, drivers, and optional peripherals. Firmware shall reject unsupported shield types.

### 9.2 Actuator

The telescopic drain is driven by a linear actuator controlled by:

- actuator enable/on-off output;
- direction up/down output.

The actuator has built-in end stops.

The absolute drain-crest position sensor is the authoritative source of actual position.

### 9.3 Digital overcurrent input

Overcurrent is exposed by hardware only as a digital true/false state. There is no analog current measurement.

```text
ActuatorOvercurrentInputConfiguration
- Enabled
- ActiveLevel
- ConfirmationTimeMs
```

When the input becomes active:

1. actuator output is disabled immediately;
2. `ActuatorOvercurrent` is raised if the signal remains active for the confirmation period;
3. movement remains blocked while the input is active;
4. no current engineering value or amperage is reported.

### 9.4 Unsupported direction detection

`ActuatorWrongDirection` shall not exist. The hardware cannot reliably determine this fault condition.

## 10. Common sensor model

Water level, drain-crest position, and local external temperature shall use the same conceptual structure.

```text
Hardware interface
→ Raw value
→ Scaling and calibration
→ Filtering
→ Engineering value
→ Plausibility and freshness validation
→ Quality state
```

### 10.1 Common configuration

```text
SensorConfiguration
- Enabled
- InterfaceType
- EngineeringUnit
- MinimumPlausibleEngineeringValue
- MaximumPlausibleEngineeringValue
- StaleMeasurementTimeoutSeconds
- FilterConfiguration
- CalibrationConfiguration
- InterfaceConfiguration
- CalibrationPerformedAtUtc
```

Plausibility limits shall primarily be expressed in engineering units meaningful to operators.

Raw-value checks may remain internal driver diagnostics for conditions such as ADC saturation, disconnected input, or invalid Modbus data.

### 10.2 Common measurement

```text
SensorMeasurement
- EngineeringValue
- RawValue
- ObservedAtUtc
- Quality
```

`SensorQuality`:

- `Valid`
- `Disabled`
- `Stale`
- `Implausible`
- `InterfaceFault`
- `CalibrationInvalid`

### 10.3 Analog sensors

Analog sensors are treated as generic ADC values. The software shall not encode assumptions about 4–20 mA or a physical measurement principle.

```text
AnalogSensorConfiguration
- RawValueAtReferencePoint1
- EngineeringValueAtReferencePoint1
- RawValueAtReferencePoint2
- EngineeringValueAtReferencePoint2
- AdcResolutionBits
```

Typical ADC resolution may be 10 or 12 bits, but scaling shall use observed calibration values rather than assuming full-scale constants.

### 10.4 Modbus sensors

```text
ModbusSensorConfiguration
- SlaveAddress
- BaudRate
- Parity
- RegisterAddress
- RegisterDataType
- RegisterScaleFactor
- RegisterOffset
```

Control logic shall consume the same validated engineering measurement regardless of interface.

### 10.5 Calibration

```text
SensorCalibrationConfiguration
- ReferencePoint1
- ReferencePoint2
- Offset
- CalibrationPerformedAtUtc
```

Each reference point contains:

```text
SensorCalibrationPoint
- RawValue
- EngineeringValue
```

One-point calibration normally updates offset. Full commissioning calibration defines or updates both reference points.

## 11. Operating modes and control authority

### 11.1 Normal operating modes

```text
NormalOperatingMode
- FullyLowered
- FixedCrestPosition
- ControlledHead
```

#### Fully lowered

Target position is `MinimumDrainCrestPositionMm`, which may be negative relative to the drain datum.

#### Fixed crest position

Backend provides `TargetDrainCrestPositionMm`.

#### Controlled head

Backend provides `TargetEffectiveHeadMm`.

Firmware derives a crest-position target from measured water level and constrains it to the configured physical range.

### 11.2 Control authority

```text
ControlAuthority
- Automatic
- LocalManual
```

`LocalManual`:

- blocks backend movement commands, including Flush;
- suspends automatic control and safe lowering;
- permits local Up and Down hold-to-run movement;
- retains hardware interlocks;
- shall remain persistent across reset;
- shall restart with actuator stopped.

The backend may request transition to automatic control. This shall display a local warning and configurable countdown and shall not cause immediate movement.

## 12. Temporary control overrides

```text
TemporaryOverride
- None
- SafeLowering
- Flush
- CalibrationHold
```

OTA is a separate maintenance process and not a normal control override.

### 12.1 Priority

```text
1. Actuator hardware interlocks
2. Local manual control
3. Calibration hold
4. Safe lowering
5. Flush
6. Normal operating mode
```

### 12.2 Safe lowering

Safe lowering is activated by one or more reasons:

- water-level sensor fault;
- active temperature-source fault;
- frost protection;
- long communication loss.

All reasons use the same configured lowering rate.

Safe lowering shall be implemented as a time-driven sequence of fixed crest-position targets, not as average-speed regulation.

```text
SafeLoweringTargetMm =
  max(MinimumDrainCrestPositionMm,
      PreviousTargetMm - StepSizeMm)
```

A typical initial value is 10 mm per minute, configurable.

### 12.3 Flush

Flush shall:

1. preserve previous normal mode and setpoints;
2. validate minimum water availability;
3. move rapidly to the mechanical minimum;
4. remain there for a configurable dwell time, approximately 0–30 seconds;
5. restore previous normal mode and setpoints.

Flush shall be rejected or interrupted by:

- local manual mode;
- blocking actuator fault;
- safe lowering;
- calibration;
- OTA;
- command expiry;
- insufficient water.

Flush shall not automatically resume after reboot.

### 12.4 Calibration hold

Calibration shall stop actuator movement and suspend normal control until the calibration operation completes or fails.

## 13. Actuator controller and interlocks

### 13.1 Movement state

```text
ActuatorMovementState
- Stopped
- MovingUp
- MovingDown
```

Only `ActuatorDriver` may write directly to actuator output pins.

### 13.2 Position controller

The controller shall use absolute position feedback and a simple staged approach:

- full-power movement far from target;
- reduced PWM or short full-power pulses near target;
- stopped inside deadband.

PWM shall only be used if verified with the installed actuator driver. Pulsed on/off control is an acceptable MVP alternative.

### 13.3 Blocking actuator faults

- `DrainCrestPositionSensorFault`
- `ActuatorNoMovement`
- `ActuatorMovementTimeout`
- `ActuatorOvercurrent`
- `ActuatorPositionOutsideLimits`
- `ActuatorDriverFault`
- invalid position calibration

A blocking fault shall disable movement immediately and prevent manual, Flush, safe-lowering, and normal control movement.

## 14. Temperature and frost protection

### 14.1 Temperature source modes

```text
TemperatureSourceMode
- Local
- Remote
- Disabled
```

- `Local`: outdoor sensor near the roof and telescopic drain.
- `Remote`: normalized backend temperature from one selected source.
- `Disabled`: no temperature fault and no frost protection based on that sensor.

There is no automatic fallback between source modes.

### 14.2 Remote sources

A remote source may originate from:

- external API;
- external MQTT feed;
- local sensor of another SlowFlow drain node.

Backend shall normalize value, source identity, observation time, freshness, and quality before sending it to assigned drain nodes.

### 14.3 Frost behavior

A configurable trigger around 2 °C is the initial expectation. Trigger, release threshold, delay, and hysteresis shall be configurable.

In automatic control, frost protection activates safe lowering. In local manual mode, the condition is displayed and reported but automatic movement is suspended.

If an enabled selected source is invalid, stale, missing, or implausible, a temperature-source fault is raised and safe lowering is activated.

## 15. Local user interface

The drain node shall provide a local display and four arrow keys.

The display shall show:

- water level relative to drain datum;
- head relative to drain crest;
- actual drain-crest position;
- operating mode;
- communication state;
- active fault state.

Manual movement:

- hold Up to move upward;
- hold Down to move downward;
- release to stop immediately;
- Left exits manual mode.

A separate confirmed menu action shall move the drain automatically to the mechanical minimum.

The project shall also evaluate a local Wi-Fi access point and web portal for provisioning. A QR code may link to the local portal where display capability permits. It shall not expose a permanent secret.

## 16. Connectivity and device identity

### 16.1 Network interfaces

Wi-Fi and 4G may be active simultaneously.

- Wi-Fi is preferred.
- Automatic failover to 4G shall occur when Wi-Fi is unavailable or unhealthy.
- Return to Wi-Fi shall occur after a configurable stable period.
- Rapid interface flapping shall be avoided.
- MQTT and application logic shall be independent of the active interface.

### 16.2 Mobile settings

Mobile configuration may include APN, username, password, and SIM PIN where required.

### 16.3 Device identity

Device identity shall primarily use a hardware-provided identifier such as IMEI or an equivalent immutable hardware ID.

A separate stable `NodeIdentifier`, such as `DN-000123`, shall be used in application data and MQTT topics.

## 17. Time synchronization

The primary authoritative time source shall be the backend over MQTT.

After boot, the drain node sends boot/status. Backend responds with UTC time, normally in `ServerAck`.

The node updates its clock only when the difference exceeds a configurable threshold, initially around 10 seconds.

Mobile-network time may be used as preliminary time but is not relied upon. NTP availability is not assumed.

Control loops and timeouts shall use monotonic time. Samples and events shall use UTC where available.

`TimeQuality`:

- `Unsynchronized`
- `Estimated`
- `MobileNetworkSynchronized`
- `BackendSynchronized`

Samples created before synchronization should retain boot-relative monotonic timing and be resolved to UTC after synchronization where possible.

## 18. Telemetry and local buffering

### 18.1 Stored measurements

Backend shall persist selected engineering time series:

- `WaterLevelMm`;
- `DrainCrestPositionMm`;
- `ExternalTemperatureC` when available.

Calculated discharge and retained volume are derived and are not authoritative stored time series.

### 18.2 Sampling

A sample shall be recorded:

- at the configured periodic anchor interval, initially around 10 minutes;
- when water level changes more than a configurable deadband, initially around 3 mm;
- when drain-crest position changes more than a configurable deadband, initially around 3 mm.

`TriggerReason` shall not be stored.

### 18.3 Buffer

Each drain node shall retain samples locally for at least 10 days under the expected sampling profile.

The store shall be a persistent bounded ring buffer with:

- unique persistent sample sequence;
- batched upload;
- acknowledgement-based deletion;
- explicit overflow reporting;
- idempotent backend ingestion.

### 18.4 Status reporting

A complete status message shall normally be published at a configurable interval, initially around 10 minutes.

Fault activation and important state changes shall be reported immediately rather than waiting for the periodic interval.

## 19. MQTT security scope

Version 1 intentionally uses simplified MQTT security:

- no TLS;
- all drain nodes share one username and password.

The architecture shall isolate MQTT transport and authentication so later versions can add:

- TLS;
- per-device credentials;
- certificates;
- secure-element-backed keys;
- topic ACLs;
- credential rotation;
- signed commands.

MQTT credentials shall not be logged or exposed through status APIs.

## 20. MQTT protocol

### 20.1 Topics

```text
slowflow/v1/nodes/{drainNodeId}/boot
slowflow/v1/nodes/{drainNodeId}/status
slowflow/v1/nodes/{drainNodeId}/telemetry
slowflow/v1/nodes/{drainNodeId}/event
slowflow/v1/nodes/{drainNodeId}/ack

slowflow/v1/nodes/{drainNodeId}/command
slowflow/v1/nodes/{drainNodeId}/configuration
slowflow/v1/nodes/{drainNodeId}/desired-state
slowflow/v1/nodes/{drainNodeId}/remote-temperature
slowflow/v1/nodes/{drainNodeId}/server-ack
```

### 20.2 MQTT behavior

Recommended MVP settings:

- QoS 1;
- no retained physical commands;
- clean sessions;
- node-owned telemetry buffer;
- reconnect and resubscribe automatically.

### 20.3 Common envelope

```json
{
  "protocolVersion": 1,
  "messageType": "FullStatus",
  "messageId": "...",
  "drainNodeId": "DN-000123",
  "bootId": "...",
  "sequenceNumber": 1042,
  "createdAtUtc": "2026-08-06T12:30:00Z",
  "timeQuality": "BackendSynchronized",
  "payload": {}
}
```

### 20.4 Message types

- `Boot`
- `ServerAck`
- `FullStatus`
- `TelemetryBatch`
- `AlarmActivated`
- `AlarmCleared`
- `OperationalEvent`
- `ConfigurationSnapshot`
- `DesiredState`
- `RemoteTemperature`
- `Command`
- `CommandAck`

### 20.5 Command requirements

Physical-action commands shall include:

- unique `CommandId`;
- issue time;
- expiry time;
- idempotency handling;
- acknowledgement state.

Duplicate command IDs shall not repeat physical actions. The previous result shall be returned.

Command states:

- `Received`
- `Accepted`
- `Executing`
- `Completed`
- `Rejected`
- `Failed`
- `Cancelled`
- `Expired`

### 20.6 Versioned configuration and desired state

Hardware configuration is transmitted as a complete versioned snapshot.

Desired state is separately versioned and includes normal operating mode and relevant setpoint.

Both shall include expected previous version to prevent stale overwrite.

## 21. Backend architecture

The MVP backend shall use three main projects:

```text
SlowFlow.Web
SlowFlow.Core
SlowFlow.Infrastructure
```

### 21.1 SlowFlow.Web

Contains:

- Blazor UI;
- Integration and Automation APIs;
- authentication and authorization;
- MQTT hosted service;
- background services;
- health endpoints;
- public routes.

### 21.2 SlowFlow.Core

Contains:

- entities and enums;
- typed configuration models;
- protocol DTOs;
- application services;
- validation;
- calculations;
- API DTOs and stable error codes.

### 21.3 SlowFlow.Infrastructure

Contains:

- `SlowFlowDbContext`;
- SQL Server entity mappings and migrations;
- ASP.NET Core Identity persistence;
- firmware file storage;
- API credential storage;
- concrete external providers.

A generic repository layer is not required. EF Core may be used directly by clear application services.

## 22. Backend application services

Core services include:

- `MqttConnectionService`
- `MqttMessageRouter`
- `TelemetryIngestionService`
- `AlarmService`
- `OperationalEventService`
- `DrainControlService`
- `DrainNodeCommandService`
- `DrainNodeConfigurationService`
- `CalibrationService`
- `RemoteTemperatureService`
- `StoredWaterCalculationService`
- `DischargeCalculationService`
- `PublicRoofSnapshotService`

Important processing rule:

```text
Persist database state
→ Commit
→ Publish MQTT
→ Record publishing result
```

An MQTT message that matters for delivery shall be represented in a simple database outbox:

```text
PendingMqttMessage
- Id
- DrainNodeId
- Topic
- PayloadJson
- CreatedAtUtc
- PublishedAtUtc
- AttemptCount
- LastAttemptAtUtc
- LastError
- ExpiresAtUtc
```

Expired physical messages shall never be published later.

## 23. Main backend persistence model

The database shall include at least:

- regions, buildings, roofs, catchments, geometry, and drains;
- drain nodes and installation history;
- versioned hardware configuration snapshots;
- versioned desired states;
- current status snapshot;
- telemetry samples;
- alarms;
- operational events;
- audit records;
- commands;
- pending MQTT messages;
- temperature sources and assignments;
- firmware releases and deployments;
- API clients.

### 23.1 Telemetry sample

```text
DrainNodeTelemetrySample
- Id
- DrainNodeId
- DrainNodeInstallationId
- SampleSequence
- ObservedAtUtc
- ReceivedAtUtc
- BootId
- WaterLevelMm
- WaterLevelQuality
- DrainCrestPositionMm
- DrainCrestPositionQuality
- ExternalTemperatureC
- ExternalTemperatureQuality
- HardwareConfigurationVersion
- TimeQuality
```

Unique identity:

```text
DrainNodeId + SampleSequence
```

### 23.2 Configuration storage

The full hardware configuration may be stored as strongly typed JSON in a versioned snapshot entity to reduce table complexity.

### 23.3 Current status

Only the latest full status snapshot is stored as current status. Historical state is represented by telemetry, commands, alarms, events, and audit data.

## 24. Alarm model

Alarms represent actual faults, not normal operating states.

Main alarm types:

### Sensors and configuration

- `WaterLevelSensorFault`
- `DrainCrestPositionSensorFault`
- `LocalTemperatureSensorFault`
- `RemoteTemperatureSourceFault`
- `HardwareConfigurationFault`
- `CalibrationInvalid`
- `PersistentStorageFault`
- `TelemetryBufferOverflow`

### Actuator

- `ActuatorNoMovement`
- `ActuatorMovementTimeout`
- `ActuatorOvercurrent`
- `ActuatorPositionOutsideLimits`
- `ActuatorDriverFault`

`ActuatorWrongDirection` shall not exist.

### Backend and communication

- `DrainNodeCommunicationLost`
- `LongCommunicationLoss`
- `OtaFailure`

Frost protection is a runtime condition and operational event, not an alarm, unless the selected temperature source is faulty.

Alarm acknowledgement means the alarm has been seen. It shall not clear the fault or bypass firmware interlocks.

## 25. Operational events and audit

Operational events include:

- boot and restart;
- manual-mode transitions;
- safe-lowering lifecycle;
- Flush lifecycle;
- calibration lifecycle;
- configuration and desired-state application;
- network-interface change;
- OTA lifecycle.

Audit records shall capture human and automation actions, including:

- actor identity;
- source interface;
- action;
- affected entity;
- old and new values where appropriate;
- result and timestamp.

Routine telemetry shall not be written to audit history.

## 26. Read-only Integration API

Base route:

```text
/api/integration/v1
```

The API is intended for Grafana and ordinary third-party consumers and shall expose GET operations only.

### 26.1 Endpoints

```http
GET /regions

GET /buildings
GET /regions/{regionId}/buildings

GET /roofs
GET /regions/{regionId}/roofs
GET /buildings/{buildingId}/roofs

GET /drains
GET /regions/{regionId}/drains
GET /buildings/{buildingId}/drains
GET /roofs/{roofId}/drains

GET /drains/{drainId}
GET /drains/{drainId}/timeseries?from=...&to=...&metrics=...
```

When no parent ID is supplied, all entities of that type are returned, subject to configured result limits.

### 26.2 Ancestor IDs

- Building DTO includes `RegionId`.
- Roof DTO includes `BuildingId` and `RegionId`.
- Drain DTO includes `RoofId`, `BuildingId`, and `RegionId`.

### 26.3 Drain data

Current drain data shall include:

- actual operating mode;
- desired operating mode and state version;
- control authority;
- temporary override;
- water level;
- drain-crest position;
- estimated retained water;
- data observation time;
- online and freshness state;
- blocking condition where relevant.

Historical time series may include water level, drain-crest position, temperature, and calculated retained water for the selected period.

The API shall not expose write operations, commands, calibration, configuration, private credentials, or sensitive hardware identifiers.

## 27. Automation API

Base route:

```text
/api/automation/v1
```

This API is intended for a trusted external optimization system using weather forecasts and downstream stormwater capacity.

### 27.1 Permitted write actions

- change normal operating mode;
- change relevant setpoint;
- issue Flush.

It shall not manage:

- hardware configuration;
- calibration;
- users;
- alarms;
- geometry;
- firmware or OTA;
- network settings;
- safety limits.

### 27.2 Endpoints

```http
POST /drains/{drainId}/desired-state
POST /drains/{drainId}/commands/flush
GET  /commands/{commandId}
GET  /drains/{drainId}/desired-state
```

### 27.3 Credential model

One machine credential may hold multiple scopes and may call both Integration and Automation API routes.

Typical scopes:

- `integration.read`
- `automation.status.read`
- `automation.desired-state.write`
- `automation.flush.write`

A Grafana credential normally has only `integration.read`.

### 27.4 Concurrency and idempotency

Automation desired-state requests shall include:

- `requestId`;
- `expectedDesiredStateVersion`;
- expiry for pending publication;
- reason text.

If the expected version differs from current state, return `409 Conflict` without overwriting newer operator or automation decisions.

The combination of client identity and `requestId` shall be unique. Retried requests shall return the original result and shall not repeat a physical action.

### 27.5 Safety

Automation actions use the same backend services and firmware validation as operator actions. They remain subordinate to:

- local manual mode;
- safety overrides;
- actuator interlocks;
- configuration limits;
- Flush water threshold;
- command expiry.

All automation actions shall be audit logged using the machine-client identity.

## 28. Public insight pages

A roof may be marked publicly visible.

Public pages shall use an explicit snapshot DTO and may show:

- approved roof/building display name;
- simplified operational status;
- estimated retained volume in litres and cubic metres;
- rainfall equivalent in millimetres;
- number of active drains;
- data observation time and freshness.

Public pages shall not expose:

- hardware identifiers;
- network or MQTT data;
- calibration values;
- commands or setpoints;
- detailed alarms;
- raw sensor values;
- user data.

## 29. Web UX requirements

The authenticated application is an expert operational tool and shall use a compact, information-dense visual design.

It shall prioritize:

- compact data tables;
- limited whitespace;
- efficient side-by-side comparison;
- short labels with visible units;
- sticky headers and compact filter bars;
- clear actual-versus-desired-state presentation;
- clear freshness and quality indicators.

Large decorative cards and consumer-style whitespace-heavy layouts shall not dominate operational screens.

Safety-related actions shall still use deliberate confirmation dialogs.

### 29.1 Main navigation

```text
Dashboard
Assets
  Regions
  Buildings
  Roofs
  Drains
Alarms
Operations
Firmware
System
```

### 29.2 Core pages

- system dashboard;
- region, building, roof, and drain lists and details;
- drain overview and trends;
- drain control;
- alarms and alarm details;
- command and event history;
- typed hardware configuration;
- calibration;
- table-based geometry editor;
- drain-node installation history;
- firmware release and deployment;
- API-client administration.

### 29.3 Drain page

The drain overview shall compactly display:

- identity and hierarchy;
- installed drain node, firmware, and shield;
- current measurements and quality;
- actual state;
- desired state;
- pending application state;
- active alarms and safe-lowering reasons;
- communication freshness;
- relevant actions.

Raw ADC values shall not be shown in ordinary operational views. They may appear in restricted commissioning diagnostics.

Remote actuator jogging shall not be implemented.

## 30. OTA firmware updates

Backend shall manage:

- firmware release metadata;
- board and shield compatibility;
- version and protocol compatibility;
- file, checksum, and cryptographic signature;
- deployment assignments;
- lifecycle status and audit history.

The drain node shall:

1. receive an OTA command/manifest;
2. download over HTTPS using active Wi-Fi or 4G;
3. verify compatibility, checksum, and signature;
4. install to an update partition;
5. reboot;
6. perform a post-boot health check;
7. confirm the image or roll back.

OTA shall not start during:

- actuator movement;
- local manual mode;
- Flush;
- calibration;
- safe lowering;
- blocking hardware fault.

OTA management is restricted to System Administrators.

## 31. Deployment and operations

### 31.1 Topology

- one SlowFlow ASP.NET Core Docker container;
- existing external SQL Server;
- existing external MQTT broker;
- persistent mounted firmware and application-data directories;
- reverse proxy with HTTPS.

SQL Server and the MQTT broker shall not run inside the SlowFlow Docker Compose project.

### 31.2 Docker host preparation

The repository shall contain a deployment guide for preparing a clean Ubuntu Docker host. Host preparation and application deployment shall be documented as separate procedures.

The host-preparation guide shall cover at least:

1. supported Ubuntu version and minimum CPU, memory and disk assumptions;
2. installation or verification of Docker Engine and the Docker Compose plugin;
3. enabling Docker to start automatically after host reboot;
4. creation of an appropriate deployment user and Docker permissions;
5. creation and ownership of persistent host directories;
6. verification of DNS, routing and firewall connectivity to the external SQL Server and MQTT broker;
7. preparation of inbound ports for the reverse proxy and HTTPS;
8. host time synchronization and timezone expectations;
9. container-log configuration and disk-space monitoring;
10. verification commands proving that Docker, Compose, networking and persistent storage are ready before SlowFlow is deployed.

The deployment documentation shall not require SQL Server or MQTT to be installed on the Docker host.

### 31.3 Required deployment files

The repository shall include, at minimum:

```text
deploy/
├── docker-compose.yml
├── .env.example
├── README-DEPLOYMENT.md
├── reverse-proxy/
│   └── example configuration
└── scripts/
    ├── check-host.sh
    ├── deploy.sh
    ├── migrate.sh
    ├── verify.sh
    └── rollback.sh
```

The scripts may remain simple shell scripts. They shall be readable and shall fail with a non-zero exit code when a required step fails.

### 31.4 Persistent directories and permissions

The deployment guide shall define persistent locations for at least:

```text
/opt/slowflow/config
/opt/slowflow/firmware
/opt/slowflow/logs
/opt/slowflow/backups
```

Exact host paths may be changed by deployment configuration, but the guide shall explain:

- which paths must survive container replacement;
- required owner and group;
- required read/write permissions;
- how firmware binaries are backed up;
- how to confirm that the application container can write to its mounted directories.

Database files shall not be stored in these directories because SQL Server is external.

### 31.5 Network preparation

Before deployment, the host-preparation procedure shall verify:

- name resolution for the SQL Server host;
- TCP connectivity to the configured SQL Server port and instance;
- name resolution and TCP connectivity to the MQTT broker;
- outbound HTTPS access required for external services and future OTA downloads where applicable;
- inbound HTTP/HTTPS access to the reverse proxy;
- that the SlowFlow application port is not unnecessarily exposed publicly.

Named SQL Server instances, non-default ports and certificate trust settings shall be documented where relevant to the actual environment.

The application shall be attached to a dedicated Docker network. Only the reverse proxy shall normally expose a public host port. Communication from the reverse proxy to the SlowFlow container may use HTTP on the isolated Docker network.

### 31.6 Configuration and secrets

Non-sensitive defaults may be stored in `appsettings.json`. Environment-specific values shall be supplied through environment variables, mounted configuration files or the selected secret mechanism.

The repository shall include a complete `.env.example` containing placeholder names but no real credentials. Required settings shall include at least:

```text
ASPNETCORE_ENVIRONMENT
ConnectionStrings__SlowFlowDatabase
Mqtt__Host
Mqtt__Port
Mqtt__UseTls
Mqtt__Username
Mqtt__Password
SlowFlow__FirmwareStoragePath
SlowFlow__LogPath
```

The deployment guide shall explain how to protect the real environment file, restrict file permissions and prevent it from being committed to GitHub.

### 31.7 Docker Compose requirements

The Compose definition shall include:

- a fixed application service name;
- an explicit image tag or release version;
- `restart: unless-stopped` or an equivalent restart policy;
- persistent mounts for firmware files and other required application data;
- health-check configuration;
- environment or secret references;
- a dedicated network;
- configurable log rotation;
- no embedded production passwords;
- no SQL Server or MQTT service definitions for the production topology.

The production Compose file shall not use `latest` as the only image version. Deployments shall be reproducible from a known application image tag.

### 31.8 Reverse proxy and HTTPS preparation

The deployment guide shall explain how to connect an existing or new reverse proxy to the SlowFlow container.

It shall cover:

- public hostname and DNS prerequisite;
- TLS certificate installation or automatic certificate renewal;
- forwarding of HTTPS traffic to the internal SlowFlow HTTP endpoint;
- forwarded headers required by ASP.NET Core;
- request-size limits sufficient for firmware upload;
- WebSocket support required by interactive Blazor connections;
- HTTP-to-HTTPS redirect;
- verification of certificate validity and application reachability.

A reverse-proxy example may use one selected product, but SlowFlow shall not depend on that product in application code.

### 31.9 Database migration procedure

EF Core migrations shall be stored in source control and applied deliberately during deployment. Production startup shall not automatically apply arbitrary migrations.

The deployment documentation shall provide a concrete migration command or migration container workflow and shall describe this sequence:

```text
1. Confirm database backup status.
2. Confirm the target application and migration version.
3. Stop or place the application in maintenance mode when required.
4. Run the migration against the external SQL Server.
5. Verify migration success and expected schema version.
6. Start or upgrade the application container.
7. Run post-deployment verification.
```

The migration process shall use the same connection configuration as the application without printing the connection string to logs.

### 31.10 Deployment and upgrade procedure

The deployment guide shall include commands or scripts for:

- authenticating to the selected container registry when required;
- pulling the intended image version;
- validating the Compose configuration;
- applying migrations;
- starting or updating the application;
- checking container state and logs;
- running health and connectivity verification;
- confirming application version after deployment.

A normal upgrade shall preserve all mounted data and shall not replace or recreate the external SQL Server or MQTT broker.

### 31.11 Rollback procedure

Rollback instructions shall cover:

- retaining the previously deployed image tag;
- redeploying the previous application image;
- handling database migrations that are not backward compatible;
- restoring the database when a schema rollback is required;
- verifying health, database access and MQTT connectivity after rollback.

The documentation shall state that container-image rollback alone may be unsafe after an incompatible database migration.

### 31.12 Health checks and post-deployment verification

```text
/health/live
/health/ready
```

Liveness confirms the process is running. Readiness includes SQL Server, expected schema, firmware storage, and required background services. MQTT state shall be reported separately.

The post-deployment verification procedure shall check at least:

1. container is running with the intended image version;
2. `/health/live` succeeds;
3. `/health/ready` succeeds or clearly identifies the unavailable dependency;
4. the application can connect to the external SQL Server;
5. the application can connect to the MQTT broker;
6. the firmware directory is writable;
7. HTTPS and forwarded headers work through the reverse proxy;
8. authenticated login works;
9. no secrets are present in container logs;
10. a controlled application restart preserves configuration and mounted files.

### 31.13 Backup integration

Backup shall cover:

- external SQL Server database;
- firmware binaries;
- application and reverse-proxy configuration;
- MQTT broker configuration;
- required credentials and verification keys.

The Docker guide shall identify which host directories must be included in file backup and shall reference, but not replace, the external SQL Server backup procedure.

The exact database-backup frequency, retention and restore-test interval remain open deployment decisions.

## 32. Non-functional requirements

### 32.1 Expected scale

Initial design target:

- up to approximately 100 active drain nodes;
- status around every 10 minutes;
- event-based samples between periodic samples;
- several years of telemetry;
- a small number of operators and integration clients.

This is a design target, not a contractual capacity guarantee.

### 32.2 Performance

Under expected MVP load:

- ordinary list and status pages should normally load within two seconds;
- backend write requests should normally be persisted within one second, excluding device acknowledgement;
- telemetry processing shall keep pace with normal sustained input;
- API requests shall not block MQTT ingestion.

### 32.3 API limits

Configurable limits shall include:

- maximum entity rows;
- maximum time-series rows;
- maximum time range;
- request body size;
- request timeout;
- per-client rate limits.

Suggested starting limits:

- 10,000 entity rows;
- 50,000 time-series rows;
- 1 MB request body.

### 32.4 Data retention

Retention shall be configurable. Three years of telemetry is a recommended initial default, but the final deployment value remains to be confirmed.

Configuration, calibration, command, alarm, audit, installation, and firmware history should normally be retained for the lifetime of the MVP installation.

### 32.5 Logging

Use structured logging with identifiers such as:

- `DrainNodeId`;
- `DrainId`;
- `CommandId`;
- `MessageId`;
- `BootId`;
- `ConfigurationVersion`;
- `DesiredStateVersion`;
- user or automation-client identity.

Do not log secrets or one information message per telemetry sample.

### 32.6 Error handling

API errors shall include:

- stable error code;
- human-readable message;
- correlation identifier.

Production errors shall not expose stack traces, SQL, credentials, or internal paths.

### 32.7 Code quality

- nullable reference types enabled;
- asynchronous I/O;
- cancellation tokens for long operations;
- thin UI components and API endpoints;
- physical and hydraulic rules in named services;
- validation of all external input;
- no unnecessary interfaces or frameworks;
- comments for non-obvious safety behavior;
- automated tests for critical control and calculation logic.

## 33. Key acceptance criteria

### 33.1 Local autonomy

When MQTT communication is lost, a drain node continues using the last valid mode and setpoints.

### 33.2 Safe boot

Actuator outputs remain disabled until position feedback, calibration, and hardware limits are valid.

### 33.3 Position-sensor fault

When drain-crest position is invalid, actuator movement stops and all new movement is blocked.

### 33.4 Digital overcurrent

When the digital overcurrent input becomes active, actuator output is disabled immediately. No analog current value is calculated or displayed.

### 33.5 Safe lowering

Given a configured step of 10 mm per minute, a qualifying safety condition lowers the temporary target approximately 10 mm each minute until mechanical minimum.

### 33.6 Local manual mode

Backend Flush and movement requests are rejected while local manual mode is active. Manual Up and Down movement occurs only while the corresponding key is held.

### 33.7 Duplicate physical commands

A repeated `CommandId` does not execute the action again and returns the previous acknowledgement state.

### 33.8 Telemetry idempotency

Repeated samples with the same drain-node and sample sequence do not create duplicate database rows.

### 33.9 Desired versus actual

A new desired state remains pending in the UI until device acknowledgement or actual status confirms it.

### 33.10 Automation API

- read-only credentials cannot perform automation writes;
- one scoped machine credential may read Integration API data and perform permitted automation actions;
- stale expected version returns `409 Conflict`;
- duplicate request IDs do not create duplicate desired states or Flush commands;
- local firmware safety rules cannot be bypassed.

### 33.11 Public data

Public pages never expose hardware identifiers, credentials, calibration, commands, detailed private alarms, or raw sensor values.

### 33.12 OTA

Firmware with invalid compatibility, checksum, or signature is not installed. OTA does not start during blocked operational states.

## 34. Recommended implementation phases

1. Repository, solution scaffold, and terminology.
2. Domain model, EF Core mappings, Identity, and initial migration.
3. MQTT protocol DTOs and transport.
4. Boot, status, telemetry, alarms, and idempotent ingestion.
5. Desired state, commands, outbox, and acknowledgements.
6. Firmware shield, sensors, storage, and actuator foundation.
7. Firmware control modes, safe lowering, Flush, and manual control.
8. Compact operational web UI.
9. Integration and Automation APIs.
10. Geometry and hydraulic calculations.
11. Calibration and configuration management.
12. Remote temperature integrations.
13. Public insight pages.
14. OTA.
15. Operational hardening, backup, and deployment verification.

Each phase shall build and test before the next phase begins.

## 35. Code-generation rules

Code generation shall:

- read this specification before producing code;
- implement one phase at a time;
- preserve agreed terminology;
- prefer explicit and simple code;
- compile and run tests after each phase;
- document deliberate MVP limitations;
- never silently change safety behavior.

Code generation shall not:

- introduce microservices, CQRS, or event sourcing;
- expose writes through the Integration API;
- implement remote actuator jogging;
- treat desired state as actual state;
- resume Flush after reboot;
- implement `ActuatorWrongDirection`;
- model overcurrent as an analog engineering sensor;
- distinguish ultrasonic and radar as software configuration;
- expose raw sensor values in ordinary operational UI;
- create schedules on drain nodes.

## 36. Open items before final implementation

The following values or details remain intentionally open and shall be finalized during the relevant phase or pilot commissioning:

1. Exact Portenta build tooling and library versions.
2. Concrete `ShieldType` profiles, pin mappings, and peripheral drivers.
3. Exact actuator tuning values, pulse lengths, deadbands, and timeouts.
4. Final sensor ranges, calibration values, and plausible engineering limits per installation.
5. Final frost trigger, release threshold, delay, and hysteresis.
6. Communication restart threshold and long-loss safe-lowering threshold.
7. Exact telemetry buffer capacity in samples.
8. Whether full Modbus sensor support is required in the first pilot or may begin as a placeholder.
9. Exact V-notch discharge formula and hydraulic calibration parameters.
10. Final telemetry retention period.
11. Final API rate limits, maximum time range, and log retention.
12. Production backup schedule and restore-test frequency.
13. Detailed local display menus and provisioning workflow.
14. OTA bootloader/update-partition capabilities and signing-key provisioning.
15. Exact public location information permitted for each roof.
16. Whether external login providers are needed in addition to local ASP.NET Core Identity accounts.

These open items do not block the initial repository scaffold and domain-model phases.
