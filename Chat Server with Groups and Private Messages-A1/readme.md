# Chat Application

This is a multi-client chat application implemented in C++ using sockets and threads. The chat server supports various commands for private messaging, broadcast messaging, group management, and group messaging. User credentials are stored in a `users.txt` file.

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
```

**Run the Server Executable:**  
   - On **Linux** :
     ```bash
     ./server_file
     ```
     

### 2. Start a Client

1. **Open a New Terminal Tab:**  
- In the integrated terminal panel in VS Code, click the plus sign (+) to open a new terminal tab.

2. **Run the Client Executable:**  
- On **Linux** :
  ```bash
  ./client_grp
  ```

**Authentication:**  
- The client will prompt for a username and password.
- Enter a valid username and password combination (for example, if your `user.txt` contains `alice:password123`, use those credentials).

Running Additional Clients

To simulate multiple users (which is useful for testing private messaging, group messaging, etc.):

2. **Run the Client Executable in Each Tab:**  
- Follow the same steps as above:
  - Linux/macOS: `./client_grp`
- Log in with different valid credentials (e.g., one client as `alice`, another as `bob`, etc.).

Now you can test commands such as:
- **Private Messaging:** `/msg bob Hello Bob!`
- **Broadcast Messaging:** `/broadcast Good morning everyone!`
- **Group Commands:** Create, join, leave, and send group messages using the corresponding commands.

### Example Terminal Output

- **Server Terminal:**
Server is listening on port 8080


- **Client Terminal (Alice):**
Connected to the server. Enter username: alice Enter password: password123 Welcome to the chat server!


- **Client Terminal (Bob):**
Connected to the server. Enter username: bob Enter password: qwerty456 alice has joined the chat. Welcome to the chat server!
