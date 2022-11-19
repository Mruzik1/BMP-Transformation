#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"


struct bmp_header* read_bmp_header(FILE* stream) {

    if (stream == NULL || !stream) return NULL;

    struct bmp_header* header = calloc(sizeof(struct bmp_header), 1);

    fseek(stream, 0, SEEK_SET);

    fread(&header->type, sizeof(uint16_t), 1, stream);
    if (header->type != 0x4d42) {
        
        fprintf(stderr, "Error: This is not a BMP file.\n");
        free(header);
        return NULL;

    }
    fseek(stream, 0, SEEK_SET);
    fread(header, sizeof(struct bmp_header), 1, stream);

    return header;
}


struct pixel* read_data(FILE* stream, const struct bmp_header* header) {

    if (stream == NULL || header == NULL) return NULL;
    
    uint32_t size = header->width*header->height;
    struct pixel* pixels = (struct pixel*) calloc(sizeof(struct pixel), size);
    unsigned padding = (4-(header->width*3)%4)%4;
    int idx = 0;

    fseek(stream, header->offset, SEEK_SET);
    for (int i = 0; i < header->height; ++i) {
        for (int j = 0; j < header->width; ++j) {
            fread(&pixels[idx], 3, 1, stream);
            if (pixels[idx].blue > 255 || pixels[idx].green > 255 || pixels[idx].red > 255) {

                fprintf(stderr, "Error: Corrupted BMP file.\n");
                free(pixels);
                return NULL;

            }
            idx++;
        }
        
        fseek(stream, padding, SEEK_CUR);

    }

    return pixels;
}


struct bmp_image* read_bmp(FILE* stream) {

    struct bmp_header* header = read_bmp_header(stream);
    struct pixel* bmp_data = read_data(stream, header);

    if (header == NULL || bmp_data == NULL) {

        if (header != NULL) free(header);
        if (bmp_data != NULL) free(bmp_data);

        return NULL;

    }

    struct bmp_image* image = calloc(sizeof(struct bmp_image), 1);

    image->header = header;
    image->data = bmp_data;

    return image;
}


bool write_bmp(FILE* stream, const struct bmp_image* image) {

    if (stream == NULL || image == NULL) return false;

    fwrite(image->header, sizeof(struct bmp_header), 1, stream);

    unsigned padding = (4-(image->header->width*3)%4)%4;
    int idx = 0;

    for (int i = 0; i < image->header->height; ++i) {
        for (int j = 0; j < image->header->width; ++j) {
            fwrite(&image->data[idx], sizeof(struct pixel), 1, stream);
            idx++;
        }
        fwrite(PADDING_CHAR, padding, 1, stream);
    }

    return true;
}


void free_bmp_image(struct bmp_image* image) {

    if (image == NULL) return;

    free(image->header);
    free(image->data);
    free(image);
}