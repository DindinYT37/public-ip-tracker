# IP Logger

A Windows system tray application that monitors and logs changes to your public IP address.

## Features

- Runs in system tray with minimal resource usage
- Automatically checks public IP every 5 seconds
- Logs IP changes with timestamps to CSV file
- Multiple IP checking services for reliability
- Right-click menu to view or open log file
- Supports both IPv4 and IPv6 addresses

## Installation

1. Download the latest release from the Releases page
2. Run the executable
3. The application will start in your system tray

## Building from Source

### Prerequisites

- MinGW with g++
- Visual Studio Build Tools 2022
- Visual Studio Code (recommended)

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/DindinYT37/public-ip-tracker.git
   cd ip-logger
   ```

2. Open in VS Code and build:
   - Press F5 for debug build
   - Run "build" task for release build

## Usage

1. Run the application
2. Look for the IP Logger icon in your system tray
3. Right-click the icon for options:
   - Reveal CSV: Opens Explorer with the log file selected
   - Open CSV: Opens the log file in your default CSV application
   - Exit: Closes the application

The log file (ip_log.csv) will be created in the same directory as the executable.

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