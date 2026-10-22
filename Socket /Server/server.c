#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <ctype.h>

#define PORT 8080
#define BUF_SIZE 4096
#define EPS 1e-6
#define PI 3.14159265358979323846

/* ---------- Send exactly len bytes ---------- */
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

/* ---------- Receive one line ---------- */
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
 *  SERVICE 1: Say Hello
 * ============================================================ */
static void service_hello(int fd) {
    char buf[BUF_SIZE];
    recv_line(fd, buf, BUF_SIZE);
    printf("[HELLO] Client says: %s\n", buf);
    send_str(fd, "Hello Client! Nice to meet you.\n");
    printf("[HELLO] Server replied.\n");
}

/* ============================================================
 *  SERVICE 2: File Transfer
 * ============================================================ */
static void service_file(int fd) {
    char filename[BUF_SIZE];
    char buf[BUF_SIZE];
    FILE *fp;
    long file_size;

    recv_line(fd, filename, BUF_SIZE);
    printf("[FILE] Client requested: %s\n", filename);

    fp = fopen(filename, "rb");
    if (!fp) { send_str(fd, "ERROR\n"); printf("[FILE] Not found.\n"); return; }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    snprintf(buf, BUF_SIZE, "OK %ld\n", file_size);
    send_str(fd, buf);
    printf("[FILE] Sending %ld bytes...\n", file_size);

    size_t n; long sent_total = 0;
    while ((n = fread(buf, 1, BUF_SIZE, fp)) > 0) {
        send_all(fd, buf, n);
        sent_total += (long)n;
    }
    fclose(fp);
    printf("[FILE] Sent %ld bytes.\n", sent_total);
}

/* ============================================================
 *  SERVICE 3: Arithmetic Calculator
 * ============================================================ */
static void service_calc(int fd) {
    char buf[BUF_SIZE];
    float a, b, result;
    char op;

    recv_line(fd, buf, BUF_SIZE);
    if (sscanf(buf, "%f %c %f", &a, &op, &b) != 3) {
        send_str(fd, "ERROR: bad expression\n"); return;
    }
    printf("[CALC] %.2f %c %.2f\n", a, op, b);

    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (fabs(b) < EPS) { send_str(fd, "ERROR: Division by zero\n"); return; }
            result = a / b; break;
        default:
            send_str(fd, "ERROR: Invalid operator\n"); return;
    }
    snprintf(buf, BUF_SIZE, "%.4f\n", result);
    send_str(fd, buf);
    printf("[CALC] Result: %.4f\n", result);
}

/* ============================================================
 *  SERVICE 4: Trigonometric Calculator
 *  Supports compound expressions:
 *      sin(30) + cos(60) - tan(45) * 2 / 5
 * ============================================================ */

/* --- Angle checks (degrees) --- */
static int is_multiple_of_180(float deg) {
    float r = fmodf(deg, 180.0f);
    if (r < 0) r += 180.0f;
    return (r < 1e-3f || r > 180.0f - 1e-3f);
}
static int is_odd_multiple_of_90(float deg) {
    float r = fmodf(deg - 90.0f, 180.0f);
    if (r < 0) r += 180.0f;
    return (r < 1e-3f || r > 180.0f - 1e-3f);
}

/* Evaluate one func(angle) token.
 * Returns: 1 ok, 0 undefined, -1 unknown function. */
static int eval_trig_token(const char *func, float angle, float *out) {
    float rad = angle * PI / 180.0f;

    if (strcmp(func, "sin") == 0)         *out = sinf(rad);
    else if (strcmp(func, "cos") == 0)    *out = cosf(rad);
    else if (strcmp(func, "tan") == 0) {
        if (is_odd_multiple_of_90(angle)) return 0;
        *out = tanf(rad);
    }
    else if (strcmp(func, "cosec") == 0 || strcmp(func, "csc") == 0) {
        if (is_multiple_of_180(angle)) return 0;
        *out = 1.0f / sinf(rad);
    }
    else if (strcmp(func, "sec") == 0) {
        if (is_odd_multiple_of_90(angle)) return 0;
        *out = 1.0f / cosf(rad);
    }
    else if (strcmp(func, "cot") == 0) {
        if (is_multiple_of_180(angle)) return 0;
        *out = 1.0f / tanf(rad);
    }
    else return -1;

    if (fabsf(*out) < 1e-6f) *out = 0.0f;
    return 1;
}

/* ---------- Trig expression parser ----------
 * Grammar:
 *   expr   := term  (('+' | '-') term)*
 *   term   := factor (('*' | '/') factor)*
 *   factor := func '(' number ')' | number | '(' expr ')'
 * We use simple global state for recursion.
 */
