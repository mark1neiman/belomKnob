# BEL0M Windows UART Flasher

Windows GUI for ESP UART flashing with server-side firmware delivery.

## What this app does

- Fetches firmware manifest from WP endpoint.
- Sends embedded API key to server on each request.
- Lets user pick only COM port (baud is fixed to `460800`).
- Downloads firmware images.
- Flashes device via `esptool`.
- Shows readiness state and flashing status.
- Runs mini tests after flash.

## Auth model

Requests include:

- header `X-Belom-Key: <key>`
- header `Authorization: Bearer <key>`

Set key in app:

- edit `API_KEY` in [flasher_app.py](./flasher_app.py), or
- set env var `BELOM_FLASHER_KEY` (takes priority).

## Install

```powershell
cd tools/windows_flasher
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
```

## Run

```powershell
python flasher_app.py
```

## Manifest format

Example: [firmware_manifest.example.json](./firmware_manifest.example.json)

Minimal structure:

```json
{
  "latest": "2.2.1",
  "base_url": "https://admin.belom.eu/uploads/fw/",
  "firmwares": [
    {
      "version": "2.2.1",
      "chip": "esp32s3",
      "images": [
        { "name": "bootloader", "offset": "0x0", "url": "bootloader.bin" },
        { "name": "partition-table", "offset": "0x8000", "url": "partition-table.bin" },
        { "name": "app", "offset": "0x10000", "url": "viewe_knob_touch.bin" }
      ]
    }
  ]
}
```

`url` may be absolute or relative (relative is resolved against `base_url`).

## Build EXE

Use spec file (includes black logo asset and default Windows-style icon mode):

```powershell
cd tools/windows_flasher
.venv\Scripts\activate
pyinstaller --noconfirm --clean BelomFlasher.spec
```

EXE output:

`tools/windows_flasher/dist/BelomFlasher.exe`

## Notes

- If key is hardcoded in EXE, it can be extracted by reverse engineering.
- For stronger protection, also validate device/license server-side and return short-lived signed download links/tokens.
