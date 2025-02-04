#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#define BUFFER_SIZE 1024

// Global mutexes for thread-safe access.
std::mutex clients_mutex;
std::mutex groups_mutex;

// Global data structures.
std::unordered_map<int, std::string>
    clients;  // Maps client sockets to usernames.
std::unordered_map<std::string, std::string>
    users;  // Stores username:password pairs.
std::unordered_map<std::string, std::unordered_set<int>>
    groups;  // Maps group names to sets of client sockets (members).

// Utility function to trim whitespace from both ends of a string.
std::string trim(const std::string &s) {
  size_t start = s.find_first_not_of(" \n\r\t");
  size_t end = s.find_last_not_of(" \n\r\t");
  if (start == std::string::npos) return "";
  return s.substr(start, end - start + 1);
}

// Loads user credentials from a file (each line formatted as
// username:password).
void load_users(const std::string &filename) {
  std::ifstream infile(filename);
  if (!infile.is_open()) {
    std::cerr << "Could not open user file: " << filename << std::endl;
    return;
  }
  std::string line;
  while (std::getline(infile, line)) {
    if (line.empty()) continue;
    std::istringstream iss(line);
    std::string username, password;
    if (std::getline(iss, username, ':') && std::getline(iss, password)) {
      username = trim(username);
      password = trim(password);
      users[username] = password;
    }
  }
  infile.close();
}

