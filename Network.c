#include <stdio.h>

int main() {
    int a, b, c, d;

    printf("Enter IP Address (e.g. 192.168.1.10): ");
    scanf("%d.%d.%d.%d", &a, &b, &c, &d);

    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255) {
        printf("Invalid IP Address\n");
        return 0;
    }

    printf("\n");

    if (a >= 1 && a <= 126) {
        printf("Class Name        : A\n");
        printf("Subnet Mask       : 255.0.0.0\n");
        printf("First IP Address  : %d.0.0.0\n", a);
        printf("Last IP Address   : %d.255.255.255\n", a);
        printf("Last Working IP   : %d.255.255.254\n", a);
        printf("Broadcast Address : %d.255.255.255\n", a);
    }

    else if (a >= 128 && a <= 191) {
        printf("Class Name        : B\n");
        printf("Subnet Mask       : 255.255.0.0\n");
        printf("First IP Address  : %d.%d.0.0\n", a, b);
        printf("Last IP Address   : %d.%d.255.255\n", a, b);
        printf("Last Working IP   : %d.%d.255.254\n", a, b);
        printf("Broadcast Address : %d.%d.255.255\n", a, b);
    }

    else if (a >= 192 && a <= 223) {
        printf("Class Name        : C\n");
        printf("Subnet Mask       : 255.255.255.0\n");
        printf("First IP Address  : %d.%d.%d.0\n", a, b, c);
        printf("Last IP Address   : %d.%d.%d.255\n", a, b, c);
        printf("Last Working IP   : %d.%d.%d.254\n", a, b, c);
        printf("Broadcast Address : %d.%d.%d.255\n", a, b, c);
    }

    else if (a >= 224 && a <= 239) {
        printf("Class Name        : D (Multicast)\n");
    }

    else if (a >= 240 && a <= 255) {
        printf("Class Name        : E (Experimental)\n");
    }else if(a == 127){
        printf("Class Name        : Loopback\n");
    }

    else {
        printf("Invalid IP Address\n");
        return 0;
    }

    if ((a == 10) ||
        (a == 172 && b >= 16 && b <= 31) ||
        (a == 192 && b == 168))
        printf("Type              : Private\n");
    else
        printf("Type              : Public\n");

    return 0;
}