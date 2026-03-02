param(
    [string]$Version = "2.2.1",
    [string]$ApiKey = "CHANGE_ME_SECRET_KEY"
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $root "..\..")
$packageDir = Join-Path $root "site_upload_package"
$firmwareDir = Join-Path $packageDir "belom-firmware"
$pluginDir = Join-Path $packageDir "wp-plugin"

New-Item -ItemType Directory -Force -Path $packageDir, $firmwareDir, $pluginDir | Out-Null

$bootSrc = Join-Path $projectRoot "build\bootloader\bootloader.bin"
$partSrc = Join-Path $projectRoot "build\partition_table\partition-table.bin"
$appSrc  = Join-Path $projectRoot "build\viewe_knob_touch.bin"

if (!(Test-Path $bootSrc) -or !(Test-Path $partSrc) -or !(Test-Path $appSrc)) {
    throw "Build binaries not found. Run 'idf.py build' first."
}

$bootDst = Join-Path $firmwareDir "bootloader.bin"
$partDst = Join-Path $firmwareDir "partition-table.bin"
$appDst  = Join-Path $firmwareDir "viewe_knob_touch.bin"

Copy-Item $bootSrc $bootDst -Force
Copy-Item $partSrc $partDst -Force
Copy-Item $appSrc  $appDst  -Force

$hBoot = (Get-FileHash $bootDst -Algorithm SHA256).Hash.ToLower()
$hPart = (Get-FileHash $partDst -Algorithm SHA256).Hash.ToLower()
$hApp  = (Get-FileHash $appDst  -Algorithm SHA256).Hash.ToLower()

$manifest = @{
    latest = $Version
    base_url = "https://admin.belom.eu/wp-json/belom-flasher/v1/download/"
    firmwares = @(
        @{
            version = $Version
            chip = "esp32s3"
            images = @(
                @{ name = "bootloader"; offset = "0x0"; url = "bootloader.bin"; sha256 = $hBoot }
                @{ name = "partition-table"; offset = "0x8000"; url = "partition-table.bin"; sha256 = $hPart }
                @{ name = "app"; offset = "0x10000"; url = "viewe_knob_touch.bin"; sha256 = $hApp }
            )
        }
    )
}

$manifestPath = Join-Path $firmwareDir "manifest.json"
$manifest | ConvertTo-Json -Depth 10 | Set-Content $manifestPath -Encoding UTF8

$pluginPath = Join-Path $pluginDir "belom-flasher-endpoint.php"
$pluginCode = @"
<?php
/**
 * Plugin Name: BEL0M Flasher Endpoint
 * Description: Auth-protected firmware manifest + file download endpoint for BEL0M Flasher.
 * Version: 1.0.0
 */

if (!defined('ABSPATH')) { exit; }

add_action('rest_api_init', function () {
    register_rest_route('belom-flasher/v1', '/manifest', [
        'methods' => 'GET',
        'callback' => 'belom_flasher_manifest_cb',
        'permission_callback' => 'belom_flasher_auth_cb',
    ]);
    register_rest_route('belom-flasher/v1', '/download/(?P<file>[a-zA-Z0-9._-]+)', [
        'methods' => 'GET',
        'callback' => 'belom_flasher_download_cb',
        'permission_callback' => 'belom_flasher_auth_cb',
    ]);
});

function belom_flasher_auth_cb(WP_REST_Request `$request) {
    `$server_key = get_option('belom_flasher_key', '$ApiKey');
    `$client_key = `$request->get_header('x-belom-key');
    if (!`$client_key) {
        `$auth = `$request->get_header('authorization');
        if (stripos(`$auth, 'Bearer ') === 0) {
            `$client_key = substr(`$auth, 7);
        }
    }
    if (!`$client_key) { return false; }
    return hash_equals((string)`$server_key, (string)`$client_key);
}

function belom_flasher_manifest_cb() {
    `$upload = wp_upload_dir();
    `$manifest = trailingslashit(`$upload['basedir']) . 'belom-firmware/manifest.json';
    if (!file_exists(`$manifest)) {
        return new WP_Error('manifest_missing', 'Manifest not found', ['status' => 404]);
    }
    `$json = file_get_contents(`$manifest);
    `$data = json_decode(`$json, true);
    if (!is_array(`$data)) {
        return new WP_Error('manifest_invalid', 'Manifest invalid', ['status' => 500]);
    }
    return rest_ensure_response(`$data);
}

function belom_flasher_download_cb(WP_REST_Request `$request) {
    `$file = basename(`$request['file']);
    `$upload = wp_upload_dir();
    `$path = trailingslashit(`$upload['basedir']) . 'belom-firmware/' . `$file;
    if (!file_exists(`$path)) {
        return new WP_Error('file_missing', 'Firmware file not found', ['status' => 404]);
    }
    header('Content-Type: application/octet-stream');
    header('Content-Length: ' . filesize(`$path));
    header('Content-Disposition: attachment; filename="' . `$file . '"');
    readfile(`$path);
    exit;
}
"@
Set-Content -Path $pluginPath -Value $pluginCode -Encoding UTF8

$readme = @"
SITE UPLOAD PACKAGE
===================

1) Upload firmware files:
   - Copy folder: belom-firmware
   - Destination on server: wp-content/uploads/belom-firmware/

2) Install WP plugin:
   - Copy file: wp-plugin/belom-flasher-endpoint.php
   - Destination: wp-content/plugins/belom-flasher-endpoint.php
   - Activate plugin in WP Admin.

3) Set API key:
   - WP option name: belom_flasher_key
   - Must match key in flasher app (or BELOM_FLASHER_KEY env var).

4) Endpoint:
   - Manifest: /wp-json/belom-flasher/v1/manifest
   - File: /wp-json/belom-flasher/v1/download/<filename>
"@
Set-Content -Path (Join-Path $packageDir "README_UPLOAD.txt") -Value $readme -Encoding UTF8

Write-Host "Site package created:" -ForegroundColor Green
Write-Host $packageDir
