# Technical debt and accepted MVP limitations

This register documents deliberate limitations. It does not override safety
requirements or authorize implementation beyond the current phase.

## Device and MQTT security

Version 1 intentionally uses MQTT without TLS and shares one broker username
and password across drain nodes. Per-device credentials, certificates, secure
elements, topic ACLs, credential rotation, and signed commands remain future
security work. MQTT transport and authentication must remain isolated so these
controls can be added later.

## OTA

The concrete Portenta H7 OTA library, bootloader and flash layout, authenticity
mechanism, rollback strategy, and key provisioning remain deferred until the
Phase 14 technical prototype.

## Firmware language level

The pinned Arduino Mbed framework currently builds in GNU C++14 mode. Forcing
C++17 causes an upstream framework and standard-library macro conflict when
U8g2 and Arduino interfaces are compiled together. SlowFlow project code shall
remain C++14-compatible until a tested framework or toolchain upgrade resolves
that conflict.

## Backup policy

The exact production backup frequency, retention, off-host copy policy, and
restore-test interval remain deployment decisions for Phase 15.

## Scale

The modular monolith targets the specified MVP scale of approximately 100
active drain nodes. Multitenancy, microservice decomposition, and generalized
enterprise scaling infrastructure are intentionally outside Version 1.