void handle_client(int client_socket) {
  char buffer[BUFFER_SIZE];

  // --- Authentication ---
  std::string username_prompt = "Enter username: ";
  send(client_socket, username_prompt.c_str(), username_prompt.size(), 0);
  memset(buffer, 0, BUFFER_SIZE);
  int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
  if (bytes_received <= 0) {
    close(client_socket);
    return;
  }
  std::string username(buffer, bytes_received);
  username = trim(username);

  std::string password_prompt = "Enter password: ";
  send(client_socket, password_prompt.c_str(), password_prompt.size(), 0);
  memset(buffer, 0, BUFFER_SIZE);
  bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
  if (bytes_received <= 0) {
    close(client_socket);
    return;
  }
  std::string password(buffer, bytes_received);
  password = trim(password);

  bool authenticated = false;
  if (users.find(username) != users.end() && users[username] == password) {
    authenticated = true;
  }
  if (!authenticated) {
    std::string fail = "Authentication failed.\n";
    send(client_socket, fail.c_str(), fail.size(), 0);
    close(client_socket);
    return;
  }

  // Add client to active list.
  {
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients[client_socket] = username;
  }

  // Notify other clients that a new user has joined.
  std::string join_msg = username + " has joined the chat.\n";
  {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &p : clients) {
      if (p.first != client_socket)
        send(p.first, join_msg.c_str(), join_msg.size(), 0);
    }
  }

  std::string welcome = "Welcome to the chat server!\n";
  send(client_socket, welcome.c_str(), welcome.size(), 0);

  // --- Main command-processing loop ---
  while (true) {
    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) break;

    std::string msg(buffer, bytes_received);
    msg = trim(msg);
    if (msg.empty()) continue;

    // 1. Private Message: /msg <username> <message>
    if (msg.rfind("/msg ", 0) == 0) {
      size_t first_space = msg.find(' ');
      size_t second_space = msg.find(' ', first_space + 1);
      if (first_space == std::string::npos ||
          second_space == std::string::npos) {
        std::string error =
            "Invalid command. Usage: /msg <username> <message>\n";
        send(client_socket, error.c_str(), error.size(), 0);
      } else {
        std::string target_user =
            msg.substr(first_space + 1, second_space - first_space - 1);
        std::string message_text = msg.substr(second_space + 1);
        int target_socket = -1;
        {
          std::lock_guard<std::mutex> lock(clients_mutex);
          for (const auto &p : clients) {
            if (p.second == target_user) {
              target_socket = p.first;
              break;
            }
          }
        }
        if (target_socket == -1) {
          std::string error = "User " + target_user + " is not online.\n";
          send(client_socket, error.c_str(), error.size(), 0);
        } else {
          std::string sender = clients[client_socket];
          std::string private_msg =
              "[" + sender + "] (private): " + message_text + "\n";
          send(target_socket, private_msg.c_str(), private_msg.size(), 0);
        }
      }
    }
    // 2. Broadcast Message: /broadcast <message>
    else if (msg.rfind("/broadcast ", 0) == 0) {
      std::string message_text = msg.substr(strlen("/broadcast "));
      std::string sender = clients[client_socket];
      std::string broadcast_msg =
          "[broadcast] " + sender + ": " + message_text + "\n";
      {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto &p : clients) {
          if (p.first != client_socket)
            send(p.first, broadcast_msg.c_str(), broadcast_msg.size(), 0);
        }
      }
    }
    // 3. Create Group: /create group <group name>
    else if (msg.rfind("/create group ", 0) == 0) {
      std::string group_name = trim(msg.substr(strlen("/create group ")));
      if (group_name.empty()) {
        std::string error =
            "Group name cannot be empty. Usage: /create group <group name>\n";
        send(client_socket, error.c_str(), error.size(), 0);
      } else {
        std::lock_guard<std::mutex> lock(groups_mutex);
        if (groups.find(group_name) != groups.end()) {
          std::string error = "Group '" + group_name + "' already exists.\n";
          send(client_socket, error.c_str(), error.size(), 0);
        } else {
          // Create the group and add the client as the first member.
          groups[group_name] = std::unordered_set<int>();
          groups[group_name].insert(client_socket);
          std::string info =
              "Group '" + group_name + "' created and you have joined it.\n";
          send(client_socket, info.c_str(), info.size(), 0);
        }
      }
    }
    // 4. Join Group: /join group <group name>
    else if (msg.rfind("/join group ", 0) == 0) {
      std::string group_name = trim(msg.substr(strlen("/join group ")));
      if (group_name.empty()) {
        std::string error =
            "Group name cannot be empty. Usage: /join group <group name>\n";
        send(client_socket, error.c_str(), error.size(), 0);
      } else {
        std::lock_guard<std::mutex> lock(groups_mutex);
        if (groups.find(group_name) == groups.end()) {
          std::string error = "Group '" + group_name + "' does not exist.\n";
          send(client_socket, error.c_str(), error.size(), 0);
        } else {
          groups[group_name].insert(client_socket);
          std::string info = "You have joined group '" + group_name + "'.\n";
          send(client_socket, info.c_str(), info.size(), 0);
        }
      }
    }
    // 5. Leave Group: /leave group <group name>
    else if (msg.rfind("/leave group ", 0) == 0) {
      std::string group_name = trim(msg.substr(strlen("/leave group ")));
      if (group_name.empty()) {
        std::string error =
            "Group name cannot be empty. Usage: /leave group <group name>\n";
        send(client_socket, error.c_str(), error.size(), 0);
      } else {
        std::lock_guard<std::mutex> lock(groups_mutex);
        if (groups.find(group_name) == groups.end()) {
          std::string error = "Group '" + group_name + "' does not exist.\n";
          send(client_socket, error.c_str(), error.size(), 0);
        } else {
          if (groups[group_name].find(client_socket) !=
              groups[group_name].end()) {
            groups[group_name].erase(client_socket);
            std::string info = "You have left group '" + group_name + "'.\n";
            send(client_socket, info.c_str(), info.size(), 0);
          } else {
            std::string error =
                "You are not a member of group '" + group_name + "'.\n";
            send(client_socket, error.c_str(), error.size(), 0);
          }
        }
      }
    }
    // 6. Group Message: /group msg <group name> <message>
    else if (msg.rfind("/group msg ", 0) == 0) {
      // Find the first space after the command prefix to separate group name
      // from message.
      size_t pos = msg.find(' ', strlen("/group msg "));
      if (pos == std::string::npos) {
        std::string error =
            "Invalid command. Usage: /group msg <group name> <message>\n";
        send(client_socket, error.c_str(), error.size(), 0);
      } else {
        std::string group_name =
            msg.substr(strlen("/group msg "), pos - strlen("/group msg "));
        std::string group_message = trim(msg.substr(pos + 1));
        std::lock_guard<std::mutex> lock(groups_mutex);
        if (groups.find(group_name) == groups.end()) {
          std::string error = "Group '" + group_name + "' does not exist.\n";
          send(client_socket, error.c_str(), error.size(), 0);
        } else if (groups[group_name].find(client_socket) ==
                   groups[group_name].end()) {
          std::string error =
              "You are not a member of group '" + group_name + "'.\n";
          send(client_socket, error.c_str(), error.size(), 0);
        } else {
          std::string sender = clients[client_socket];
          std::string full_message = "[group " + group_name + "] " + sender +
                                     ": " + group_message + "\n";
          // Send the message to every group member except the sender.
          for (int sock : groups[group_name]) {
            if (sock != client_socket) {
              send(sock, full_message.c_str(), full_message.size(), 0);
            }
          }
        }
      }
    }

    // Unknown command.
    else {
      std::string error = "Unknown command.\n";
      send(client_socket, error.c_str(), error.size(), 0);
    }
  }

  // --- Client Disconnecting ---
  std::string left_username;
  {
    std::lock_guard<std::mutex> lock(clients_mutex);
    left_username = clients[client_socket];
    clients.erase(client_socket);
  }
  close(client_socket);
  std::string left_msg = left_username + " has left the chat.\n";
  {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &p : clients)
      send(p.first, left_msg.c_str(), left_msg.size(), 0);
  }
}

void start_server(int port) {
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }
  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  if (listen(server_socket, 10) < 0) {
    perror("Listen failed");
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  std::cout << "Server is listening on port " << port << std::endl;
  while (true) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket =
        accept(server_socket, (sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
      perror("Accept failed");
      continue;
    }
    std::thread(handle_client, client_socket).detach();
  }
  close(server_socket);
}

int main() {
  // Load user credentials from "user.txt"
  load_users("users.txt");

  int port = 8080;
  start_server(port);
  return 0;
}