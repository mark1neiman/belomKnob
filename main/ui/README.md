# UI structure

- `main/main.c` - app entrypoint, hardware bring-up, panel init, app startup.
- `main/ui/core/system_core.inc` - settings storage, fan PWM helpers, runtime settings init.
- `main/ui/runtime/ui_runtime.inc` - main/status UI runtime logic, ring updates, sleep/wake flow.
- `main/ui/platform/lvgl_port.inc` - touch + LVGL port init and display invalidation rounder.
- `main/ui/flow/ui_flow.inc` - screen wiring and user interaction flow (single/double/long/knob).
- `main/ui/input/input_devices.inc` - knob/button callbacks and registration.
- `main/ui/input/uart_console.inc` - UART command console.
- `main/ui/screens/**` - per-screen rendering blocks and settings subsections.

This is still a single translation unit (included `.inc` blocks) to keep behavior stable during refactor.
Next step can migrate these blocks to separate `.c/.h` compilation units.
