# Public IP Tracker

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Windows-0078D6?style=flat&logo=windows&logoColor=white)](https://github.com/DindinYT37/public-ip-tracker)
[![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)](https://isocpp.org/)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/DindinYT37/public-ip-tracker)](https://github.com/DindinYT37/public-ip-tracker/releases)

A Windows system tray application that monitors and logs changes to your public IP address.

## Overview

Public IP Tracker is a lightweight Windows utility that runs in your system tray and monitors changes to your public IP address. When a change is detected, it automatically logs the timestamp and new IP address to a CSV file for easy tracking and analysis.

## Features

- Runs in system tray with minimal resource usage
- Automatically checks public IP every 5 seconds
- Logs IP changes with timestamps to CSV file
- Multiple IP checking services for reliability
- Right-click menu to view or open log file
- Supports both IPv4 and IPv6 addresses
- Zero configuration required
- Minimal CPU and memory footprint

## Installation

1. Download the latest release from the Releases page
2. Run the executable
3. The application will start in your system tray

No installation required - just download and run.

## Building from Source

### Prerequisites

- MinGW with g++
- Visual Studio Build Tools 2022
- Visual Studio Code (recommended)

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/DindinYT37/public-ip-tracker.git
   cd public-ip-tracker
   ```

2. Open in VS Code and build:
   - Press F5 for debug build
   - Run "build" task for release build

### Build Options

- Debug build: Includes console window for debugging
- Release build: No console window, runs silently in system tray

## Usage

1. Run the application
2. Look for the IP Logger icon in your system tray
3. Right-click the icon for options:
   - Reveal CSV: Opens Explorer with the log file selected
   - Open CSV: Opens the log file in your default CSV application
   - Exit: Closes the application

The log file (ip_log.csv) will be created in the same directory as the executable.

## CSV Format

The log file uses a simple CSV format:
```csv
Timestamp,IP Address
2024-01-01 12:00:00,192.168.1.1
```

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on contributing to this project.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to various IP checking services:
  - checkip.amazonaws.com
  - api.ipify.org
  - icanhazip.com
  - ipinfo.io 