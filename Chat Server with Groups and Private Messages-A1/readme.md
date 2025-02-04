# Chat Application

This is a multi-client chat application implemented in C++ using sockets and threads. The chat server supports various commands for private messaging, broadcast messaging, group management, and group messaging. User credentials are stored in a `user.txt` file.

## Features

- **Private Messaging:**  
  `/msg <username> <message>`  
  Send a private message to a specific online user.

- **Broadcast Messaging:**  
  `/broadcast <message>`  
  Send a message to all connected users (except the sender).

- **Group Management:**
  - **Create Group:**  
    `/create group <group name>`  
    Create a new group. The creator is automatically added as a member.
  - **Join Group:**  
    `/join group <group name>`  
    Join an existing group.
  - **Leave Group:**  
    `/leave group <group name>`  
    Leave a group if you are a member.

- **Group Messaging:**  
  `/group msg <group name> <message>`  
  Send a message to all members of a group (if you are a member).

- **Exit:**  
  `/exit`  
  Disconnect from the server.

## Files

- **server_file.cpp:** The server implementation.
- **client_grp.cpp:** The client implementation.
- **user.txt:** A text file containing valid user credentials in the format `username:password` (one per line).
- **Makefile:** A Makefile to compile the project easily.

## Requirements

- A C++ compiler supporting C++20 (e.g., `g++` version 9 or later)
- A POSIX-compliant operating system (e.g., Linux or macOS)
- Networking support (sockets) and threading support (`pthread`)

## Compilation

You can compile the project using the provided Makefile or manually compile using the command line.

### Using Makefile

Run the following command in the project directory:

```bash
make
