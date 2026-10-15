# SecureGate Pro

**Intelligent Multi-Level Biometric Access Control and Intrusion Prevention System**

Embedded firmware for an **LPC2148 (ARM7TDMI)** door-access controller. A user must pass three stages — **User ID → Password → Fingerprint** — before the door motor opens. Every event is time-stamped and streamed to a PC terminal.

## Features

- 3-factor authentication: ID, keypad password, R305 fingerprint match
- Account lockout after 3 wrong passwords or 3 fingerprint mismatches
- 30 s system lockdown after 3 invalid IDs or 3 wrong admin PINs
- Admin menu (EINT0 switch + admin PIN): add/delete users, edit passwords, enrol/delete fingerprints, unlock users, view log, set clock
- Up to 8 users (IDs up to 6 digits, passwords 4–8 digits), stored in external I2C EEPROM
- Real-time clock time-stamped event log over UART (9600 8N1)
- R305 auto-detect on UART0/UART1 at 9600 / 57600 baud
- I2C time-outs and bus recovery — the board never freezes if the EEPROM is missing

## Hardware

| Part | Purpose |
|---|---|
| LPC2148 (12 MHz crystal) | Microcontroller |
| R305 fingerprint module | Biometric verification |
| 16x2 LCD (8-bit mode) | User interface |
| 4x4 matrix keypad | ID / password / admin entry |
| AT24C256 I2C EEPROM (0x50) | User records and admin PIN |
| L293D + DC motor | Door actuator |
| MAX232 | PC serial log |
| Push switch on EINT0 | Admin menu request |

### Pin map (all defined in `src/board_config.h`)

| Pins | Function |
|---|---|
| P0.8 / P0.9 | UART1 TXD1 / RXD1 → R305 (57600 baud default) |
| P0.0 / P0.1 | UART0 TXD0 / RXD0 → PC via MAX232 (log, 9600 8N1) |
| P0.2 / P0.3 | I2C0 SCL / SDA → AT24C256 |
| P0.4 / P0.5 | L293D IN1 / IN2 |
| P0.7 | L293D ENABLE (`MOTOR_EN_ENABLED`; set 0 if EN1 tied to +5 V) |
| P0.6 | Door-closed switch, active low (`DOOR_SWITCH_ENABLED`, off by default) |
| P0.12 / P0.13 | LCD RS / EN |
| P0.16 | EINT0 admin switch |
| P1.16–P1.19 | Keypad rows |
| P1.20–P1.23 | Keypad columns |
| P1.24–P1.31 | LCD D0–D7 |

## Repository layout

```
SecureGate-Pro/
├── src/                  Application sources (.c / .h)
├── firmware/             Pre-built SecureGate_Pro.hex
├── Startup.s             Keil startup code
├── SecureGate_Pro.uvproj Keil uVision project
├── SecureGate_Pro.uvopt  Keil target options
├── LICENSE
└── README.md
```

| Module | Role |
|---|---|
| `main.c` | Boot sequence and the ID → password → fingerprint flow |
| `admin.c` | Admin menu, user management, event log |
| `r305.c` | Fingerprint module protocol |
| `eeprom.c`, `i2c.c` | Persistent storage and I2C driver (with time-outs) |
| `lcd.c`, `keypad.c` | User interface |
| `motor.c` | Door open / close / deny pulse |
| `rtc.c` | Real-time clock |
| `uart0.c`, `uart1.c`, `interrupt.c`, `delay.c` | Peripheral drivers |

## Build and flash

1. Open `SecureGate_Pro.uvproj` in **Keil uVision (MDK-ARM)**.
2. Build the target (F7). Output goes to `Objects/`.
3. Flash `SecureGate_Pro.hex` with Flash Magic (or your ISP tool), or use the pre-built file in `firmware/`.

## Usage

**Normal access:** enter ID → `#` → password → `#` → place finger. The door opens, holds, then closes.
Keypad: digits only, `#` = enter, `*` = backspace.

**Admin menu:** press the EINT0 switch and enter the admin PIN.

| Key | Action |
|---|---|
| 1 | Edit password (User ID, 0 = admin) |
| 2 | Fingerprint: 1 enrol / 2 delete by User ID / 3 delete all |
| 3 | Add user |
| 4 | Delete user |
| 5 | Exit |
| A | Unlock user |
| B | View last 25 log lines |
| C | Set clock (time `HHMMSS`, then date `DDMMYY`) |

**Log:** connect a PC terminal at 9600 8N1. Example line:

```
LOG 012: [15/10/2026 10:42:07] USER 243562 - ACCESS GRANTED
```

The clock starts at 01/01/2026 00:00:00 on a fresh power-up; set it once with admin `C` (it keeps running from the RTC battery pin).

## Configuration

Edit `src/board_config.h` for UART assignment, baud rates, R305 address/password, LCD pins, motor and door timing, and fingerprint wait time.

## Security note

The factory admin PIN is **1234**. Change it with admin option 1 before real use.

## License

MIT — see [LICENSE](LICENSE).
