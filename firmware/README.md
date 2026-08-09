# SlowFlow firmware

The firmware targets the Arduino Portenta H7 M7 core with the
`SlowFlowShieldRev1` profile and uses PlatformIO as the primary toolchain.

The dependency baseline follows the compatible libraries already proven in
the sibling HASTIG project. SlowFlow does not modify or build from HASTIG
sources.

The pinned Arduino Mbed framework uses its default GNU C++14 mode. Forcing
C++17 currently breaks framework and U8g2 compilation through an upstream
`abs` macro conflict, so project code shall remain C++14-compatible until the
firmware toolchain is deliberately upgraded and reverified.

Build the firmware from the repository root:

```powershell
pio run --project-dir firmware
```

Compile the embedded test image without requiring connected hardware:

```powershell
pio test --project-dir firmware --environment portenta_h7_m7 --without-uploading --without-testing
```

Execution on a physical Portenta board will be added with hardware-facing
tests in the relevant firmware phase.
