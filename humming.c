#include <stdio.h>

int main() {
    int n;
    printf("Enter number of data bits: ");
    scanf("%d", &n);

    int data[100];
    printf("Enter the data bits (0 or 1): ");
    for (int i = 0; i < n; i++) {
        if(n > 100){
            printf("Number of data bits should not exceed 100.\n");
            return 0;
        }
        scanf("%d", &data[i]);
    }


    int r = 0;
    int p = 1; 
    while (p < n + r + 1) {
        r++;
        p = p * 2;
    }

    int total_len = n + r;
    int codeword[100];

    int data_idx = 0;
    for (int i = 1; i <= total_len; i++) {
        // Check if i is a power of 2
        int temp = i;
        while (temp > 1 && temp % 2 == 0) {
            temp = temp / 2;
        }

        if (temp == 1) {
            codeword[i] = 0; // Parity position
        } else {
            codeword[i] = data[data_idx]; 
            data_idx++;
        }
    }

    for (int i = 0; i < r; i++) {
        // Calculate parity bit position (2^i)
        int parity_pos = 1;
        for (int k = 0; k < i; k++) {
            parity_pos = parity_pos * 2;
        }

        int sum = 0;
        for (int j = 1; j <= total_len; j++) {
            if ((j / parity_pos) % 2 == 1) {
                sum = sum + codeword[j];
            }
        }

        // Set parity bit (even parity)
        codeword[parity_pos] = sum % 2;
    }

    printf("Hamming Codeword: ");
    for (int i = 1; i <= total_len; i++) {
        printf("%d ", codeword[i]);
    }
    printf("\n");


    return 0;
}