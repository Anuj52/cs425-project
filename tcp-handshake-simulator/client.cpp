#include <iostream>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"

#define CLIENT_SYN_SEQ 200
#define CLIENT_ACK_SEQ 600
#define SERVER_SYN_SEQ 400
#define PACKET_SIZE (sizeof(struct iphdr) + sizeof(struct tcphdr))

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) error_exit("Socket creation failed");

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
        error_exit("setsockopt() failed");

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(SERVER_PORT);
    dest.sin_addr.s_addr = inet_addr(SERVER_IP);

    char packet[PACKET_SIZE];

    // Step 1: Send SYN
    memset(packet, 0, PACKET_SIZE);
    struct iphdr *iph = (struct iphdr *)packet;
    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));

    iph->ihl = 5;
    iph->version = 4;
    iph->tot_len = htons(PACKET_SIZE);
    iph->id = htons(54321);
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->saddr = inet_addr("127.0.0.1");
    iph->daddr = inet_addr(SERVER_IP);
    iph->check = 0;

    uint16_t src_port = 54321;
    tcph->source = htons(src_port);
    tcph->dest = htons(SERVER_PORT);
    tcph->seq = htonl(CLIENT_SYN_SEQ);
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->syn = 1;
    tcph->window = htons(8192);
    tcph->check = 0;
    tcph->urg_ptr = 0;

    if (sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
        error_exit("sendto() failed for SYN");
    else
        std::cout << "[+] Sent SYN, seq=" << CLIENT_SYN_SEQ << std::endl;

    // Step 2: Receive SYN-ACK
    char recv_buffer[65536];
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len = sizeof(recv_addr);
    bool synack_received = false;
    uint32_t server_seq = 0;

    while (!synack_received) {
        int data_size = recvfrom(sock, recv_buffer, sizeof(recv_buffer), 0,
                                 (struct sockaddr *)&recv_addr, &recv_addr_len);
        if (data_size < 0) continue;

        struct iphdr *recv_iph = (struct iphdr *)recv_buffer;
        struct tcphdr *recv_tcph = (struct tcphdr *)(recv_buffer + (recv_iph->ihl * 4));

        if (ntohs(recv_tcph->dest) != src_port) continue;
        if (recv_tcph->syn && recv_tcph->ack &&
            ntohl(recv_tcph->ack_seq) == CLIENT_SYN_SEQ + 1) {
            std::cout << "[+] Received SYN-ACK" << std::endl;
            server_seq = ntohl(recv_tcph->seq);
            synack_received = true;
        }
    }

    // Step 3: Send ACK
    memset(packet, 0, PACKET_SIZE);
    iph = (struct iphdr *)packet;
    tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));

    iph->ihl = 5;
    iph->version = 4;
    iph->tot_len = htons(PACKET_SIZE);
    iph->id = htons(54321);
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->saddr = inet_addr("127.0.0.1");
    iph->daddr = inet_addr(SERVER_IP);
    iph->check = 0;

    tcph->source = htons(src_port);
    tcph->dest = htons(SERVER_PORT);
    tcph->seq = htonl(CLIENT_ACK_SEQ);
    tcph->ack_seq = htonl(server_seq + 1);
    tcph->doff = 5;
    tcph->ack = 1;
    tcph->window = htons(8192);
    tcph->check = 0;
    tcph->urg_ptr = 0;

    if (sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
        error_exit("sendto() failed for ACK");
    else
        std::cout << "[+] Sent ACK, seq=" << CLIENT_ACK_SEQ 
                  << ", ack=" << (server_seq + 1) << std::endl;

    std::cout << "[+] Handshake complete." << std::endl;
    close(sock);
    return 0;
}
