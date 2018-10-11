#include <stdio.h>

char *inputFilename;
char *outputFilename;

int skipIndices844[] = {
    0, 19, 19, 19, 19, 19, 19, 19,
    19, 21, 21, 21,
    21, 27, 27, 27
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s [h04.hft] [h04.han]\n", argv[0]);
        return 0;
    }

    inputFilename = argv[1];
    outputFilename = argv[2];

    FILE *input = fopen(inputFilename, "r");
    FILE *output = fopen(outputFilename, "w");

    int index = 0;
    int skipIndex = 0;
    int skipRemain = skipIndices844[skipIndex++];
    while (!feof(input)) {
        unsigned char buf[32] = {0, };

        if (skipRemain == 0) {
            fwrite(buf, 32, 1, output);
            skipRemain = skipIndices844[skipIndex++];
        } else {
            fread(buf, 32, 1, input);
            fwrite(buf, 32, 1, output);
            skipRemain--;
        }
    }

    fclose(input);
    fclose(output);
}