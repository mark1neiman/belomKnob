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

function belom_flasher_auth_cb(WP_REST_Request $request) {
    $server_key = get_option('belom_flasher_key', 'CHANGE_ME_SECRET_KEY');
    $client_key = $request->get_header('x-belom-key');
    if (!$client_key) {
        $auth = $request->get_header('authorization');
        if (stripos($auth, 'Bearer ') === 0) {
            $client_key = substr($auth, 7);
        }
    }
    if (!$client_key) { return false; }
    return hash_equals((string)$server_key, (string)$client_key);
}

function belom_flasher_manifest_cb() {
    $upload = wp_upload_dir();
    $manifest = trailingslashit($upload['basedir']) . 'belom-firmware/manifest.json';
    if (!file_exists($manifest)) {
        return new WP_Error('manifest_missing', 'Manifest not found', ['status' => 404]);
    }
    $json = file_get_contents($manifest);
    $data = json_decode($json, true);
    if (!is_array($data)) {
        return new WP_Error('manifest_invalid', 'Manifest invalid', ['status' => 500]);
    }
    return rest_ensure_response($data);
}

function belom_flasher_download_cb(WP_REST_Request $request) {
    $file = basename($request['file']);
    $upload = wp_upload_dir();
    $path = trailingslashit($upload['basedir']) . 'belom-firmware/' . $file;
    if (!file_exists($path)) {
        return new WP_Error('file_missing', 'Firmware file not found', ['status' => 404]);
    }
    header('Content-Type: application/octet-stream');
    header('Content-Length: ' . filesize($path));
    header('Content-Disposition: attachment; filename="' . $file . '"');
    readfile($path);
    exit;
}
