<div align="center">

  <img src="resources/hydra_logo.png" alt="Hydra-ESP Logo" width="220"/>

  # Hydra-ESP
  **Wi-Fi + BT Security Research Tool for ESP32**

<p width="80%">
Extended with a redesigned web UI, multi-target deauth, BLE attacks, HID keystroke payloads, an aggressive Evil Twin module with password verification and Wi-Fi cloning, Ghost Probe, Wi-Fi beacon spam, Clientless PMKID Capture, WPA Handshake Capture, a deauth attack detector, optional OLED display support, and various security research modules.
</p>

<br/>

<p>
<a href="https://github.com/SameerAlSahab/Hydra-ESP/actions/workflows/build_page.yml"><img src="https://img.shields.io/github/actions/workflow/status/SameerAlSahab/Hydra-ESP/build_page.yml?branch=main&style=flat-square&label=pages%20build" alt="pages build status"></a>
<a href="https://github.com/SameerAlSahab/Hydra-ESP/stargazers"><img src="https://img.shields.io/github/stars/SameerAlSahab/Hydra-ESP?style=flat-square&label=stars" alt="stars"></a>
<a href="https://github.com/SameerAlSahab/Hydra-ESP/network/members"><img src="https://img.shields.io/github/forks/SameerAlSahab/Hydra-ESP?style=flat-square&label=forks" alt="forks"></a>
<a href="https://github.com/SameerAlSahab/Hydra-ESP/issues"><img src="https://img.shields.io/github/issues/SameerAlSahab/Hydra-ESP?style=flat-square&label=issues" alt="open issues"></a>
<a href="https://github.com/SameerAlSahab/Hydra-ESP/pulls"><img src="https://img.shields.io/github/issues-pr/SameerAlSahab/Hydra-ESP?style=flat-square&label=PRs" alt="open PRs"></a>
<a href="https://github.com/SameerAlSahab/Hydra-ESP/commits/main"><img src="https://img.shields.io/github/last-commit/SameerAlSahab/Hydra-ESP?style=flat-square" alt="last commit"></a>
<a href="LICENSE"><img src="https://img.shields.io/github/license/SameerAlSahab/Hydra-ESP?style=flat-square&label=license" alt="license"></a>
<a href="https://github.com/SameerAlSahab/Hydra-ESP/graphs/contributors"><img src="https://img.shields.io/github/contributors/SameerAlSahab/Hydra-ESP?style=flat-square" alt="contributors"></a>
<a href="https://github.com/SameerAlSahab/Hydra-ESP/discussions"><img src="https://img.shields.io/github/discussions/SameerAlSahab/Hydra-ESP?style=flat-square" alt="discussions"></a>
</p>

---

## Demo videos

<table>
<tr>
<td width="50%" align="center">
<a href="https://www.youtube.com/watch?v=91EigDhE2SE&t=22s">
<img src="https://img.youtube.com/vi/91EigDhE2SE/hqdefault.jpg" width="100%" alt="Hydra-ESP v1 demo"/>
</a>
<br/>
<b>v1 (old, fewer features)</b> — 62K+ genuine views (as of Sept 2026)
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

- ESP32 DevKit V1, or any board built on the same ESP32 (Xtensa LX6 dual-core) SoC. Dev + tested on the standard 38-pin DevKit V1. WROOM-32 / WROVER modules should work fine too. S2, S3, C3, and other variants are **not** supported, they run different radio hardware.

**Optional**

- SSD1306 OLED (128x64, I2C). Shows attack timers, current status, menu, captured Evil Twin passwords, and logs. Auto-detected on boot — if it's not there, it just skips init quietly and you still get everything through the web UI.

---

## How it works

ESP32 boots up its own management AP. Connect your phone/laptop to it and open `http://192.168.4.1`. UI is served straight off SPIFFS on the device — scan networks, pick a target, run attacks, watch deauth activity, change creds.

Some attacks (Deauth, Evil Twin, Super Clone) need the radio to themselves, so the management AP drops while they're running — you lose the web UI connection for the duration. Set a timeout and it comes back on its own; without one you'll need to power cycle to stop the attack and get access back.

---

## Dependencies

