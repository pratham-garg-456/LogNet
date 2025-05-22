# UNX511 - Assignment 3: Embedded Debug Logging

**Short Name:** LogNet

## Overview

This project implements an embedded debug logging system—**LogNet**—designed to help track and diagnose issues that occur in deployed software. The system is composed of two main parts:
- **Logger**: A client-side library (`Logger.cpp`, `Logger.h`) included in application processes to generate and send logs.
- **Log Server**: A server application (`LogServer.cpp`) that receives, filters, and stores log messages from multiple clients for future reference and debugging.

The solution uses **UDP** for fast, non-blocking log message delivery and supports runtime configuration of log filtering.

---

## Features & Key Functionalities

### Logger (Client Side)
- **Log Message Format**: Each log contains:
  - Log severity (DEBUG, WARNING, ERROR, CRITICAL)
  - File name (`__FILE__`)
  - Function name (`__func__`)
  - Line number (`__LINE__`)
  - Timestamp
  - Custom message
- **Log Filtering**: Only logs at or above the configured severity filter are sent to the server.
- **Fast, Non-blocking Delivery**: Uses non-blocking UDP sockets to avoid slowing down the application.
- **Threaded Server Command Listener**: Receives server commands (e.g., to update log level) on a separate thread.
- **Mutex Protection**: All shared resources (e.g., log buffer, log level variable) are protected with mutexes for thread safety.
- **API Functions**:
  - `int InitializeLog();` — Set up logging (socket, thread, mutex).
  - `void SetLogLevel(LOG_LEVEL level);` — Set minimum log severity.
  - `void Log(LOG_LEVEL level, const char *file, const char *func, int line, const char *message);` — Send a log message.
  - `void ExitLog();` — Clean up (close socket, destroy thread/mutex).
- **Integration**: Include `Logger.h` and add `Logger.cpp` to your build system. Typical use in code:
  ```cpp
  Log(DEBUG, __FILE__, __func__, __LINE__, "Created the objects");
  ```

### Log Server
- **UDP Receiver**: Binds to a configurable IP/port, receives log messages from clients.
- **Central Log File**: Writes all received logs to a single file (`server log file`).
- **User Menu**:
  1. **Set Log Level**: Update filter severity for connected clients (broadcasts command via UDP).
  2. **Dump Log File**: Display log file contents on screen.
  3. **Shutdown**: Gracefully terminate server and threads.
- **Threaded Receiver**: Runs log collection on a separate thread for responsiveness.
- **Mutex Protection**: Ensures thread-safe access to the log file.
- **Command Handling**: Receives and processes commands from the user or clients.

---

## Directory Structure

- `Logger.h`, `Logger.cpp` — Client-side logging library.
- `TravelSimulator.cpp`, `Automobile.cpp`, `Automobile.h` — Sample application using the logger.
- `Makefile` — For building the client application.
- `LogServer.cpp` — Server application source.
- `Makefile` — For building the server.

---

## How It Works

1. **Start the Log Server** on one machine. Note its IP and port.
2. **Start the Application (e.g., TravelSimulator)** on another machine, configured to send logs to the server's IP/port.
3. Application logs events using the Logger API. Logs are filtered and sent via UDP.
4. Server receives logs, writes them to a file, and offers options to change filter level or view logs in real-time.
5. Server can broadcast commands (like changing log level) to all clients.

---

## Testing

- Verify logs are sent and received with all required fields.
- Change filter levels from the server and observe log filtering in real time.
- Use the server's menu to dump logs and check their format and completeness.
- Ensure the logger does not impact application performance.

---

## Notes

- Ensure correct IP/port configuration between client and server.
- The logger/server are designed for demonstration and educational purposes. For production, consider enhancements for security, reliability, and scalability.

---

## Authors

- UNX511 Assignment 3
- Contributors: [Your Name(s) Here]