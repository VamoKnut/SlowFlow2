# SlowFlow terminology

The full definitions in the requirement specification are authoritative. Code,
database names, APIs, MQTT messages, tests, and documentation use the following
terms consistently.

* `Drain` is the physical hydraulic installation, including its telescopic
  mechanism and local datum.
* `DrainNode` is the electronic hardware and microcontroller assembly installed
  at a drain.
* `NodeIdentifier` is the stable external identifier used in application data
  and MQTT, for example `DN-000123`. It is distinct from the backend GUID key.
* `DrainDatumElevationNN2000` maps the drain's local zero datum to absolute roof
  geometry.
* `DrainCrestPositionMm` is the measured telescopic crest elevation relative to
  the drain datum.
* `WaterLevelMm` is measured relative to the same drain datum.
* `DesiredState` is requested backend or automation state.
* `ActualState` is state reported by the drain node. Desired state is never
  presented as confirmed actual state before acknowledgement or status.
* `ArduinoPortentaH7` is the only MVP board type.
* `SlowFlowShieldRev1` is the only MVP shield profile.

The Norwegian operational term *sluknode* maps to `DrainNode`; English code
identifiers remain authoritative.