| Library | License |
|---|---|
| [u8g2-hal-esp-idf](https://github.com/mkfrey/u8g2-hal-esp-idf) | See repo |
| [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) | See repo |
| [u8g2](https://github.com/olikraus/u8g2) | BSD 2-Clause |
| [esp-nimble-cpp](https://github.com/h2zero/esp-nimble-cpp) | Apache 2.0 |

---

## Attacks

### Deauthentication
Picked from the "Deauth" button in the Attack tab. Sends raw 802.11 deauth frames to boot clients off a target AP. Up to 16 targets at once. Devices with 802.11w (MFP) enabled can resist plain frame-injection deauth — that's what BSSID Clone mode below is for.

**Modes (pick one from the Attack Method dropdown):**
- **Normal Deauth** — plain 802.11 deauth frames, the classic method.
- **Combined Deauth** — deauth + disassociation frames sent together, harder to ignore for some clients.
- **Multi-Clone Deauth** — runs the attack across several cloned identities at once instead of one.
- **BSSID Clone (Aggressive)** — clones the target's SSID *and* BSSID onto the ESP32 on the same channel, so the real AP and the clone look identical. The address clash boots clients off on its own, and it still works on 802.11w/MFP devices since it's not sending raw unprotected deauth frames at all.

**Timeout:** default 2 minutes,but turn the timeout checkbox off and the attack runs forever — you'll need to power cycle the ESP32 to get the management AP back.

---

### WPA Handshake Capture
Forces reconnects with deauth frames, captures the resulting WPA2 4-way handshake, saves it as `.pcap` and `.hccapx` for offline auditing with Hashcat / aircrack-ng against a wordlist. Needs a connected client on the target network. Runs until it gets a handshake or you stop it.

**Modes:** Normal Deauth / BSSID Clone (Aggressive) / Silent Capture (just listens, doesn't force a reconnect — needs a client to reconnect naturally).

---

### Clientless PMKID Capture
Grabs the PMKID off the first EAPOL-Key frame during association — no connected client needed, the AP just has to be in range. No mode to pick here, it's a single fixed method. Works with Hashcat hash mode 22000. Most modern WPA2 APs leak this, some don't include it at all.

---

### Beacon Spam
Floods the area with fake 802.11 beacon frames. 1–100 fake networks, configurable. Default timeout 5 minutes.

**Modes:**
- **Common Names** — everyday SSIDs like "Home WiFi", "TP-Link", etc, blends in with normal noise.
- **Random Strings** — gibberish SSIDs, obviously fake, good for just flooding the scan list.
- **Rick Roll Mode** — you can guess.
- **Security Names** — SSIDs styled to look like security/surveillance gear, mostly for messing with a scan list.

---

### Ghost Mode (Probe Request Spam)
Listens for probe requests devices send out for their saved networks, grabs the SSIDs, then starts advertising those exact names back. Devices try to connect to the ESP32 instead of their real saved network. No mode/method to configure — just target-less and timeout, default 5 minutes.

---

### Evil Twin
Called "Devil Twin" in the UI. Spins up an open clone of the target AP (same SSID, no password) and starts deauthing the real one at the same time to push clients off it. Runs its own DNS server so every request from a connected client gets redirected to a captive portal ("firmware update" style page) asking for the Wi-Fi password.

Whatever password gets submitted isn't just logged blind — the firmware actually tries connecting to the *real* AP with it to confirm it's correct. Wrong password → it's logged as a failed attempt and the victim gets sent back to the portal. Right password → captured, verified, and shown in the web UI. Default timeout 5 minutes, and the web UI is unreachable while it's running — power cycle to stop early if no timeout is set.

---

### Super Clone
This is the "SSID Cloner" — spins up multiple APs sharing (near-)identical SSIDs by padding the real name with spaces, so a scan list shows what looks like several copies of the same network. Only one method: "Open Multiple Clones". Default timeout 5 minutes.

---

### BLE Spam
Broadcasts BLE advertisement packets mimicking Apple/Samsung/Google proximity pairing signals — iPhones, iPads, Android devices pop up pairing prompts for nearby "devices". Default timeout 15 minutes, MAC address rotates on every run.

**25 selectable modes**, grouped as:
- **Apple Audio (8 modes)** — AirPods/AirPods Pro/AirPods Max/Beats style pairing popups.
- **Apple Setup (5 modes)** — Apple TV, HomePod, Vision Pro style setup prompts.
- **Samsung (6 modes)** — Galaxy Buds variants 1–5, plus a Samsung Random mode.
- **Google (5 modes)** — Fast Pair variants 1–4, plus a Google Random mode.
- **Mixed Random (1 mode)** — picks randomly across all of the above.

---

### BT Payload
Advertises the ESP32 as a Bluetooth HID keyboard under a random name (`HydraBT-XXXX`) with a randomised MAC, generated fresh every run. Once something pairs with it, it types out one of a few payloads. Mostly aimed at Windows — pair from Android with an app like nRF Connect.

**Built-in payloads (pick from the Attack tab once connected):**
- **Write to Notepad** — opens Notepad, types a short message. The "hello world" of this feature.
- **Play Rick Roll (YouTube)** — opens the default browser to a YouTube link.
- **Set Warning Wallpaper** — downloads an image and sets it as desktop wallpaper. May need a restart to fully apply, and the target needs to be online to pull the image.
- **Grab Wi-Fi Passwords** — reads saved Wi-Fi profiles off the machine and sends them back over HTTP to the ESP32's own log endpoint (or a custom remote URL / RequestBin if you set one in the UI).
- **Hydra God Mode** — runs the bundled prank script (wallpaper swap + sound).

All five payloads live in [`bt_payload_attack.cpp`](https://github.com/SameerAlSahab/Hydra-ESP/blob/main/main/bt/bt_payload_attack.cpp) as small keystroke-injection functions (`do_notepad_sequence`, `do_payload_2` .. `do_payload_5`), wired into a switch statement in `execute_current_payload()`. Adding your own payload is basically: write a new `do_payload_N` function that presses/releases keys through `s_keyboard`, add a `case N:` for it in that switch, and add a button for it in the web UI (`data/index.html` + `data/app.js`, same pattern as the existing five). Standalone scripts the payloads fetch and run (like the two `.ps1` files already there) live in [`/payloads`](https://github.com/SameerAlSahab/Hydra-ESP/tree/main/payloads) in the repo root — that's the place to drop new `.ps1` scripts or other files a payload needs to pull down. PRs adding new payloads or payload files are welcome, see [CONTRIBUTING.md](CONTRIBUTING.md).

---

### Deauth Attack Detector
Puts the radio into promiscuous 802.11 monitor mode and watches raw management frames. Flags anything with 10+ deauth frames from one BSSID inside a second — the usual signature of a deauth attack — and also flags broadcast deauth (`00:00:00:00:00:00` source). Shows up live in a log table on the web UI.

---

## Web interface

Live at `http://192.168.4.1` once you're on the device's AP.

- **Scan** — nearby networks, SSID/BSSID/signal. Tap a row to target it.
- **Attack** — pick and launch any attack above, see live status/timer/results.
- **Detector** — start/stop the deauth monitor, view the alert log.
- **Settings** — change management AP SSID/password (reboots on save).
- **About** — firmware version, credits, legal notice.

---

## Default credentials

| Field    | Default       |
|----------|---------------|
| SSID     | `hydra`       |
| Password | `notforfun`   |
| Web UI   | `192.168.4.1` |

Change these from Settings — they're saved to NVS flash and survive reboots.

---

## Building it yourself

Needs [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) set up (this was built/tested against the standard ESP32 target).

```bash
git clone https://github.com/SameerAlSahab/Hydra-ESP.git
cd Hydra-ESP
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Web UI assets in `data/` get flashed to SPIFFS separately — check the `partitions.csv` layout and use your IDF version's SPIFFS flashing step.

---

## Credits

| Role | Name |
|---|---|
| Lead Developer | Sameer Al Sahab |
| Original Codebase | [risinek](https://github.com/risinek/esp32-wifi-penetration-tool) |
| Inspiration | [spacehuhn](https://github.com/SpacehuhnTech/esp8266_deauther) |
| BLE Spam Code | [justcallmekoko and ckcr4lyf](https://github.com/ckcr4lyf/EvilAppleJuice-ESP32) |

---

## Citation

There's a paper alongside this repo (`paper/hydra_esp_paper.pdf`) walking through the 802.11/BLE background for everything above — how each attack actually works at the protocol level, and the countermeasures that fix the root cause (802.11w, WPA3-SAE, rogue-AP detection, tighter BT pairing). If you're citing the firmware or the paper, use the `CITATION.cff` in this repo (GitHub's "Cite this repository" button picks it up automatically), or the BibTeX below:

```bibtex
@article{alsahab2026hydraesp,
  author  = {Al Sahab, Sameer},
  title   = {Low-Cost IEEE 802.11 and Bluetooth Attack Surfaces on Commodity Microcontrollers: A Case Study of the ESP32 Platform},
  year    = {2026},
  url     = {https://github.com/SameerAlSahab/Hydra-ESP/blob/main/paper/hydra_esp_paper.pdf},
  note    = {Firmware and paper available in this repository.}
}
```

---

## Contributing

Got a bug, an idea, or a module you want to add? Check [CONTRIBUTING.md](CONTRIBUTING.md). Questions, ideas, "does this work on X board" type stuff — that goes in [Discussions](https://github.com/SameerAlSahab/Hydra-ESP/discussions), not Issues.

---

## Legal

Educational security research tool. Only run this against networks/devices you own or have explicit written permission to test. Unauthorised use is illegal under the Computer Fraud and Abuse Act (US), Computer Misuse Act (UK), IT Act 2000 (Bangladesh/India), and equivalent laws pretty much everywhere else.

Authors accept no liability for misuse. Whatever you do with it is on you.
