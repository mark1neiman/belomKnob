<?php
/**
 * Example WP endpoint for BEL0M flasher.
 * Put into a small plugin or theme functions.php (recommended: plugin).
 */

add_action('rest_api_init', function () {
    register_rest_route('belom-flasher/v1', '/manifest', [
        'methods'  => 'GET',
        'callback' => 'belom_flasher_manifest',
        'permission_callback' => 'belom_flasher_auth',
    ]);

    register_rest_route('belom-flasher/v1', '/download/(?P<file>[a-zA-Z0-9._-]+)', [
        'methods'  => 'GET',
        'callback' => 'belom_flasher_download',
        'permission_callback' => 'belom_flasher_auth',
    ]);
});

function belom_flasher_auth(WP_REST_Request $request) {
    $server_key = get_option('belom_flasher_key', 'CHANGE_ME_SECRET_KEY');
    $client_key = $request->get_header('x-belom-key');
    if (!$client_key) {
        $auth = $request->get_header('authorization');
        if (stripos($auth, 'Bearer ') === 0) {
            $client_key = substr($auth, 7);
        }
    }
    return hash_equals($server_key, (string)$client_key);
}

function belom_flasher_manifest() {
    $base = site_url('/wp-json/belom-flasher/v1/download/');
    return [
        'latest' => '2.2.1',
        'base_url' => $base,
        'firmwares' => [
            [
                'version' => '2.2.1',
                'chip' => 'esp32s3',
                'images' => [
                    [ 'name' => 'bootloader', 'offset' => '0x0', 'url' => 'bootloader.bin' ],
                    [ 'name' => 'partition-table', 'offset' => '0x8000', 'url' => 'partition-table.bin' ],
                    [ 'name' => 'app', 'offset' => '0x10000', 'url' => 'viewe_knob_touch.bin' ],
                ],
            ],
        ],
    ];
}

function belom_flasher_download(WP_REST_Request $request) {
    $allowed = ['bootloader.bin', 'partition-table.bin', 'viewe_knob_touch.bin'];
    $file = $request['file'];
    if (!in_array($file, $allowed, true)) {
        return new WP_Error('not_found', 'File not allowed', ['status' => 404]);
    }

    $upload = wp_upload_dir();
    $path = trailingslashit($upload['basedir']) . 'belom-firmware/' . $file;
    if (!file_exists($path)) {
        return new WP_Error('not_found', 'File not found', ['status' => 404]);
    }

    header('Content-Type: application/octet-stream');
    header('Content-Length: ' . filesize($path));
    header('Content-Disposition: attachment; filename="' . basename($path) . '"');
    readfile($path);
    exit;
}
