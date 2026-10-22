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

    int default_mask_octets = 0;
    char class_name = ' ';

    if (a >= 1 && a <= 126) {
        class_name = 'A';
        default_mask_octets = 1; // 255.0.0.0
    }
    else if (a >= 128 && a <= 191) {
        class_name = 'B';
        default_mask_octets = 2; // 255.255.0.0
    }
    else if (a >= 192 && a <= 223) {
        class_name = 'C';
        default_mask_octets = 3; // 255.255.255.0
    }
    else if (a >= 224 && a <= 239) {
        printf("Class Name        : D (Multicast)\n");
        return 0;
    }
    else if (a >= 240 && a <= 255) {
        printf("Class Name        : E (Experimental)\n");
        return 0;
    }
    else if (a == 127) {
        printf("Class Name        : Loopback\n");
        return 0;
    }
    else {
        printf("Invalid IP Address\n");
        return 0;
    }

    if (bits_needed > 8) {
        printf("Cannot borrow more than 8 bits in a single octet for simple subnetting.\n");
        return 0;
    }

    // IP Type
    if ((a == 10) || (a == 172 && b >= 16 && b <= 31) || (a == 192 && b == 168))
        printf("Type              : Private\n");
    else
        printf("Type              : Public\n");

    printf("Class Name        : %c\n", class_name);

    // 3. Block Size Calculation
    int block_size = 256 / total_subnets;
    int custom_octet_value = 256 - block_size;

    // 4. Subnet Mask Construction
    int mask_a = 255, mask_b = 0, mask_c = 0, mask_d = 0;

    if (class_name == 'C') {
        mask_b = 255;
        mask_c = 255;
        mask_d = custom_octet_value;
    } 
    else if (class_name == 'B') {
        mask_b = 255;
        mask_c = custom_octet_value;
        mask_d = 0;
    } 
    else if (class_name == 'A') {
        mask_b = custom_octet_value;
        mask_c = 0;
        mask_d = 0;
    }

    int default_cidr = default_mask_octets * 8;
    int new_cidr = default_cidr + bits_needed;

    printf("Default CIDR      : /%d\n", default_cidr);
    printf("New Subnet Mask   : %d.%d.%d.%d (/%d)\n", mask_a, mask_b, mask_c, mask_d, new_cidr);
    printf("Bits Borrowed     : %d\n", bits_needed);
    printf("Block Size        : %d\n", block_size);

    // 5. Formatted Output Table
    printf("\n--------------------------------------------------------------------------------------------------\n");
    printf("%-8s %-20s %-20s %-20s %-20s\n", "Subnet", "Network IP", "First Usable IP", "Last Usable IP", "Broadcast IP");
    printf("--------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < total_subnets; i++) {
        int start_val = i * block_size;
        int end_val = start_val + block_size - 1;

        char net_ip[30], first_ip[30], last_ip[30], bcast_ip[30];

        if (class_name == 'C') {
            snprintf(net_ip, sizeof(net_ip), "%d.%d.%d.%d", a, b, c, start_val);
            snprintf(first_ip, sizeof(first_ip), "%d.%d.%d.%d", a, b, c, start_val + 1);
            snprintf(last_ip, sizeof(last_ip), "%d.%d.%d.%d", a, b, c, end_val - 1);
            snprintf(bcast_ip, sizeof(bcast_ip), "%d.%d.%d.%d", a, b, c, end_val);
        }
        else if (class_name == 'B') {
            snprintf(net_ip, sizeof(net_ip), "%d.%d.%d.0", a, b, start_val);
            snprintf(first_ip, sizeof(first_ip), "%d.%d.%d.1", a, b, start_val);
            snprintf(last_ip, sizeof(last_ip), "%d.%d.%d.254", a, b, end_val);
            snprintf(bcast_ip, sizeof(bcast_ip), "%d.%d.%d.255", a, b, end_val);
        }
        else if (class_name == 'A') {
            snprintf(net_ip, sizeof(net_ip), "%d.%d.0.0", a, start_val);
            snprintf(first_ip, sizeof(first_ip), "%d.%d.0.1", a, start_val);
            snprintf(last_ip, sizeof(last_ip), "%d.%d.255.254", a, end_val);
            snprintf(bcast_ip, sizeof(bcast_ip), "%d.%d.255.255", a, end_val);
        }

        printf("%-8d %-20s %-20s %-20s %-20s\n", i + 1, net_ip, first_ip, last_ip, bcast_ip);
    }
    printf("--------------------------------------------------------------------------------------------------\n");

    return 0;
}