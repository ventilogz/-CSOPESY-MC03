# CSOPESY Semi-Major Output 1 — OS Emulator

## Members
- Lance Krystofer Galicia
- Raina Helaga
- Maurienne Marie Mojica
- Venice Raeka Plurad

**Entry file:** `console_ui.cpp` (contains `main()`)

## How to Compile

From a terminal (Command Prompt, PowerShell, or a Linux/Mac terminal) in the same folder as `console_ui.cpp`, run:

```
g++ -std=c++17 -Wall -Wextra -o console_ui.exe console_ui.cpp
```

## How to Run

```
console_ui.exe
```

On Mac/Linux: `./console_ui.exe`

## Current Status

This build currently contains the **Console & UI module** only:

- Startup screen (ASCII header, group names, version date)
- Main command loop with the `Command>` prompt
- Screen formatting / refresh handling (`clearScreen`, `refreshScreen`)

**Recognized commands at this stage:**

| Command | Behavior |
|---|---|
| `exit` | Terminates the program |
| *(anything else)* | Echoed back as a placeholder until the full command interpreter is merged in |

**Still being integrated by other team members:**

- Command interpreter — `help`, `start_marquee`, `stop_marquee`, `set_text`, `set_speed`
- Marquee animation engine

## Notes

- Update the **Members** list above if names change.
- Once the command interpreter and marquee engine are merged in, update **Current Status** to reflect the full command set and remove this note.