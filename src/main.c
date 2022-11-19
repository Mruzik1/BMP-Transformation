#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "transformations.h"


int main(int argc, char** argv) {

    float s = (float)atoi(argv[1])/100.0f;
    
    FILE* fp = fopen("assets/lenna.bmp", "rb");
    struct bmp_image* image = read_bmp(fp);
    struct bmp_image* scaleImage = scale(image, s);

    FILE* result = fopen("assets/result.bmp", "wb");
    write_bmp(result, scaleImage);

    if (fp != NULL) fclose(fp);
    fclose(result);
    free_bmp_image(scaleImage);
    free_bmp_image(image);

    return 0;
}