#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef struct QuadtreeNode {
    unsigned char blue, green, red;
    uint32_t area;
    int32_t top_left, top_right;
    int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode ;

typedef struct pixel {
    unsigned char Red, Green, Blue;
} pixel; 

typedef struct Quadtree {
    unsigned char Red, Green, Blue;
    struct Quadtree *child1, *child2, *child3, *child4;
} Quadtree;

/* crearea arborelui cuaternar de compresie */
Quadtree* create_Tree(Quadtree* Tree, int limit, int x, int y, int width, int height, pixel **pixels)
{
    int i, j;

    /* calcularea culorii medii a blocului */
    int Red=0, Green=0, Blue=0;   
    for(i=x; i<x+width; i++)
        for(j=y; j<y+height; j++) {
            Red   += pixels[i][j].Red;
            Green += pixels[i][j].Green;
            Blue  += pixels[i][j].Blue;
        }
    Red   /= width*height;
    Green /= width*height;
    Blue  /= width*height;
    
    /* calcularea scorului al similaritatii */
    int mean = 0;  /// scor al similaritatii
    for(i = x; i < x + width; i++)
        for(j = y; j < y + height; j++) {
            mean += (Red - pixels[i][j].Red) * (Red - pixels[i][j].Red);
            mean += (Green - pixels[i][j].Green) * (Green - pixels[i][j].Green);
            mean += (Blue - pixels[i][j].Blue) * (Blue - pixels[i][j].Blue);
        }
    mean /= 3*width*height;

    /* crearea unui nod in arborele cuaternar */ 
    Tree = calloc(1, sizeof(Quadtree));
    Tree->Red = Red; Tree->Green = Green; Tree->Blue = Blue;
  
    /* verificarea necesitatii pentru o noua diviziune */
    if(mean > limit) {
        Tree->child1 = create_Tree(Tree, limit, 0, 0, width/2, height/2, pixels);
        Tree->child2 = create_Tree(Tree, limit, 0, height/2, width/2, height/2, pixels);
        Tree->child3 = create_Tree(Tree, limit, width/2, height/2, width/2, height/2, pixels);
        Tree->child4 = create_Tree(Tree, limit, width/2, 0, width/2, height/2, pixels);
    }
    else 
        Tree->child1 = Tree->child2 = Tree->child3 = Tree->child4 = NULL; 
    
    return Tree;
}

int main(int argc, char **argv)
{
    int k;
    for(k=1; k<argc-2; k++) {
        /* cerinta 1 : comprimarea unei imagini */
        if(strcmp("-c",argv[k]) == 0) {

            FILE *fin = fopen(argv[argc-2], "rb");
            if(fin == NULL) {printf("Error: %s\n", argv[argc-2]); break;}

            /* citirea antetului */
            char type[2];
            int width, height, maxx;
            fscanf(fin,"%s%d%d%d", type, &width, &height, &maxx);
            if(width*height) fseek(fin, 1, SEEK_CUR);

            /* citirea unei imagini intr-o matrice de pixeli */
            int i, j;
            pixel **pixels = calloc(width, sizeof(pixel *));
            for(i=0; i<width; i++) {
                pixels[i] = calloc(height, sizeof(pixel));
                for(j=0; j<height; j++) 
                    fread(&pixels[i][j], sizeof(pixel), 1, fin);
            }

            /* crearea arborelui cuaternar de compresie */
            Quadtree *Tree;
            Tree = create_Tree(Tree, argv[k+1], 0, 0, width, height, pixels);

            k++;
        }
    }
}

