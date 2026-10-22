#include <stdio.h>

int main() {

    int windowSize, totalFrames, lostFrame;

    printf("Enter Window Size\n");
    printf("------------------------------\n");
    scanf("%d", &windowSize);

    printf("Enter Total Frames\n");
    printf("------------------------------\n");
    scanf("%d", &totalFrames);

    printf("Enter Lost Frame\n");
    printf("------------------------------\n");
    scanf("%d", &lostFrame);

    int base = 0;

    while (base < totalFrames) {

        printf("\nCurrent Window:\n");

        // Send all frames in the current window
        for (int i = base;
             i < base + windowSize && i < totalFrames;
             i++) {

            printf("Sending Frame: %d\n", i);

            // Simulate frame loss
            if (i == lostFrame) {
                printf("Frame %d is lost!\n", i);
            }
        }

        // Check if lost frame is in the current window
        if (lostFrame >= base &&
            lostFrame < base + windowSize &&
            lostFrame < totalFrames) {

            printf("\nReceiver detected missing Frame: %d\n",
                   lostFrame);

            // Selective Repeat:
            // Only the lost frame is retransmitted
            printf("Selective Repeat: Retransmitting Frame: %d\n",
                   lostFrame);

            printf("Frame %d received successfully after retransmission.\n",
                   lostFrame);

            // Prevent the same frame from being lost again
            lostFrame = -1;
        }
        else {

            printf("\nAll frames received successfully.\n");
        }

        // Move the sliding window forward
        base += windowSize;
    }

    printf("\nAll frames transmitted successfully!\n");

    return 0;
}