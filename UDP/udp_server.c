#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>

#define PORT        9090
#define BUF_SIZE    65507
#define MAX_FILENAME 256
#define MAX_TEXT    4096

typedef struct {
    char type[8];
    char category[8];
    char filename[MAX_FILENAME];
    long file_size;
    int  seq;
} __attribute__((packed)) PacketHeader;

static const char* pick_folder(const char *c) {
    if (strcmp(c, "AUDIO") == 0) return "received_audio";
    if (strcmp(c, "VIDEO") == 0) return "received_video";
    return "received_files";
}

static void make_dir(const char *p) {
    struct stat st = {0};
    if (stat(p, &st) == -1) mkdir(p, 0755);
}

static void handle_text(const char *msg, struct sockaddr_in *cli,
                        socklen_t cl, int sock) {
    printf("\n[💬 TEXT from %s:%d]\n   %s\n",
           inet_ntoa(cli->sin_addr), ntohs(cli->sin_port), msg);
    const char *ack = "ACK:TEXT received";
    sendto(sock, ack, strlen(ack), 0, (struct sockaddr*)cli, cl);
}

static void handle_transfer(int sock, struct sockaddr_in *cli, socklen_t cl,
                            const char *category, const char *filename,
                            long file_size) {
    const char *folder = pick_folder(category);
    make_dir(folder);

    char out_path[512];
    snprintf(out_path, sizeof(out_path), "%s/%s", folder, filename);

    printf("\n[📦 %s] %s (%ld bytes) -> %s\n",
           category, filename, file_size, out_path);

    FILE *fp = fopen(out_path, "wb");
    if (!fp) {
        const char *err = "ERROR:cannot create";
        sendto(sock, err, strlen(err), 0, (struct sockaddr*)cli, cl);
        return;
    }

    char ack[128];
    snprintf(ack, sizeof(ack), "READY:%s", filename);
    sendto(sock, ack, strlen(ack), 0, (struct sockaddr*)cli, cl);

    char buf[BUF_SIZE];
    long received = 0;
    int expected = 0;
    time_t start = time(NULL);

    while (received < file_size) {
        struct sockaddr_in from;
        socklen_t fl = sizeof(from);
        ssize_t n = recvfrom(sock, buf, BUF_SIZE, 0,
                             (struct sockaddr*)&from, &fl);
        if (n < (ssize_t)sizeof(PacketHeader)) continue;

        PacketHeader *hdr = (PacketHeader*)buf;
        if (strcmp(hdr->type, "END") == 0) break;
        if (strcmp(hdr->type, "DATA") != 0) continue;
        if (hdr->seq != expected) continue;

        int payload = n - sizeof(PacketHeader);
        fwrite(buf + sizeof(PacketHeader), 1, payload, fp);
        received += payload;
        expected++;

        char a[32];
        snprintf(a, sizeof(a), "ACK:%d", hdr->seq);
        sendto(sock, a, strlen(a), 0, (struct sockaddr*)cli, cl);

        printf("\r   Progress: %ld/%ld (%.1f%%)",
               received, file_size, 100.0 * received / file_size);
        fflush(stdout);
    }
    fclose(fp);

    time_t end = time(NULL);
    double s = difftime(end, start);
    if (s < 1) s = 1;
    printf("\n[✅ Done] %s | %.1f s | %.2f KB/s\n",
           out_path, s, (received / 1024.0) / s);

    sendto(sock, "DONE", 4, 0, (struct sockaddr*)cli, cl);
}

int main(void) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    int rcvbuf = 4 * 1024 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = INADDR_ANY;
    srv.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*)&srv, sizeof(srv)) < 0) {
        perror("bind"); return 1;
    }

    char ip[64] = "unknown";
    FILE *f = popen("hostname -I | awk '{print $1}'", "r");
    if (f) { if (fgets(ip, sizeof(ip), f)) ip[strcspn(ip, "\n")] = 0; pclose(f); }

    printf("============================================\n");
    printf("   UDP Multi-Service Receiver\n");
    printf("   Port: %d\n", PORT);
    printf("   Your IP: %s\n", ip);
    printf("   Tell your friend to run:\n");
    printf("       ./udp_client %s\n", ip);
    printf("============================================\n");

    char buf[BUF_SIZE];
    struct sockaddr_in cli;
    socklen_t cl = sizeof(cli);

    while (1) {
        ssize_t n = recvfrom(sock, buf, BUF_SIZE, 0,
                             (struct sockaddr*)&cli, &cl);
        if (n < (ssize_t)sizeof(PacketHeader)) continue;

        PacketHeader *hdr = (PacketHeader*)buf;

        if (strcmp(hdr->type, "TEXT") == 0) {
            char safe[MAX_TEXT];
            int mlen = n - sizeof(PacketHeader);
            if (mlen > MAX_TEXT - 1) mlen = MAX_TEXT - 1;
            memcpy(safe, buf + sizeof(PacketHeader), mlen);
            safe[mlen] = 0;
            handle_text(safe, &cli, cl, sock);
        } else if (strcmp(hdr->type, "META") == 0) {
            handle_transfer(sock, &cli, cl,
                            hdr->category, hdr->filename, hdr->file_size);
        }
    }
    close(sock);
    return 0;
}