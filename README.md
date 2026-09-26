<table>
<tr>
<td width="160" align="center">
<img src="resources/hydra_logo.png" alt="Hydra-ESP logo" width="140"/>
</td>
<td>

# Hydra-ESP
**wifi + bt test tool for the ESP32**

Built on top of [risinek's](https://github.com/risinek/esp32-wifi-penetration-tool) esp32-wifi-penetration-tool. Redesigned web UI, multi-target deauth, BLE attacks, a deauth detector, optional OLED, and a bunch of extra modules on top of the original.

</td>
</tr>
</table>

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
<b>v2 (new, current build)</b> — 13K+ views
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
Raw 802.11 deauth frames to boot clients off a target AP. Up to 16 targets at once. Devices with 802.11w (MFP) enabled can resist plain frame-injection deauth — see BSSID Clone below for a way around that.

**Methods:** deauth frames only / deauth + disassociation frames

---

### WPA Handshake Capture
Forces reconnects with deauth frames, captures the resulting WPA2 4-way handshake, saves it as `.pcap` and `.hccapx` for offline auditing with Hashcat / aircrack-ng against a wordlist. Needs a connected client on the target network. Runs until it gets a handshake or you stop it.

---

### Clientless PMKID Capture
Grabs the PMKID off the first EAPOL-Key frame during association — no connected client needed, the AP just has to be in range. Works with Hashcat hash mode 22000. Most modern WPA2 APs leak this, some don't include it at all.

---

### Beacon Spam
Floods the area with fake 802.11 beacon frames, each with a randomly generated SSID. Trashes the Wi-Fi scan list on every nearby device. 1–100 fake networks, configurable.

---

### Ghost Mode (Probe Request Spam)
Listens for probe requests devices send out for their saved networks, grabs the SSIDs, then starts advertising those exact names back. Devices try to connect to the ESP32 instead of their real saved network.

---

### Evil Twin
Spins up an open clone of the target AP (same SSID, no password) while deauthing the real one to push clients off it. Clients see the open clone, connect, get hit with a captive portal asking for the network password. Runs until it gets and verifies a submission. Web UI is down for the duration — power cycle to stop if no timeout is set.

---

### BSSID Clone (Twin Deauth)
Clones the target's SSID *and* BSSID onto the ESP32 on the same channel, so the real AP and the clone look identical. The address collision kicks clients off on its own — and unlike raw deauth frames, this still works against 802.11w/MFP devices since it's not relying on unprotected management frames.

---

### SSID Cloner
Spins up multiple clones sharing (near-)identical SSIDs by padding the name with spaces.

---

### BLE Spam
Broadcasts BLE advertisement packets mimicking Apple/Samsung/Google proximity pairing signals — iPhones, iPads, Android devices pop up pairing prompts for nearby "devices". Target type is selectable, MAC rotation supported.

Supports: AirPods (all gens), AirPods Pro (all gens), AirPods Max, Beats, Apple TV setup/pairing, HomePod setup, Vision Pro, Galaxy Buds (all variants), Pixel Buds, or random.

---

### BT Payload
Advertises the ESP32 as a BT HID keyboard (`Hydra-<random>`). Once a Windows box pairs with it, the firmware fires off keystrokes to run the payload.

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
