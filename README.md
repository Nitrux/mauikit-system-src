# MauiKit System

![](https://mauikit.org/wp-content/uploads/2018/12/maui_project_logo.png)

_Shared desktop-integration libraries for the Maui stack._

This repository currently provides the following modules:

- `audio`: PipeWire/WirePlumber session integration and volume feedback
- `network`: NetworkManager integration for connection management and status
- `notifications`: Freedesktop notifications integration
- `power`: power-management, battery, and brightness integration

## Runtime notes

- The `network` module uses NetworkManager/ModemManager directly and discovers VPN support from NetworkManager plugin metadata.
- The `bluetooth` module talks to BlueZ D-Bus services directly for discovery/pairing and OBEX file transfer (`org.bluez.obex` / `obexd`).
- `network` secret storage is backend-driven: NM-owned and keychain backends are both available; keychain is preferred when enabled (`MAUIKIT_SYSTEM_NETWORK_USE_KEYCHAIN`) and available.
- Keychain persistence uses QtKeychain (service `org.mauikit.system.network`, key format `<uuid>/<setting>/<secret-key>`). When both stores hold a value during transitions, keychain values are treated as authoritative.

# Issues

If you find problems with the contents of this repository, please create an issue and use the **🐞 Bug report** template.

## Submitting a bug report

Before submitting a bug, you should look at the [existing bug reports](https://github.com/Nitrux/mauikit-system-src/issues) to verify that no one has reported the bug already.

©2026 Nitrux Latinoamericana S.C.
