#include <inttypes.h>

/* structura pentru vectorul asociat unui arbore */
typedef struct QuadtreeNode {
    unsigned char blue, green, red;
    uint32_t area;
    int32_t top_left, top_right;
    int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode ;

/* structura pentru un pixel dintr-o imagine sub forma RGB */
typedef struct pixel {
    unsigned char Red, Green, Blue;
} pixel; 

/* structura asociata arborelui de compresie */
typedef struct Quadtree {
    unsigned char Red, Green, Blue;
    uint32_t area;
    struct Quadtree *child1, *child2, *child3, *child4;
} Quadtree;