typedef struct {
    const char *p;
    int undefined;   /* set to 1 if any sub-func is undefined */
    int error;       /* set to 1 on syntax error */
} TrigParser;

static float parse_expr(TrigParser *tp);

static void skip_spaces(TrigParser *tp) {
    while (*tp->p == ' ' || *tp->p == '\t') tp->p++;
}

static float parse_factor(TrigParser *tp) {
    skip_spaces(tp);

    /* function: sin/cos/tan/sec/cosec/csc/cot */
    if (isalpha((unsigned char)*tp->p)) {
        char func[16]; int j = 0;
        while (isalpha((unsigned char)*tp->p) && j < 15)
            func[j++] = *tp->p++;
        func[j] = '\0';

        skip_spaces(tp);
        if (*tp->p != '(') { tp->error = 1; return 0; }
        tp->p++;  /* skip '(' */

        skip_spaces(tp);
        char *end;
        float angle = strtof(tp->p, &end);
        if (end == tp->p) { tp->error = 1; return 0; }
        tp->p = end;

        skip_spaces(tp);
        if (*tp->p != ')') { tp->error = 1; return 0; }
        tp->p++;  /* skip ')' */

        float val;
        int st = eval_trig_token(func, angle, &val);
        if (st == 0) { tp->undefined = 1; return 0; }
        if (st == -1) { tp->error = 1; return 0; }
        return val;
    }

    /* parenthesized sub-expression */
    if (*tp->p == '(') {
        tp->p++;
        float v = parse_expr(tp);
        skip_spaces(tp);
        if (*tp->p != ')') { tp->error = 1; return 0; }
        tp->p++;
        return v;
    }

    /* plain number */
    char *end;
    float v = strtof(tp->p, &end);
    if (end == tp->p) { tp->error = 1; return 0; }
    tp->p = end;
    return v;
}

static float parse_term(TrigParser *tp) {
    float v = parse_factor(tp);
    while (1) {
        skip_spaces(tp);
        if (*tp->p == '*') {
            tp->p++;
            v *= parse_factor(tp);
        } else if (*tp->p == '/') {
            tp->p++;
            float d = parse_factor(tp);
            if (fabsf(d) < EPS) { tp->undefined = 1; return 0; }
            v /= d;
        } else break;
    }
    return v;
}

static float parse_expr(TrigParser *tp) {
    float v = parse_term(tp);
    while (1) {
        skip_spaces(tp);
        if (*tp->p == '+') { tp->p++; v += parse_term(tp); }
        else if (*tp->p == '-') { tp->p++; v -= parse_term(tp); }
        else break;
    }
    return v;
}

static void service_trig(int fd) {
    char buf[BUF_SIZE];
    char expr[BUF_SIZE];

    recv_line(fd, expr, BUF_SIZE);
    printf("[TRIG] %s\n", expr);

    TrigParser tp;
    tp.p = expr;
    tp.undefined = 0;
    tp.error = 0;

    float result = parse_expr(&tp);

    skip_spaces(&tp);
    if (tp.error || *tp.p != '\0') {
        send_str(fd, "ERROR: syntax error\n");
        printf("[TRIG] Syntax error.\n");
        return;
    }
    if (tp.undefined) {
        send_str(fd, "ERROR: undefined for this angle\n");
        printf("[TRIG] Undefined.\n");
        return;
    }

    snprintf(buf, BUF_SIZE, "%.4f\n", result);
    send_str(fd, buf);
    printf("[TRIG] Result: %.4f\n", result);
}

/* ============================================================
 *  MAIN
 * ============================================================ */
int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char service[BUF_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    listen(server_fd, 5);

    printf("============================================\n");
    printf("  Multi-Service Server running on port %d\n", PORT);
    printf("  (Ctrl+C to stop)\n");
    printf("============================================\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&addr, &len);
        if (client_fd < 0) { perror("accept"); continue; }

        printf("\n[+] Client connected.\n");

        while (1) {
            ssize_t n = recv_line(client_fd, service, BUF_SIZE);
            if (n <= 0) { printf("[+] Client disconnected.\n"); break; }

            printf("[+] Service: %s\n", service);

            if (strcmp(service, "HELLO") == 0)      service_hello(client_fd);
            else if (strcmp(service, "FILE") == 0)  service_file(client_fd);
            else if (strcmp(service, "CALC") == 0)  service_calc(client_fd);
            else if (strcmp(service, "TRIG") == 0)  service_trig(client_fd);
            else if (strcmp(service, "BYE") == 0) {
                printf("[+] Client said BYE.\n");
                break;
            }
            else send_str(client_fd, "ERROR: Unknown service\n");
        }
        close(client_fd);
    }

    close(server_fd);
    return 0;
}