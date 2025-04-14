# 🔗 TCP 3-Way Handshake (Raw Sockets in C++)

This project demonstrates a manual implementation of the **TCP 3-way handshake** using **raw sockets** in C++. It showcases how TCP connections are established under the hood by directly constructing IP and TCP headers, bypassing the operating system’s TCP stack.

## 🧠 What is the TCP 3-Way Handshake?

The TCP 3-way handshake is the process used to establish a TCP connection between a client and a server:

1. **SYN**: Client sends a SYN (synchronize) packet to the server to initiate a connection.
2. **SYN-ACK**: Server responds with a SYN-ACK (synchronize-acknowledgment) packet.
3. **ACK**: Client sends an ACK (acknowledgment) packet to finalize the connection.

---

## 🎯 Project Goal

The goal is to simulate this handshake manually using raw packets with the following hardcoded values (as per assignment spec):

| Step        | Sender  | Sequence Number | Acknowledgment Number |
|-------------|---------|------------------|------------------------|
| SYN         | Client  | 200              | —                      |
| SYN-ACK     | Server  | 400              | 201                    |
| ACK         | Client  | 600              | 401                    |

---

## 📁 Project Structure

- `server.cpp` – C++ code for the server to handle TCP packets.
- `client.cpp` – C++ code for the client to initiate the handshake.
- `Makefile` – Build automation for compiling the project.
- `README.md` – This documentation.

---

## ⚙️ How to Build & Run

> 🛑 **Warning**: Requires root/admin privileges and a Linux environment. Not recommended on public networks.

### 🧪 Build

```bash
make 
```
## 🚀 Run

To start the server :

```bash
sudo make run-server
```
To start the client:

```bash
sudo make run-client
```
## 🛠️ Implementation Details
Raw Sockets: Used to bypass the OS TCP stack and manually construct TCP/IP headers.
Hardcoded Values: Sequence and acknowledgment numbers are hardcoded for simplicity.
Error Handling: Basic error handling is implemented for socket creation and packet sending.