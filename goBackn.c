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

        // Send frames within the current window
        for (int i = base;
             i < base + windowSize && i < totalFrames;
             i++) {

            printf("Sending Frame: %d\n", i);
        }

        // Check if lost frame is in the current window
        if (lostFrame >= base &&
            lostFrame < base + windowSize &&
            lostFrame < totalFrames) {

            printf("\nFrame %d is lost!\n", lostFrame);

            printf("Receiver expects frame %d\n", lostFrame);

            printf("\nGo-Back-N: Retransmitting from frame %d\n",
                   lostFrame);

            // Retransmit lost frame and all frames after it
            for (int i = lostFrame;
                 i < base + windowSize && i < totalFrames;
                 i++) {

                printf("Retransmitting Frame: %d\n", i);
            }

            // Move the window forward
            base = base + windowSize;

            // Prevent the same frame from being lost again
            lostFrame = -1;
        }
        else {

            printf("\nAll frames received successfully.\n");

            // Slide the window forward
            base = base + windowSize;
        }
    }

    printf("\nAll frames transmitted successfully!\n");

    return 0;
}