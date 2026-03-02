# -*- mode: python ; coding: utf-8 -*-

from pathlib import Path

BASE_DIR = Path(SPECPATH)

a = Analysis(
    ['flasher_app.py'],
    pathex=[],
    binaries=[],
    datas=[
        (str(BASE_DIR / 'assets' / 'logo-black.png'), 'assets'),
        (str(BASE_DIR / 'assets' / 'flash_icon.ico'), 'assets'),
    ],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='BelomFlasher',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=str(BASE_DIR / 'assets' / 'flash_icon.ico'),
)
