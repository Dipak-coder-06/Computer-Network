#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#define PORT        9090
#define BUF_SIZE    65507
#define CHUNK_SIZE  60000
#define MAX_FILENAME 256
#define MAX_TEXT    4096
#define TIMEOUT_SEC 1

typedef struct {
    char type[8];
    char category[8];
    char filename[MAX_FILENAME];
    long file_size;
    int  seq;
} __attribute__((packed)) PacketHeader;

static long fsize(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 ? st.st_size : -1;
}

static const char* category_of(const char *n) {
    const char *e = strrchr(n, '.');
    if (!e) return "FILE";
    if (!strcmp(e,".mp3")||!strcmp(e,".wav")||!strcmp(e,".ogg")||
        !strcmp(e,".m4a")||!strcmp(e,".flac")) return "AUDIO";
    if (!strcmp(e,".mp4")||!strcmp(e,".avi")||!strcmp(e,".mkv")||
        !strcmp(e,".mov")||!strcmp(e,".webm")) return "VIDEO";
    return "FILE";
}

static void set_timeout(int s, int sec) {
    struct timeval tv = { sec, 0 };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

static void send_text(int sock, struct sockaddr_in *srv, const char *msg) {
    char buf[BUF_SIZE];
    PacketHeader *h = (PacketHeader*)buf;
    memset(h, 0, sizeof(*h));
    strcpy(h->type, "TEXT");
    strcpy(h->category, "TEXT");

    int mlen = strlen(msg);
    if (mlen > MAX_TEXT - 1) mlen = MAX_TEXT - 1;
    memcpy(buf + sizeof(*h), msg, mlen);

    sendto(sock, buf, sizeof(*h) + mlen, 0, (struct sockaddr*)srv, sizeof(*srv));
    printf("[TEXT] Sent: %s\n", msg);

    set_timeout(sock, 2);
    char reply[128];
    socklen_t sl = sizeof(*srv);
    ssize_t n = recvfrom(sock, reply, sizeof(reply)-1, 0,
                         (struct sockaddr*)srv, &sl);
    if (n > 0) { reply[n] = 0; printf("[TEXT] Server: %s\n", reply); }
    else printf("[TEXT] No ack.\n");
}

static void send_file(int sock, struct sockaddr_in *srv, const char *path) {
    long total = fsize(path);
    if (total < 0) { printf("Cannot stat %s\n", path); return; }

    const char *cat = category_of(path);
    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;

    printf("\n[%s] Sending %s (%ld bytes)\n", cat, base, total);

    char buf[BUF_SIZE];
    PacketHeader *h = (PacketHeader*)buf;
    memset(h, 0, sizeof(*h));
    strcpy(h->type, "META");
    strcpy(h->category, cat);
    strncpy(h->filename, base, MAX_FILENAME - 1);
    h->file_size = total;

    sendto(sock, buf, sizeof(*h), 0, (struct sockaddr*)srv, sizeof(*srv));

    set_timeout(sock, 3);
    char reply[128];
    socklen_t sl = sizeof(*srv);
    ssize_t n = recvfrom(sock, reply, sizeof(reply)-1, 0,
                         (struct sockaddr*)srv, &sl);
    if (n <= 0) { printf("[%s] No READY, aborting.\n", cat); return; }
    reply[n] = 0;
    printf("[%s] Server: %s\n", cat, reply);

    FILE *fp = fopen(path, "rb");
    if (!fp) { perror("fopen"); return; }

    int seq = 0;
    long sent = 0;

    while (sent < total) {
        int want = (total - sent) < CHUNK_SIZE ? (int)(total - sent) : CHUNK_SIZE;
        size_t r = fread(buf + sizeof(*h), 1, want, fp);
        if (r == 0) break;

        memset(h, 0, sizeof(*h));
        strcpy(h->type, "DATA");
        strcpy(h->category, cat);
        h->seq = seq;

        int acked = 0;
        for (int attempt = 0; attempt < 10 && !acked; attempt++) {
            sendto(sock, buf, sizeof(*h) + r, 0,
                   (struct sockaddr*)srv, sizeof(*srv));

            set_timeout(sock, TIMEOUT_SEC);
            char ack[64];
            socklen_t al = sizeof(*srv);
            ssize_t an = recvfrom(sock, ack, sizeof(ack)-1, 0,
                                  (struct sockaddr*)srv, &al);
            if (an > 0) {
                ack[an] = 0;
                if (strncmp(ack, "ACK:", 4) == 0 && atoi(ack+4) == seq) {
                    acked = 1;
                }
            }
        }

        if (!acked) { printf("\n[!] Seq %d failed.\n", seq); fclose(fp); return; }

        sent += r;
        seq++;
        printf("\r[%s] %ld/%ld (%.1f%%)  ", cat, sent, total,
               100.0 * sent / total);
        fflush(stdout);
    }
    fclose(fp);

    memset(h, 0, sizeof(*h));
    strcpy(h->type, "END");
    h->seq = seq;
    sendto(sock, buf, sizeof(*h), 0, (struct sockaddr*)srv, sizeof(*srv));

    set_timeout(sock, 3);
    n = recvfrom(sock, reply, sizeof(reply)-1, 0,
                 (struct sockaddr*)srv, &sl);
    if (n > 0) { reply[n] = 0; printf("\n[%s] Server: %s\n", cat, reply); }
    printf("[%s] Complete.\n", cat);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    int sndbuf = 4 * 1024 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &srv.sin_addr);

    printf("============================================\n");
    printf("   UDP Multi-Service Client\n");
    printf("   Target: %s:%d\n", argv[1], PORT);
    printf("============================================\n");

    int choice;
    char input[BUF_SIZE];

    while (1) {
        printf("\n===== UDP Menu =====\n");
        printf("1. Send Text Message\n");
        printf("2. Send File\n");
        printf("3. Send Audio\n");
        printf("4. Send Video\n");
        printf("5. Exit\n");
        printf("Choice: ");

        if (scanf("%d", &choice) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        getchar();

        switch (choice) {
            case 1:
                printf("Message: ");
                if (!fgets(input, BUF_SIZE, stdin)) break;
                input[strcspn(input, "\n")] = 0;
                send_text(sock, &srv, input);
                break;
            case 2: case 3: case 4:
                printf("Path to file: ");
                if (!fgets(input, BUF_SIZE, stdin)) break;
                input[strcspn(input, "\n")] = 0;
                send_file(sock, &srv, input);
                break;
            case 5:
                close(sock); printf("Bye.\n"); return 0;
            default: printf("Invalid.\n");
        }
    }
    close(sock);
    return 0;
}