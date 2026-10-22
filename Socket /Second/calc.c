#include <stdio.h>

int main() {
    float a, b;
    char op;

    printf("Enter first number: ");
    scanf("%f", &a);

    printf("Enter operator (+ - * /): ");
    scanf(" %c", &op);

    printf("Enter second number: ");
    scanf("%f", &b);

    if (op == '+')
        printf("Answer = %.2f\n", a + b);
    else if (op == '-')
        printf("Answer = %.2f\n", a - b);
    else if (op == '*')
        printf("Answer = %.2f\n", a * b);
    else if (op == '/') {
        if (b == 0)
            printf("Cannot divide by zero!\n");
        else
            printf("Answer = %.2f\n", a / b);
    }
    else
        printf("Invalid operator!\n");

    return 0;
}