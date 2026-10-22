#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define PI 3.14159265358979323846
#define EPS 1e-6   /* small tolerance for "zero" */

/* -------- Compute one trig token, e.g. sin(30) -------- */
/* Returns 1 on success, 0 if undefined, -1 on error */
int compute_trig(char *token, float *out) {
    char func[10];
    float angle, rad;
    int i = 0, j = 0;

    while (isalpha(token[i])) func[j++] = token[i++];
    func[j] = '\0';

    if (token[i] == '(') i++;

    sscanf(&token[i], "%f", &angle);

    rad = angle * PI / 180.0;

    if (strcmp(func, "sin") == 0) {
        *out = sin(rad);
    }
    else if (strcmp(func, "cos") == 0) {
        *out = cos(rad);
    }
    else if (strcmp(func, "tan") == 0) {
        if (fabs(cos(rad)) < EPS) return 0;
        *out = tan(rad);
    }
    else if (strcmp(func, "cosec") == 0 || strcmp(func, "csc") == 0) {
        if (fabs(sin(rad)) < EPS) return 0;
        *out = 1.0 / sin(rad);
    }
    else if (strcmp(func, "sec") == 0) {
        if (fabs(cos(rad)) < EPS) return 0;
        *out = 1.0 / cos(rad);
    }
    else if (strcmp(func, "cot") == 0) {
        if (fabs(tan(rad)) < EPS) return 0;
        *out = 1.0 / tan(rad);
    }
    else {
        printf("Error: unknown function '%s'\n", func);
        return -1;
    }

    return 1;
}

int main() {
    char expr[200];
    float numbers[50];
    char ops[50];
    int num_count = 0, op_count = 0;
    int i = 0;

    printf("=====================================\n");
    printf("    TRIG + ARITHMETIC CALCULATOR     \n");
    printf("=====================================\n");

    printf("\nEnter expression: ");
    fgets(expr, sizeof(expr), stdin);
    expr[strcspn(expr, "\n")] = '\0';

    /* ---------- Parse ---------- */
    while (expr[i] != '\0') {

        if (expr[i] == ' ') { i++; continue; }

        /* --- trig function --- */
        if (isalpha(expr[i])) {
            char token[50];
            int j = 0;
            float val;
            int status;

            while (expr[i] != ')' && expr[i] != '\0' && expr[i] != ' ') {
                token[j++] = expr[i++];
            }
            if (expr[i] == ')') token[j++] = expr[i++];
            token[j] = '\0';

            status = compute_trig(token, &val);

            if (status == 0) {
                printf("\nAnswer = undefined  (%s does not exist)\n", token);
                return 0;
            }
            if (status == -1) return 1;

            numbers[num_count++] = val;
        }

        /* --- plain number --- */
        else if (isdigit(expr[i]) || expr[i] == '.') {
            float num;
            sscanf(&expr[i], "%f", &num);
            numbers[num_count++] = num;
            while (isdigit(expr[i]) || expr[i] == '.') i++;
        }

        /* --- operator --- */
        else if (expr[i] == '+' || expr[i] == '-' ||
                 expr[i] == '*' || expr[i] == '/') {
            ops[op_count++] = expr[i];
            i++;
        }

        else {
            printf("Error: unexpected character '%c'\n", expr[i]);
            return 1;
        }
    }

    /* ---------- Apply * and / first (BODMAS) ---------- */
    int n = 0;
    float new_nums[50];
    char new_ops[50];
    new_nums[0] = numbers[0];

    for (i = 0; i < op_count; i++) {
        if (ops[i] == '*') {
            new_nums[n] = new_nums[n] * numbers[i + 1];
        }
        else if (ops[i] == '/') {
            if (fabs(numbers[i + 1]) < EPS) {
                printf("\nAnswer = undefined  (division by zero)\n");
                return 0;
            }
            new_nums[n] = new_nums[n] / numbers[i + 1];
        }
        else {
            new_ops[n] = ops[i];
            n++;
            new_nums[n] = numbers[i + 1];
        }
    }

    /* ---------- Apply + and - ---------- */
    float result = new_nums[0];
    for (i = 0; i < n; i++) {
        if (new_ops[i] == '+') result += new_nums[i + 1];
        else if (new_ops[i] == '-') result -= new_nums[i + 1];
    }

    printf("\nAnswer = %.4f\n", result);
    return 0;
}