#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import os
import queue
import shlex
import subprocess
import sys
import threading
import time
from contextlib import redirect_stderr, redirect_stdout
from dataclasses import dataclass
from pathlib import Path
from tkinter import messagebox
from typing import Any, Dict, List, Optional, Tuple
from urllib.parse import urljoin

import requests
try:
    import esptool as esptool_mod
except Exception:
    esptool_mod = None

try:
    import serial
    from serial.tools import list_ports
except Exception:
    serial = None
    list_ports = None

try:
    import customtkinter as ctk
    from PIL import Image, ImageDraw
except Exception as exc:
    raise RuntimeError("Install requirements first: pip install -r requirements.txt") from exc

APP_TITLE = "BELOM Flasher"
APP_VERSION = "v1.0.2"
MANIFEST_URL = "https://admin.belom.eu/wp-json/belom-flasher/v1/manifest"
API_KEY = "CHANGE_ME_SECRET_KEY"
API_KEY_HEADER = "X-Belom-Key"
FIXED_BAUD = 460800
TEST_BAUDS = (115200, FIXED_BAUD)
TIMEOUT = 30
VER_PLACEHOLDER = "Select version..."
PORT_PLACEHOLDER = "Select COM port"
APP_DIR = Path(__file__).resolve().parent
DOWNLOAD_ROOT = APP_DIR / "downloads"

BG, CARD, BOR = "#ECECF1", "#F7F8FB", "#D8DCE8"
TXT, MUT, PUR, PUR_H, RED, GRN = "#2E3650", "#8E99B3", "#655CF0", "#584EE8", "#E44B4B", "#1D9C5A"
UI_FONT = ("Segoe UI", 12)
UI_FONT_SMALL = ("Segoe UI", 12)
UI_FONT_TITLE = ("Segoe UI", 12)
UI_MONO = ("Consolas", 12)
UI_LOG = ("Consolas", 12)


@dataclass
class Img:
    name: str
    offset: str
    url: str
    sha256: Optional[str] = None


@dataclass
class Pkg:
    version: str
    chip: str
    images: List[Img]


def rpath(rel: str) -> Path:
    if hasattr(sys, "_MEIPASS"):
        return Path(getattr(sys, "_MEIPASS")) / rel
    return APP_DIR / rel


