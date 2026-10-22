#include <stdio.h>

int main() {
    int a, b, c, d;

    printf("Enter IP Address (e.g. 192.168.1.10): ");
    if (scanf("%d.%d.%d.%d", &a, &b, &c, &d) != 4) {
        printf("Invalid IP Address format\n");
        return 0;
    }

    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255) {
        printf("Invalid IP Address\n");
        return 0;
    }

    printf("\nHow many Subnets do you want to create? ");
    int subnets;
    scanf("%d", &subnets);

    if (subnets <= 0) {
        printf("Number of subnets must be greater than 0.\n");
        return 0;
    }

    int bits_needed = 0;
    int total_subnets = 1;
    while (total_subnets < subnets) {
        total_subnets *= 2;
        bits_needed++;
    }   

    printf("\n");

    if (bits_needed > 8) {
        printf("Cannot borrow more than 8 bits in a single octet for simple subnetting.\n");
        return 0;
    }

    // IP Type
    if ((a == 10) || (a == 172 && b >= 16 && b <= 31) || (a == 192 && b == 168))
        printf("Type              : Private\n");
    else
        printf("Type              : Public\n");
        

    int block_size = 256 / total_subnets;
    int custom_octet_value = 256 - block_size;



    // 5. Print Subnet Details
    printf("\n--------------------------------------------------------------------------------------------------\n");
    printf("%-8s %-18s %-18s %-18s %-18s\n", "Subnet", "Network IP", "First Usable IP", "Last Usable IP", "Broadcast IP");
    printf("--------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < total_subnets; i++) {
        int start_val = i * block_size;
        int end_val = start_val + block_size - 1;

        if (class_name == 'C') {
            printf("%-8d %d.%d.%d.%-7d %d.%d.%d.%-7d %d.%d.%d.%-7d %d.%d.%d.%-7d\n",
                i + 1,
                a, b, c, start_val,
                a, b, c, start_val + 1,
                a, b, c, end_val - 1,
                a, b, c, end_val);
        }
        else if (class_name == 'B') {
            printf("%-8d %d.%d.%d.0% -8s %d.%d.%d.1% -8s %d.%d.%d.254 %d.%d.%d.255\n",
                i + 1,
                a, b, start_val, "",
                a, b, start_val, "",
                a, b, end_val,
                a, b, end_val);
        }
        else if (class_name == 'A') {
            printf("%-8d %d.%d.0.0% -10s %d.%d.0.1% -10s %d.%d.255.254 %d.%d.255.255\n",
                i + 1,
                a, start_val, "",
                a, start_val, "",
                a, end_val,
                a, end_val);
        }
    }
    printf("--------------------------------------------------------------------------------------------------\n");

    return 0;
}