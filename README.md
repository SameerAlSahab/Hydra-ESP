<div align="center">

  <img src="resources/hydra_logo.png" alt="Hydra-ESP Logo" width="220"/>

  # Hydra-ESP
  **Wi-Fi + BT Security Research Tool for ESP32**

  <p width="80%">
  Extended with a redesigned web UI, multi-target deauth, BLE attacks, HID keystroke payloads, an aggressive Evil Twin module with password verification and Wi-Fi cloning, Ghost Probe, Wi-Fi beacon spam, Clientless PMKID Capture, WPA Handshake Capture, a deauth attack detector, optional OLED display support, and various security research modules.
  </p>

  <p>
  <a href="https://github.com/SameerAlSahab/Hydra-ESP/actions/workflows/build_page.yml"><img src="https://img.shields.io/github/actions/workflow/status/SameerAlSahab/Hydra-ESP/build_page.yml?branch=main&style=flat-square&label=pages%20build" alt="pages build status"></a>
  <a href="https://github.com/SameerAlSahab/Hydra-ESP/stargazers"><img src="https://img.shields.io/github/stars/SameerAlSahab/Hydra-ESP?style=flat-square&label=stars" alt="stars"></a>
  <a href="https://github.com/SameerAlSahab/Hydra-ESP/network/members"><img src="https://img.shields.io/github/forks/SameerAlSahab/Hydra-ESP?style=flat-square&label=forks" alt="forks"></a>
  <a href="https://github.com/SameerAlSahab/Hydra-ESP/issues"><img src="https://img.shields.io/github/issues/SameerAlSahab/Hydra-ESP?style=flat-square&label=open issues"></a>
  <a href="https://github.com/SameerAlSahab/Hydra-ESP/pulls"><img src="https://img.shields.io/github/issues-pr/SameerAlSahab/Hydra-ESP?style=flat-square&label=PRs" alt="open PRs"></a>
  <a href="https://github.com/SameerAlSahab/Hydra-ESP/commits/main"><img src="https://img.shields.io/github/last-commit/SameerAlSahab/Hydra-ESP?style=flat-square" alt="last commit"></a>
  <a href="LICENSE"><img src="https://img.shields.io/github/license/SameerAlSahab/Hydra-ESP?style=flat-square&label=license" alt="license"></a>
  <a href="https://github.com/SameerAlSahab/Hydra-ESP/graphs/contributors"><img src="https://img.shields.io/github/contributors/SameerAlSahab/Hydra-ESP?style=flat-square" alt="contributors"></a>
  <a href="https://github.com/SameerAlSahab/Hydra-ESP/discussions"><img src="https://img.shields.io/github/discussions/SameerAlSahab/Hydra-ESP?style=flat-square" alt="discussions"></a>
  </p>

</div>

---

## Demo Videos

<table>
<tr>
<td width="50%" align="center">
<a href="https://www.youtube.com/watch?v=91EigDhE2SE&t=22s">
<img src="https://img.youtube.com/vi/91EigDhE2SE/hqdefault.jpg" width="100%" alt="Hydra-ESP v1 demo"/>
</a>
<br/>
<b>v1 (old, fewer features)</b> — 62K+ genuine views
</td>
<td width="50%" align="center">
<a href="https://youtu.be/6GDkQS9YzEI">
<img src="https://img.youtube.com/vi/6GDkQS9YzEI/hqdefault.jpg" width="100%" alt="Hydra-ESP v2 demo"/>
</a>
<br/>
<b>v1.1.3 (new, current build)</b> — 13K+ views
</td>
</tr>
</table>

---

## Hardware

**Required**

- **ESP32 DevKit V1**, or any board built on the same ESP32 (Xtensa LX6 dual-core) SoC. Developed & tested on the standard 38-pin DevKit V1. WROOM-32 / WROVER modules should work fine too. S2, S3, C3, and other variants are **not** supported as they run different radio hardware.

**Optional**

- **SSD1306 OLED** (128x64, I2C). Shows attack timers, current status, menu, captured Evil Twin passwords, and logs. Auto-detected on boot — if it is not found, initialization skips quietly while maintaining full functionality through the web UI.

---

## How It Works

1. **Access Point Initialization:** ESP32 boots up its own management AP (`hydra`).
2. **Web Management Interface:** Connect your phone or laptop and open `http://192.168.4.1`. The UI is served directly from SPIFFS storage on the device — scan networks, select targets, run attack modules, monitor deauth activity, and configure credentials.
3. **Exclusive Radio Attacks:** Some attacks (Deauth, Evil Twin, Super Clone) require exclusive access to the radio. The management AP pauses while they are active, disconnecting the web UI temporarily. Setting a timeout automatically restores access upon completion; otherwise, a power cycle is required to stop the attack.

---

## Dependencies