class App(ctk.CTk):
    def __init__(self) -> None:
        ctk.set_appearance_mode("light")
        ctk.set_default_color_theme("blue")
        super().__init__()
        self.title(APP_TITLE)
        self.geometry("860x690")
        self.minsize(790, 620)
        self.configure(fg_color=BG)
        self._apply_window_icon()

        self.q: "queue.Queue[Tuple[str, Any]]" = queue.Queue()
        self.busy = False
        self.packages: List[Pkg] = []
        self.files: Dict[str, Path] = {}
        self.dev_ok = False

        self.var_ver = ctk.StringVar(value=VER_PLACEHOLDER)
        self.var_port = ctk.StringVar(value=PORT_PLACEHOLDER)
        self.var_status = ctk.StringVar(value="Not ready")
        self.var_prog = ctk.StringVar(value="Idle")
        self.var_pct = ctk.StringVar(value="0%")
        self.var_det = ctk.StringVar(value="-")
        self.var_footer = ctk.StringVar(value=f"COM -- @ {FIXED_BAUD}")

        self.icons: Dict[str, ctk.CTkImage] = {}
        self.logo = None
        self.combo_ver = None
        self.combo_port = None
        self.lbl_hdr = None
        self.lbl_status = None
        self.lbl_manifest = None
        self.pb = None
        self.log = None
        self._esptool_prefix: Optional[List[str]] = None

        self._build_icons()
        self._ui()
        self.after(100, self._poll)
        self._ports()
        self.fetch_manifest()

    def _ui(self) -> None:
        root = ctk.CTkFrame(self, fg_color=BG, corner_radius=0)
        root.pack(fill="both", expand=True, padx=8, pady=8)
        root.grid_columnconfigure(0, weight=1)
        root.grid_rowconfigure(5, weight=1)

        head = ctk.CTkFrame(root, fg_color=BG, corner_radius=0)
        head.grid(row=0, column=0, sticky="ew", pady=(0, 2))
        self.lbl_hdr = ctk.CTkLabel(head, text="\u25CF  Idle", text_color=MUT, font=UI_FONT_TITLE)
        self.lbl_hdr.pack(side="right")

        fw = self._card(root, "FIRMWARE", 1)
        select_style = dict(
            fg_color="#FFFFFF",
            text_color=TXT,
            dropdown_fg_color="#FFFFFF",
            dropdown_text_color=TXT,
            dropdown_hover_color="#EEF2F8",
            button_color="#FFFFFF",
            button_hover_color="#F3F5FA",
            corner_radius=10,
            height=30,
            font=UI_FONT,
            anchor="w",
            dynamic_resizing=False,
        )
        ctk.CTkLabel(fw, text="Version", text_color=TXT, font=UI_FONT).grid(row=1, column=0, padx=12, pady=(1, 6), sticky="w")
        self.combo_ver = ctk.CTkOptionMenu(fw, variable=self.var_ver, values=[VER_PLACEHOLDER], command=self._sel_ver, **select_style)
        self.combo_ver.grid(row=1, column=1, padx=(6, 8), pady=(1, 6), sticky="ew")
        ctk.CTkButton(
            fw,
            text="Refresh",
            image=self.icons["refresh"],
            compound="left",
            command=self.fetch_manifest,
            fg_color=CARD,
            border_color=BOR,
            border_width=1,
            hover_color="#EEF2F8",
            text_color="#5E7196",
            height=30,
            width=94,
            corner_radius=10,
            font=UI_FONT,
        ).grid(row=1, column=2, padx=(0, 12), pady=(1, 6))
        self.lbl_manifest = ctk.CTkLabel(fw, text="Manifest not loaded", text_color=MUT, font=UI_FONT_SMALL)
        self.lbl_manifest.grid(row=2, column=1, padx=(6, 0), pady=(0, 6), sticky="w")

        uart = self._card(root, "UART CONNECTION", 2)
        ctk.CTkLabel(uart, text="COM port", text_color=TXT, font=UI_FONT).grid(row=1, column=0, padx=12, pady=(1, 6), sticky="w")
        self.combo_port = ctk.CTkOptionMenu(
            uart,
            variable=self.var_port,
            values=[PORT_PLACEHOLDER],
            command=lambda _v: self._ready(),
            **select_style,
        )
        self.combo_port.grid(row=1, column=1, padx=(6, 8), pady=(1, 6), sticky="ew")
        ctk.CTkButton(
            uart,
            text="Scan",
            image=self.icons["refresh"],
            compound="left",
            command=self._ports,
            fg_color=CARD,
            border_color=BOR,
            border_width=1,
            hover_color="#EEF2F8",
            text_color="#5E7196",
            height=30,
            width=94,
            corner_radius=10,
            font=UI_FONT,
        ).grid(row=1, column=2, padx=(0, 12), pady=(1, 6))
        ctk.CTkLabel(uart, text="Baud rate", text_color=TXT, font=UI_FONT).grid(row=2, column=0, padx=12, pady=(0, 6), sticky="w")
        ctk.CTkLabel(uart, text=f"{FIXED_BAUD}", text_color="#52627E", font=UI_MONO).grid(row=2, column=1, padx=(6, 0), pady=(0, 6), sticky="w")
        ctk.CTkButton(
            uart,
            text="Check device",
            image=self.icons["shield_check"],
            compound="left",
            command=self.check_dev,
            fg_color=CARD,
            border_color=BOR,
            border_width=1,
            hover_color="#EEF2F8",
            text_color="#5E7196",
            height=30,
            width=122,
            corner_radius=10,
            font=UI_FONT,
        ).grid(row=2, column=2, padx=(0, 12), pady=(0, 6))

        act = self._card(root, "ACTIONS", 3)
        act.grid_columnconfigure(0, weight=1)
        act.grid_columnconfigure(1, weight=1)
        ghost = dict(fg_color=CARD, text_color="#475B82", hover_color="#ECEFF6", border_color=BOR, border_width=1, corner_radius=10, height=34, font=UI_FONT)
        ctk.CTkButton(act, text="Download FW", image=self.icons["download"], compound="left", command=self.dl_fw, **ghost).grid(row=1, column=0, padx=(12, 4), pady=(1, 4), sticky="ew")
        ctk.CTkButton(act, text="Flash", image=self.icons["bolt_white"], compound="left", command=self.flash, fg_color=PUR, hover_color=PUR_H, text_color="#fff", corner_radius=10, height=34, font=UI_FONT).grid(row=1, column=1, padx=(4, 12), pady=(1, 4), sticky="ew")
        ctk.CTkButton(act, text="Flash + Tests", image=self.icons["bolt"], compound="left", command=self.flash_tests, **ghost).grid(row=2, column=0, padx=(12, 4), pady=(4, 8), sticky="ew")
        ctk.CTkButton(act, text="Mini Tests", image=self.icons["beaker"], compound="left", command=self.tests, **ghost).grid(row=2, column=1, padx=(4, 12), pady=(4, 8), sticky="ew")

        pr = ctk.CTkFrame(root, fg_color=BG, corner_radius=0)
        pr.grid(row=4, column=0, sticky="ew", pady=(1, 0))
        pr.grid_columnconfigure(0, weight=1)
        ctk.CTkLabel(pr, text="Progress", text_color=TXT, font=UI_FONT).grid(row=0, column=0, sticky="w")
        ctk.CTkLabel(pr, textvariable=self.var_pct, text_color=MUT, font=UI_FONT).grid(row=0, column=1, sticky="e")
        self.pb = ctk.CTkProgressBar(pr, fg_color="#DEE1E9", progress_color=PUR, corner_radius=8, height=8)
        self.pb.grid(row=1, column=0, columnspan=2, sticky="ew", pady=(3, 2))
        self.pb.set(0.0)
        ctk.CTkLabel(pr, textvariable=self.var_prog, text_color=MUT, font=UI_FONT).grid(row=2, column=0, sticky="w")

        logc = self._card(root, "LOG", 5)
        logc.grid_rowconfigure(1, weight=1)
        ctk.CTkButton(
            logc,
            text="Copy log",
            command=self._copy_log,
            fg_color=CARD,
            border_color=BOR,
            border_width=1,
            hover_color="#EEF2F8",
            text_color="#5E7196",
            height=28,
            width=100,
            corner_radius=8,
            font=UI_FONT,
        ).grid(row=0, column=2, padx=(0, 12), pady=(4, 2), sticky="e")
        self.log = ctk.CTkTextbox(logc, fg_color="#F5F7FB", text_color=RED, border_color=BOR, border_width=1, corner_radius=10, font=UI_LOG, height=260)
        self.log.grid(row=1, column=0, columnspan=3, sticky="nsew", padx=12, pady=(2, 8))
        self.log.configure(state="disabled")
        self.log.bind("<Control-c>", self._copy_log_event)
        self.log.bind("<Control-C>", self._copy_log_event)
        self.log.bind("<Control-a>", self._select_all_log)
        self.log.bind("<Control-A>", self._select_all_log)
        self._set_det("-")

        f = ctk.CTkFrame(root, fg_color=BG, corner_radius=0)
        f.grid(row=6, column=0, sticky="ew", pady=(1, 0))
        ctk.CTkLabel(f, textvariable=self.var_footer, text_color=MUT, font=UI_LOG).pack(side="left")

    def _apply_window_icon(self) -> None:
        icon_path = rpath("assets/flash_icon.ico")
        if not icon_path.exists():
            return
        try:
            self.iconbitmap(str(icon_path))
        except Exception:
            pass

    def _card(self, parent: ctk.CTkFrame, title: str, row: int) -> ctk.CTkFrame:
        c = ctk.CTkFrame(parent, fg_color=CARD, corner_radius=12, border_color=BOR, border_width=1)
        c.grid(row=row, column=0, sticky="ew", pady=4)
        c.grid_columnconfigure(1, weight=1)
        ctk.CTkLabel(c, text=title, text_color=MUT, font=UI_FONT).grid(row=0, column=0, padx=12, pady=(6, 4), sticky="w")
        return c

    def _build_icons(self) -> None:
        self.icons = {
            "bolt_title": self._icon("bolt", PUR, 18, 2),
            "bolt": self._icon("bolt", "#566891", 16, 2),
            "bolt_white": self._icon("bolt", "#FFFFFF", 16, 2),
            "refresh": self._icon("refresh", "#7A86A4", 16, 2),
            "download": self._icon("download", "#5C6D92", 16, 2),
            "beaker": self._icon("beaker", "#5C6D92", 16, 2),
            "shield_check": self._icon("shield_check", "#5C6D92", 16, 2),
        }

    def _icon(self, kind: str, color: str, size: int, stroke: int) -> ctk.CTkImage:
        scale = 5
        im = Image.new("RGBA", (size * scale, size * scale), (0, 0, 0, 0))
        draw = ImageDraw.Draw(im)
        w = max(1, stroke * scale)

        def p(x: float, y: float) -> Tuple[float, float]:
            return x * scale, y * scale

        def path(points: List[Tuple[float, float]]) -> None:
            draw.line([p(x, y) for x, y in points], fill=color, width=w, joint="curve")

        if kind == "bolt":
            path([(9.5, 1.5), (4.8, 8.7), (8.8, 8.7), (6.2, 14.8), (12.2, 7.2), (8.6, 7.2), (9.5, 1.5)])
        elif kind == "refresh":
            draw.arc([p(2.0, 2.0), p(14.0, 14.0)], start=40, end=200, fill=color, width=w)
            draw.arc([p(2.0, 2.0), p(14.0, 14.0)], start=220, end=380, fill=color, width=w)
            path([(11.7, 2.3), (13.8, 2.9), (13.0, 5.0)])
            path([(4.3, 13.7), (2.2, 13.1), (3.0, 11.0)])
        elif kind == "download":
            path([(8.0, 2.0), (8.0, 10.2)])
            path([(5.2, 7.4), (8.0, 10.2), (10.8, 7.4)])
            path([(3.0, 13.0), (13.0, 13.0)])
        elif kind == "beaker":
            path([(6.0, 2.0), (6.0, 5.7), (3.8, 12.5), (12.2, 12.5), (10.0, 5.7), (10.0, 2.0)])
            path([(5.2, 9.7), (10.8, 9.7)])
        elif kind == "shield_check":
            path([(8.0, 1.8), (12.2, 3.6), (12.2, 8.6), (8.0, 13.7), (3.8, 8.6), (3.8, 3.6), (8.0, 1.8)])
            path([(6.0, 8.0), (7.5, 9.5), (10.2, 6.8)])

        resampling = getattr(Image, "Resampling", Image).LANCZOS
        out = im.resize((size, size), resample=resampling)
        return ctk.CTkImage(light_image=out, dark_image=out, size=(size, size))

    def _set_hdr(self, text: str, col: str) -> None:
        if self.lbl_hdr:
            self.lbl_hdr.configure(text=f"\u25CF  {text}", text_color=col)

    def _resolve_esptool_prefix(self) -> List[str]:
        if self._esptool_prefix is not None:
            return self._esptool_prefix

        candidates: List[List[str]] = []
        override = os.environ.get("BELOM_ESPTOOL_CMD", "").strip()
        if override:
            parsed = shlex.split(override, posix=False)
            if parsed:
                candidates.append(parsed)

        frozen = bool(getattr(sys, "frozen", False))
        if not frozen:
            candidates.append([sys.executable, "-m", "esptool"])
        else:
            exe_path = Path(sys.executable).resolve()
            for py_name in ("python.exe", "python3.exe", "python"):
                py = exe_path.parent / py_name
                if py.exists() and py.resolve() != exe_path:
                    candidates.append([str(py), "-m", "esptool"])
            candidates.extend([
                ["esptool"],
                ["py", "-m", "esptool"],
                ["python", "-m", "esptool"],
            ])

        checked: List[str] = []
        for prefix in candidates:
            key = " ".join(prefix)
            if key in checked:
                continue
            checked.append(key)
            try:
                probe = subprocess.run(
                    prefix + ["version"],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    encoding="utf-8",
                    errors="replace",
                    timeout=20,
                )
                if probe.returncode == 0:
                    self._esptool_prefix = prefix
                    self.q.put(("log", "esptool command: " + key))
                    return prefix
            except Exception:
                continue

        raise RuntimeError(
            "esptool not found. Install esptool or set BELOM_ESPTOOL_CMD."
        )

    def _esptool_args(self, *args: str) -> List[str]:
        return self._resolve_esptool_prefix() + list(args)

    def _set_det(self, text: str) -> None:
        self.var_det.set(text)
        self._log(text if text else "-")

    def _copy_log(self) -> None:
        if not self.log:
            return
        text = self.log.get("1.0", "end-1c")
        if not text:
            return
        self.clipboard_clear()
        self.clipboard_append(text)
        self.update_idletasks()

    def _copy_log_event(self, _event=None):
        if not self.log:
            return "break"
        try:
            text = self.log.get("sel.first", "sel.last")
        except Exception:
            text = self.log.get("1.0", "end-1c")
        if text:
            self.clipboard_clear()
            self.clipboard_append(text)
            self.update_idletasks()
        return "break"

    def _select_all_log(self, _event=None):
        if self.log:
            self.log.tag_add("sel", "1.0", "end-1c")
            self.log.mark_set("insert", "1.0")
            self.log.see("insert")
        return "break"

    def _log(self, text: str) -> None:
        if not self.log:
            return
        self.log.configure(state="normal")
        self.log.insert("end", f"[{time.strftime('%I:%M:%S %p')}] {text}\n")
        self.log.see("end")
        self.log.configure(state="disabled")

    def _ready(self) -> None:
        pkg = self._pkg()
        port = self._selected_port()
        has = pkg is not None and all(i.name in self.files for i in pkg.images)
        ok = self.dev_ok and has and bool(port)
        self.var_status.set("Ready" if ok else "Not ready")
        if self.lbl_status:
            self.lbl_status.configure(text_color=GRN if ok else RED)
        self._set_hdr("Idle", MUT)
        self.var_footer.set(f"COM {port or '--'} @ {FIXED_BAUD}")

    def _poll(self) -> None:
        while True:
            try:
                ev, pl = self.q.get_nowait()
            except queue.Empty:
                break
            if ev == "log":
                self._log(str(pl))
            elif ev == "manifest":
                self._apply_manifest(pl)
            elif ev == "progress":
                p = max(0.0, min(100.0, float(pl)))
                self.var_pct.set(f"{int(p)}%")
                if self.pb:
                    self.pb.set(p / 100.0)
            elif ev == "prog_txt":
                self.var_prog.set(str(pl))
            elif ev == "dev":
                self.dev_ok = bool(pl.get("ok"))
                self._ready()
            elif ev == "files":
                self.files = pl
                self._ready()
            elif ev == "done":
                self.busy = False
                if self.var_prog.get() != "Error":
                    self.var_prog.set("Idle")
                    self._set_hdr("Idle", MUT)
            elif ev == "err":
                self.busy = False
                self.var_prog.set("Error")
                self._set_hdr("Error", RED)
                messagebox.showerror("Error", str(pl))
            elif ev == "info":
                messagebox.showinfo("Info", str(pl))
        self.after(100, self._poll)

    def _run(self, target, *args) -> None:
        if self.busy:
            messagebox.showwarning("Busy", "Another operation is running.")
            return
        self.busy = True
        self._set_hdr("Working", "#8D83FF")
        threading.Thread(target=target, args=args, daemon=True).start()

    def _headers(self) -> Dict[str, str]:
        key = os.environ.get("BELOM_FLASHER_KEY", API_KEY).strip()
        h = {"User-Agent": "BelomFlasher/1.0", API_KEY_HEADER: key}
        if key:
            h["Authorization"] = f"Bearer {key}"
        return h

    def _cmd(self, args: List[str], timeout: int = 300) -> Tuple[int, str]:
        self.q.put(("log", "CMD: " + " ".join(args)))
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding="utf-8", errors="replace")
        out, t0 = [], time.time()
        assert p.stdout is not None
        while True:
            ln = p.stdout.readline()
            if ln:
                s = ln.rstrip()
                out.append(s)
                self.q.put(("log", s))
            if p.poll() is not None:
                break
            if time.time() - t0 > timeout:
                p.kill()
                return 1, "timeout"
        return p.returncode, "\n".join(out)

    def _cmd_embedded_esptool(self, args: List[str]) -> Tuple[int, str]:
        if esptool_mod is None:
            return 1, "embedded esptool not available"

        logs: List[str] = []
        self.q.put(("log", "CMD: esptool " + " ".join(args)))

        class _QStream:
            def __init__(self, q_ref: "queue.Queue[Tuple[str, Any]]", out_ref: List[str]) -> None:
                self.q_ref = q_ref
                self.out_ref = out_ref
                self.buf = ""

            def write(self, text: str) -> int:
                if not text:
                    return 0
                self.buf += text
                while "\n" in self.buf:
                    line, self.buf = self.buf.split("\n", 1)
                    line = line.rstrip("\r")
                    if line:
                        self.out_ref.append(line)
                        self.q_ref.put(("log", line))
                return len(text)

            def flush(self) -> None:
                line = self.buf.rstrip("\r")
                if line:
                    self.out_ref.append(line)
                    self.q_ref.put(("log", line))
                self.buf = ""

        stream = _QStream(self.q, logs)
        try:
            with redirect_stdout(stream), redirect_stderr(stream):
                rc = 0
                try:
                    esptool_mod.main(list(args))
                except SystemExit as e:
                    if isinstance(e.code, int):
                        rc = e.code
                    elif e.code is None:
                        rc = 0
                    else:
                        rc = 1
        except Exception as e:
            stream.flush()
            msg = f"embedded esptool failed: {e}"
            self.q.put(("log", msg))
            return 1, msg
        stream.flush()
        return rc, "\n".join(logs)

    def _run_esptool(self, args: List[str], timeout: int = 300) -> Tuple[int, str]:
        if esptool_mod is not None:
            return self._cmd_embedded_esptool(args)
        return self._cmd(self._esptool_args(*args), timeout)

    def _run_esptool_probe(self, port: str, command: str, timeout: int = 40) -> Tuple[int, str]:
        last_rc = 1
        last_out = ""
        for i, baud in enumerate(TEST_BAUDS):
            args = [
                "--port", port,
                "--baud", str(baud),
                "--before", "default_reset",
                "--after", "no_reset",
                command,
            ]
            if i > 0:
                self.q.put(("log", f"Retry {command} at {baud} baud..."))
            rc, out = self._run_esptool(args, timeout)
            if rc == 0:
                return rc, out
            last_rc, last_out = rc, out
        return last_rc, last_out

    def _esptool(self) -> None:
        rc, _ = self._run_esptool(["version"], 20)
        if rc != 0:
            raise RuntimeError("esptool missing. Install requirements.")

    def _ports(self) -> None:
        vals = [p.device for p in list_ports.comports()] if list_ports is not None else []
        if self.combo_port:
            self.combo_port.configure(values=vals if vals else [PORT_PLACEHOLDER])
        if vals:
            if self.var_port.get() not in vals:
                self.var_port.set(vals[0])
            if self.combo_port:
                self.combo_port.set(self.var_port.get())
        else:
            self.var_port.set(PORT_PLACEHOLDER)
            if self.combo_port:
                self.combo_port.set(PORT_PLACEHOLDER)
        self._log(f"Ports: {', '.join(vals) if vals else 'none'}")
        self._ready()

    def _pkg(self) -> Optional[Pkg]:
        v = self.var_ver.get().strip()
        return next((p for p in self.packages if p.version == v), None)

    def _sel_ver(self, _value: str = "") -> None:
        self._ready()

    def _selected_port(self) -> str:
        port = self.var_port.get().strip()
        if not port or port == PORT_PLACEHOLDER:
            return ""
        return port

    def fetch_manifest(self) -> None:
        self._run(self._w_manifest)

    def _w_manifest(self) -> None:
        try:
            self.q.put(("prog_txt", "Fetching manifest..."))
            r = requests.get(MANIFEST_URL, headers=self._headers(), timeout=TIMEOUT)
            r.raise_for_status()
            self.q.put(("manifest", r.json()))
            self.q.put(("log", "Manifest loaded."))
        except Exception as e:
            self.q.put(("err", f"Manifest fetch failed: {e}"))
        finally:
            self.q.put(("done", None))

    def _apply_manifest(self, data: Dict[str, Any]) -> None:
        self.packages = []
        base = str(data.get("base_url", MANIFEST_URL))
        for raw in data.get("firmwares", []):
            imgs = []
            for i in raw.get("images", []):
                u = str(i.get("url", "")).strip()
                imgs.append(Img(str(i.get("name", "")).strip(), str(i.get("offset", "")).strip(), u if "://" in u else urljoin(base, u), i.get("sha256")))
            p = Pkg(str(raw.get("version", "")).strip(), str(raw.get("chip", "esp32s3")).strip(), imgs)
            if p.version and p.images:
                self.packages.append(p)
        vv = [p.version for p in self.packages]
        if self.combo_ver:
            self.combo_ver.configure(values=vv if vv else [VER_PLACEHOLDER])
        latest = str(data.get("latest", "")).strip()
        self.var_ver.set(latest if latest in vv else (vv[-1] if vv else VER_PLACEHOLDER))
        if self.combo_ver:
            self.combo_ver.set(self.var_ver.get())
        if self.lbl_manifest:
            self.lbl_manifest.configure(text=f"{len(self.packages)} versions")
        self._ready()

    def check_dev(self) -> None:
        self._run(self._w_check)

    def _w_check(self) -> None:
        try:
            port = self._selected_port()
            if not port:
                raise RuntimeError("Device not ready: no COM port.")
            self.q.put(("prog_txt", "Checking device..."))
            self._esptool()
            rc1, out1 = self._run_esptool_probe(port, "chip-id", 40)
            rc2, _ = self._run_esptool_probe(port, "read-mac", 40)
            if rc1 != 0 or rc2 != 0:
                raise RuntimeError("Device check failed.")
            self.q.put(("dev", {"ok": True}))
            self.q.put(("log", "Device is ready."))
        except Exception as e:
            self.q.put(("dev", {"ok": False}))
            self.q.put(("err", str(e)))
        finally:
            self.q.put(("done", None))

    def dl_fw(self) -> None:
        self._run(self._w_dl)

    def _w_dl(self) -> None:
        try:
            p = self._pkg()
            if not p:
                raise RuntimeError("No firmware version selected.")
            dst = DOWNLOAD_ROOT / p.version
            dst.mkdir(parents=True, exist_ok=True)
            tot, got, files = 0, 0, {}
            for i in p.images:
                try:
                    h = requests.head(i.url, headers=self._headers(), timeout=TIMEOUT, allow_redirects=True)
                    if h.ok:
                        tot += int(h.headers.get("Content-Length", "0"))
                except Exception:
                    pass
            for n, i in enumerate(p.images, 1):
                out = dst / f"{i.name}.bin"
                self.q.put(("log", f"Downloading [{n}/{len(p.images)}] {i.url}"))
                with requests.get(i.url, headers=self._headers(), stream=True, timeout=TIMEOUT * 2) as r:
                    r.raise_for_status()
                    with out.open("wb") as f:
                        for c in r.iter_content(chunk_size=64 * 1024):
                            if not c:
                                continue
                            f.write(c)
                            got += len(c)
                            if tot > 0:
                                pct = got * 100.0 / tot
                                self.q.put(("progress", pct))
                                self.q.put(("prog_txt", f"Downloading... {pct:.1f}%"))
                if i.sha256:
                    h = hashlib.sha256(out.read_bytes()).hexdigest()
                    if h.lower() != i.sha256.lower():
                        raise RuntimeError(f"SHA mismatch: {i.name}")
                files[i.name] = out
            self.q.put(("progress", 100.0))
            self.q.put(("files", files))
            self.q.put(("log", "Firmware files downloaded."))
        except Exception as e:
            self.q.put(("err", f"Download failed: {e}"))
        finally:
            self.q.put(("done", None))

    def flash(self) -> None:
        self._run(self._w_flash, False)

    def flash_tests(self) -> None:
        self._run(self._w_flash, True)

    def _w_flash(self, run_tests: bool) -> None:
        try:
            port = self._selected_port()
            p = self._pkg()
            if not port:
                raise RuntimeError("No COM port selected.")
            if not p:
                raise RuntimeError("No firmware version selected.")
            if not all(i.name in self.files for i in p.images):
                raise RuntimeError("Firmware files are not downloaded.")
            self.q.put(("prog_txt", "Flashing..."))
            self._esptool()
            cmd = ["--chip", p.chip, "--port", port, "--baud", str(FIXED_BAUD),
                   "--before", "default_reset", "--after", "hard_reset", "write_flash", "-z"]
            for i in p.images:
                cmd += [i.offset, str(self.files[i.name])]
            rc, _ = self._run_esptool(cmd, 600)
            if rc != 0:
                raise RuntimeError("Flash failed.")
            self.q.put(("log", "Flash successful."))
            self.q.put(("progress", 100.0))
            if run_tests:
                self._w_tests(True)
        except Exception as e:
            self.q.put(("err", str(e)))
        finally:
            self.q.put(("done", None))

    def tests(self) -> None:
        self._run(self._w_tests, False)

    def _w_tests(self, inline: bool) -> None:
        try:
            port = self._selected_port()
            if not port:
                raise RuntimeError("No COM port selected.")
            r = []
            rc1, _ = self._run_esptool_probe(port, "chip-id", 40)
            r.append(("chip_id", rc1 == 0))
            rc2, _ = self._run_esptool_probe(port, "read-mac", 40)
            r.append(("read_mac", rc2 == 0))
            ok3 = True
            if serial is not None:
                try:
                    with serial.Serial(port=port, baudrate=115200, timeout=1) as s:
                        s.reset_input_buffer()
                        _ = s.read(16)
                except Exception:
                    ok3 = False
            r.append(("uart", ok3))
            passed = sum(1 for _, ok in r if ok)
            self.q.put(("log", "Mini tests: " + " | ".join([f"{n}:{'OK' if ok else 'FAIL'}" for n, ok in r])))
            self.q.put(("dev", {"ok": rc1 == 0 and rc2 == 0}))
            if passed < len(r):
                raise RuntimeError("Mini tests failed.")
            if not inline:
                self.q.put(("info", f"Mini tests passed ({passed}/{len(r)})."))
        except Exception as e:
            self.q.put(("err", str(e)))
        finally:
            if not inline:
                self.q.put(("done", None))


if __name__ == "__main__":
    App().mainloop()
