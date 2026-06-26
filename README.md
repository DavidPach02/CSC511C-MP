# CSC511C-MP

A C++ command-line OS scheduler simulator with FCFS and RR scheduling, process screens, reporting, and sleep instruction support.

## Authors

- Andre Garcia
- David Pacheco
- Julius Rosario

## Entry Class
CSC511C

## Project Structure

- `CSC511C-MP/` - C++ source code
- `data/config.txt` - runtime configuration file

## Requirements

### Windows

- A C++17 compiler (for example `g++` from MSYS2/MinGW)
- Terminal/PowerShell

### Linux/macOS

- `clang++` or `g++` with C++17 support
- `make`

## Configuration

Edit `data/config.txt` before running:

```txt
num-cpu 4
scheduler fcfs
quantum-cycles 5
batch-process-freq 1
delays-per-exec 2
min-ins 25
max-ins 100
```

## Build and Run

### Option 1: Manual Build on Windows (g++)

From `CSC511C-MP/`:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -g *.cpp -o CSC511C-MP.exe
./CSC511C-MP.exe
```

### Option 2: Makefile on Linux/macOS

From `CSC511C-MP/`:

```bash
make
./simpleos
```

## How to Start the Application

If the user is using Visual Studio in debugging the code.

1. Open VS Studio 2022 or any version higher.
2. Find CSC511C-MP.cpp in the Solution Explorer.
3. Press the debug button.

When launched, the app opens in preboot mode.

1. Type `initialize`.
2. After initialization, you will enter the main console.
3. Use `help` to see available commands.

Common commands:

- `scheduler-start` - starts scheduler test and process generation
- `scheduler-stop` - stops process generation
- `screen -ls` - shows process and CPU summary
- `report-util` - writes report to `data/csopesy-log.txt`
- `exit` - exits the application

## Notes

- The application reads config from `../data/config.txt` relative to the executable location under `CSC511C-MP/`.
- If build fails with "Permission denied" on `CSC511C-MP.exe`, close any running instance of the app and rebuild.

