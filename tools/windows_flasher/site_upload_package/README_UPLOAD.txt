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