| Library | License |
|---|---|
| [u8g2-hal-esp-idf](https://github.com/mkfrey/u8g2-hal-esp-idf) | See repository |
| [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) | See repository |
| [u8g2](https://github.com/olikraus/u8g2) | BSD 2-Clause |
| [esp-nimble-cpp](https://github.com/h2zero/esp-nimble-cpp) | Apache 2.0 |

---

## Attacks & Capabilities

### Deauthentication
Sends raw 802.11 deauth frames to disconnect clients from a target AP. Supports up to 16 targets simultaneously. Devices with 802.11w (MFP) enabled can resist plain frame-injection deauth — BSSID Clone mode can be used for these targets.

**Methods:**
- **Normal Deauth:** Plain 802.11 deauth frames (classic method).
- **Combined Deauth:** Deauth + disassociation frames sent together.
- **Multi-Clone Deauth:** Runs the attack across several cloned identities simultaneously.
- **BSSID Clone (Aggressive):** Clones the target's SSID and BSSID onto the ESP32 on the same channel. The address conflict causes clients to disconnect without sending raw unprotected deauth frames, making it effective against 802.11w/MFP devices.

---

### WPA Handshake Capture
Forces client re-authentications using deauth frames, captures the resulting WPA2 4-way handshake, and exports it as `.pcap` and `.hccapx` files for offline auditing using Hashcat or aircrack-ng against a wordlist. Requires an active client on the target network.

**Modes:** Normal Deauth / BSSID Clone (Aggressive) / Silent Capture (passive listening without forcing reconnects).

---

### Clientless PMKID Capture
Extracts the PMKID from the first EAPOL-Key frame during association without requiring connected clients — only the target AP needs to be within range. Fully compatible with Hashcat hash mode 22000.

---

### Beacon Spam
Floods the environment with fake 802.11 beacon frames (1–100 fake networks, configurable).

**Modes:**
- **Common Names:** Standard SSIDs like "Home WiFi" or "TP-Link" to blend into local environments.
- **Random Strings:** Generates random SSIDs to saturate Wi-Fi scan lists.
- **Rick Roll Mode:** Populates scan lists with lyric lines.
- **Security Names:** Emulates security and surveillance device network names.

---

### Ghost Mode (Probe Request Spam)
Listens for probe requests emitted by nearby client devices searching for saved networks, captures the SSIDs, and advertises matching access point names to redirect connection attempts to the ESP32.

---

### Evil Twin
Spins up an open clone of the target AP (matching SSID, no encryption) while running targeted deauthentication against the legitimate AP. Runs an embedded DNS server redirecting HTTP requests to a captive portal requesting network credentials.

Submitted passwords are dynamically validated against the real AP to confirm correctness before logging.

---

### Super Clone
Creates multiple clone APs sharing padded variations of the target SSID to display multiple identical-looking entries in device scan lists.

---

### BLE Spam
Broadcasts BLE advertisement packets mimicking proximity pairing signals across iOS, iPadOS, and Android platforms. MAC addresses rotate on each execution cycle.

**Supported Categories (25 Modes):**
- **Apple Audio (8 modes):** AirPods, AirPods Pro, AirPods Max, Beats prompts.
- **Apple Setup (5 modes):** Apple TV, HomePod, Vision Pro setup notifications.
- **Samsung (6 modes):** Galaxy Buds variants (1–5) and Samsung Random.
- **Google (5 modes):** Fast Pair variants (1–4) and Google Random.
- **Mixed Random (1 mode):** Rotates through all target profile types randomly.

---

### BT Payload
Emulates an HID Bluetooth keyboard under randomized hardware identities (`HydraBT-XXXX`). Once paired with a target system, it executes automated keystroke sequences.

**Included Payloads:**
- **Write to Notepad:** Launches Notepad and inputs a test string.
- **Play Rick Roll (YouTube):** Opens the default web browser to a target URL.
- **Set Warning Wallpaper:** Downloads and applies a custom desktop wallpaper.
- **Grab Wi-Fi Passwords:** Queries stored Wi-Fi profiles and posts data over HTTP to a designated log endpoint.
- **Hydra God Mode:** Executes the bundled multi-stage script.

Custom payloads can be added in [`main/bt/bt_payload_attack.cpp`](https://github.com/SameerAlSahab/Hydra-ESP/blob/main/main/bt/bt_payload_attack.cpp) and defined in [`/payloads`](https://github.com/SameerAlSahab/Hydra-ESP/tree/main/payloads). Pull requests for new payload modules are welcome!

---

### Deauth Attack Detector
Enables promiscuous 802.11 monitor mode to inspect management frames. Triggers visual alerts upon detecting 10+ deauth frames within one second from a single BSSID or broadcast deauth patterns (`00:00:00:00:00:00`).

---

## Web Interface

Accessible at `http://192.168.4.1` when connected to the management AP:

- **Scan:** Scans nearby Wi-Fi networks and lists SSID, BSSID, and RSSI metrics.
- **Attack:** Selects, configures, and executes attack modules.
- **Detector:** Controls monitor mode and displays real-time frame detection logs.
- **Settings:** Configures management AP credentials (saved to NVS flash).
- **About:** Displays firmware version, contributors, and legal disclaimers.

---

## Default Credentials

| Field | Default Value |
|---|---|
| **SSID** | `hydra` |
| **Password** | `notforfun` |
| **Web Portal** | `http://192.168.4.1` |

---

## Building from Source

Requires [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) toolchain configured for the ESP32 target.

```bash
git clone [https://github.com/SameerAlSahab/Hydra-ESP.git](https://github.com/SameerAlSahab/Hydra-ESP.git)
cd Hydra-ESP
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
