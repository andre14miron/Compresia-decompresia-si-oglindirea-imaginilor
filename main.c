#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct QuadtreeNode {
    unsigned char blue, green, red;
    uint32_t area;
    int32_t top_left, top_right;
    int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode ;

int main()
{
    uint32_t nr_colors; // nr. de blocuri cu informatie utila
    uint32_t nr_nodes;  // nr. total de noduri ale arborelui cuaternar
}