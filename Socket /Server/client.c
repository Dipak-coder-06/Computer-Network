#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 4096

static void send_all(int fd, const void *data, size_t len) {
    const char *p = (const char *)data;
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, p + sent, len - sent, 0);
        if (n <= 0) return;
        sent += n;
    }
}
static void send_str(int fd, const char *s) { send_all(fd, s, strlen(s)); }

static ssize_t recv_line(int fd, char *buf, size_t max) {
    size_t i = 0;
    while (i < max - 1) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);
        if (n <= 0) return n;
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return (ssize_t)i;
}

/* ============================================================
 *  SERVICE 1: Hello
 * ============================================================ */
static void do_hello(int fd) {
    char buf[BUF_SIZE];

    printf("Enter your hello message: ");
    if (!fgets(buf, BUF_SIZE, stdin)) return;
    buf[strcspn(buf, "\n")] = '\0';

    send_str(fd, "HELLO\n");
    send_str(fd, buf);
    send_str(fd, "\n");

    recv_line(fd, buf, BUF_SIZE);
    printf("Server says: %s\n", buf);
}

/* ============================================================
 *  SERVICE 2: File Transfer
 * ============================================================ */
static void do_file(int fd) {
    char filename[BUF_SIZE];
    char buf[BUF_SIZE];

    printf("Enter filename to request: ");
    if (!fgets(filename, BUF_SIZE, stdin)) return;
    filename[strcspn(filename, "\n")] = '\0';

    send_str(fd, "FILE\n");
    send_str(fd, filename);
    send_str(fd, "\n");

    recv_line(fd, buf, BUF_SIZE);
    if (strncmp(buf, "ERROR", 5) == 0) {
        printf("Server: file not found.\n");
        return;
    }

    long file_size = 0;
    if (sscanf(buf, "OK %ld", &file_size) != 1) {
        printf("Bad response: %s\n", buf);
        return;
    }

    printf("\n----- Receiving %ld bytes -----\n", file_size);

    long received = 0;
    FILE *out = fopen("received.txt", "wb");
    if (!out) { perror("fopen"); return; }

    while (received < file_size) {
        size_t want = (file_size - received) < BUF_SIZE
                      ? (size_t)(file_size - received)
                      : BUF_SIZE;
        ssize_t n = recv(fd, buf, want, 0);
        if (n <= 0) { printf("Connection closed early.\n"); break; }
        fwrite(buf, 1, n, out);
        fwrite(buf, 1, n, stdout);
        received += n;
    }
    fclose(out);

    printf("\n----- Done. %ld/%ld bytes -----\n", received, file_size);
    printf("Saved as: received.txt\n");
}

/* ============================================================
 *  SERVICE 3: Arithmetic
 * ============================================================ */
static void do_calc(int fd) {
    char buf[BUF_SIZE];
    char expr[BUF_SIZE];

    printf("Enter expression (e.g. 10 + 5): ");
    if (!fgets(expr, BUF_SIZE, stdin)) return;
    expr[strcspn(expr, "\n")] = '\0';

    send_str(fd, "CALC\n");
    send_str(fd, expr);
    send_str(fd, "\n");

    recv_line(fd, buf, BUF_SIZE);
    printf("Result: %s\n", buf);
}

/* ============================================================
 *  SERVICE 4: Trigonometry  (supports compound expressions)
 * ============================================================ */
static void do_trig(int fd) {
    char buf[BUF_SIZE];
    char expr[BUF_SIZE];

    printf("Enter trig expression (e.g. sin(40) - tan(90) + 2): ");
    if (!fgets(expr, BUF_SIZE, stdin)) return;
    expr[strcspn(expr, "\n")] = '\0';

    send_str(fd, "TRIG\n");
    send_str(fd, expr);
    send_str(fd, "\n");

    recv_line(fd, buf, BUF_SIZE);
    if (strncmp(buf, "ERROR", 5) == 0)
        printf("Result: undefined  (%s)\n", buf + 7);
    else
        printf("Result: %s\n", buf);
}

/* ============================================================
 *  MAIN
 * ============================================================ */
int main(void) {
    int sock_fd;
    struct sockaddr_in addr;
    int choice;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); return 1; }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect"); return 1;
    }
    printf("Connected to server.\n");

    while (1) {
        printf("\n===== TCP Service Menu =====\n");
        printf("1. Say Hello\n");
        printf("2. File Transfer\n");
        printf("3. Calculator (Arithmetic)\n");
        printf("4. Calculator (Trigonometry)\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Invalid input. Try again.\n");
            continue;
        }
        getchar(); /* consume newline after scanf */

        switch (choice) {
            case 1: do_hello(sock_fd); break;
            case 2: do_file(sock_fd);  break;
            case 3: do_calc(sock_fd);  break;
            case 4: do_trig(sock_fd);  break;
            case 5:
                send_str(sock_fd, "BYE\n");
                close(sock_fd);
                printf("Disconnected.\n");
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }

    close(sock_fd);
    return 0;
}