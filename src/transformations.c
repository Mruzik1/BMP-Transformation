#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "transformations.h"


struct bmp_image* bmp_copy(const struct bmp_image* image, uint32_t width, uint32_t height) {

    struct bmp_image* result = malloc(sizeof(struct bmp_image));
    result->header = malloc(sizeof(struct bmp_header));
    result->data = calloc(sizeof(struct pixel), height*width);

    memcpy(result->header, image->header, sizeof(struct bmp_header));
    if (width <= image->header->width && height <= image->header->height)
        memcpy(result->data, image->data, sizeof(struct pixel)*width*height);
    result->header->width = width;
    result->header->height = height;


    return result;
}


struct bmp_image* flip_horizontally(const struct bmp_image* image) {

    if (image == NULL) return NULL;

    struct bmp_image* result = bmp_copy(image, image->header->width, image->header->height);

    for (int i = 0; i < result->header->height; ++i) {

        int idx = (int)(result->header->width*(uint32_t)i);
        int mirrorIdx = (int)((uint32_t)idx+result->header->width-1);

        for (int j = 0; j < result->header->width; ++j) {

            if (idx == mirrorIdx) break;

            result->data[idx] = image->data[mirrorIdx];
            result->data[mirrorIdx] = image->data[idx];

            if (!(result->header->width%2) && idx == mirrorIdx-1) break;

            idx++;
            mirrorIdx--;

        }

    }

    return result;
}


struct bmp_image* flip_vertically(const struct bmp_image* image) {

    if (image == NULL) return NULL;

    struct bmp_image* result = bmp_copy(image, image->header->width, image->header->height);

    for (int i = 0; i < result->header->width; ++i) {

        int step = (int)result->header->width;
        int idx = i;
        int mirrorIdx = (int)((uint32_t)step*(result->header->height-1))+i;

        for (int j = 0; j < result->header->height; ++j) {

            if (idx == mirrorIdx) break;

            result->data[idx] = image->data[mirrorIdx];
            result->data[mirrorIdx] = image->data[idx];

            if (!(result->header->height%2) && idx == mirrorIdx-step) break;

            idx += step;
            mirrorIdx -= step;

        }

    }

    return result;
}


struct bmp_image* rotate_left(const struct bmp_image* image) {

    if (image == NULL) return NULL;

    struct bmp_image* result = bmp_copy(image, image->header->height, image->header->width);

    uint32_t padding_size = ((4-(result->header->width*3)%4)%4)*result->header->height;
    result->header->image_size = padding_size+result->header->width*result->header->height*3;
    result->header->size = result->header->image_size+result->header->offset;

    for (int i = 0; i < result->header->height; ++i) {

        int idx = (int)((uint32_t)i*result->header->width);
        int newIdx = (int)(image->header->width*(image->header->height-1))+i;

        for (int j = 0; j < result->header->width; ++j) {

            result->data[idx] = image->data[newIdx];

            idx++;
            newIdx -= (int)image->header->width;

        }
    }

    return result;
}


struct bmp_image* rotate_right(const struct bmp_image* image) {

    if (image == NULL) return NULL;

    struct bmp_image* stepRotate = rotate_left(image);
    struct bmp_image* stepVertival = flip_vertically(stepRotate);
    struct bmp_image* result = flip_horizontally(stepVertival);

    free_bmp_image(stepRotate);
    free_bmp_image(stepVertival);

    return result;
}


struct bmp_image* crop(const struct bmp_image* image, const uint32_t start_y, const uint32_t start_x, const uint32_t height, const uint32_t width) {

    if (image == NULL) return NULL;
    if (start_x+width > image->header->width || start_y+height > image->header->height ||
        start_x < 0 || start_y < 0 || width < 1 || height < 1) return NULL;

    struct bmp_image* result = bmp_copy(image, width, height);
    uint32_t newIdx = 0;

    uint32_t padding_size = ((4-(result->header->width*3)%4)%4)*result->header->height;
    result->header->image_size = padding_size+result->header->width*result->header->height*3;
    result->header->size = result->header->image_size+result->header->offset;

    for (uint32_t i = image->header->height-start_y-height; i < image->header->height-start_y; ++i) {

        uint32_t idx = i*image->header->width+start_x;

        for (uint32_t j = start_x; j < start_x+width; ++j) {

            result->data[newIdx] = image->data[idx];

            idx++;
            newIdx++;

        }

    }

    return result;
}


struct bmp_image* scale(const struct bmp_image* image, float factor) {

    if (image == NULL || factor <= 0) return 0;

    uint32_t newWidth = (uint32_t)round((float)image->header->width*factor);
    uint32_t newHeight = (uint32_t)round((float)image->header->height*factor);
    struct bmp_image* result = bmp_copy(image, newWidth, newHeight);

    uint32_t padding_size = ((4-(newWidth*3)%4)%4)*newHeight;
    result->header->image_size = padding_size+newWidth*newHeight*3;
    result->header->size = result->header->image_size+result->header->offset;
    
    for (uint32_t i = 0; i < newHeight; ++i) {
        for (uint32_t j = 0; j < newWidth; ++j) {
            
            uint32_t idx = i*newWidth+j;
            uint32_t x, y, newIdx;
            x = (uint32_t)round((j*image->header->width)/newWidth);
            y = (uint32_t)round((i*image->header->height)/newHeight);
            newIdx = y*image->header->width+x;
            
            result->data[idx] = image->data[newIdx];
        }
    }

    return result;
}


struct bmp_image* extract(const struct bmp_image* image, const char* colors_to_keep) {

    if (image == NULL || colors_to_keep == NULL) return NULL;
    if (strlen(colors_to_keep) > 3) return NULL;

    for (int i = 0; i < strlen(colors_to_keep); ++i)
        if (colors_to_keep[i] != 'r' && colors_to_keep[i] != 'g' && colors_to_keep[i] != 'b')
            return NULL;

    struct bmp_image* result = bmp_copy(image, image->header->width, image->header->height);

    for (int i = 0; i < image->header->width*image->header->height; ++i) {

        if (!strstr(colors_to_keep, "r")) result->data[i].red = 0;
        if (!strstr(colors_to_keep, "g")) result->data[i].green = 0;
        if (!strstr(colors_to_keep, "b")) result->data[i].blue = 0;

    }

    return result;
}
