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
Quadtree* create_Tree(int limit, int x, int y, int w, int h, pixel **pixels)
{
    int i, j;
    /* calcularea culorii medii a blocului */
    uint32_t Red=0, Green=0, Blue=0;   
    for(i = x; i< x + w; i++)
        for(j = y; j < y + h; j++) {
            Red   += pixels[i][j].Red;
            Green += pixels[i][j].Green;
            Blue  += pixels[i][j].Blue;
        }
    Red   /= w * h;
    Green /= w * h;
    Blue  /= w * h;
    
    /* calcularea scorului al similaritatii */
    uint32_t mean = 0;  /// scor al similaritatii
    for(i = x; i < x + w; i++)
        for(j = y; j < y + h; j++) {
            mean += (Red - pixels[i][j].Red) * (Red - pixels[i][j].Red);
            mean += (Green - pixels[i][j].Green) * (Green - pixels[i][j].Green);
            mean += (Blue - pixels[i][j].Blue) * (Blue - pixels[i][j].Blue);
        }
    mean /= 3 * w * h;

    /* crearea unui nod in arborele cuaternar */ 
    Quadtree *Tree = calloc(1, sizeof(Quadtree));
    Tree->Red = Red; Tree->Green = Green; Tree->Blue = Blue;
  
    /* verificarea necesitatii pentru o noua diviziune */
    if(mean > limit) {
        Tree->child1 = create_Tree(limit, x, y, w/2, h/2, pixels);
        Tree->child2 = create_Tree(limit, x, y+h/2, w/2, h/2, pixels);
        Tree->child3 = create_Tree(limit, x+w/2, y+h/2, w/2, h/2, pixels);
        Tree->child4 = create_Tree(limit, x+w/2, y, w/2, h/2, pixels);
    }
    else 
        Tree->child1 = Tree->child2 = Tree->child3 = Tree->child4 = NULL; 
    
    return Tree;
}

/* calculeaza nr. de blocuri cu informatie utila in cadrul procesului de 
compresie si nr. total de noduri ale arborelui creat */
void count(Quadtree *Tree, uint32_t *nr_colors, uint32_t *nr_nodes)
{
    (*nr_nodes)++;
    if(Tree->child1 == NULL)
        (*nr_colors)++;
    else {
        count(Tree->child1, nr_colors, nr_nodes);
        count(Tree->child2, nr_colors, nr_nodes);
        count(Tree->child3, nr_colors, nr_nodes);
        count(Tree->child4, nr_colors, nr_nodes);
    } 
}

/* crearea vectorului de tip structura QuaftreeNode asociat arborelui */ 
void add(QuadtreeNode *array, Quadtree *Tree, int *k, int area)
{
    /* adaugarea informatiilor utile asociat nodului */
    array[*k].red = Tree->Red;
    array[*k].green = Tree->Green;
    array[*k].blue = Tree->Blue;
    array[*k].area = area;
    int k_cur = *k;
    (*k)++;
    if(Tree->child1 != NULL){
        /* gasire nod neterminal */
        array[k_cur].top_left = *k;
        add(array, Tree->child1, k, area/4);
        array[k_cur].top_right = *k;
        add(array, Tree->child2, k, area/4);
        array[k_cur].bottom_right = *k;
        add(array, Tree->child3, k, area/4);
        array[k_cur].bottom_left = *k;
        add(array, Tree->child4, k, area/4);
    }
    else {
        /* gasire frunza */
        array[k_cur].top_left = -1;
        array[k_cur].top_right = -1;
        array[k_cur].bottom_right = -1;
        array[k_cur].bottom_left = -1;
    }
}

/* eliberarea din memorie a arborelui */
void free_Tree(Quadtree *Tree)
{
    if(Tree->child1 != NULL) {
        free_Tree(Tree->child1);
        free_Tree(Tree->child2);
        free_Tree(Tree->child3);
        free_Tree(Tree->child4);
    }
    free(Tree);
}

int main(int argc, char **argv)
{
    /* cerinta 1 : comprimarea unei imagini */
    if(strcmp("-c",argv[1]) == 0) {

        FILE *fin = fopen(argv[argc-2], "rb");
        if(fin == NULL) {printf("Error: %s\n", argv[argc-2]); return 1;}

        /* citirea antetului */
        char type[2];
        int width, height, maxx;
        fscanf(fin,"%s%d%d%d", type, &width, &height, &maxx);
        if(width * height) fseek(fin, 1, SEEK_CUR);

        /* citirea unei imagini intr-o matrice de pixeli */
        int i, j;
        pixel **pixels = calloc(width, sizeof(pixel *));
        for(i = 0; i < width; i++) {
            pixels[i] = calloc(height, sizeof(pixel));
            for(j = 0; j < height; j++)
                fread(&pixels[i][j], sizeof(pixel), 1, fin);
        }
        fclose(fin);

        /* crearea arborelui cuaternar de compresie */
        Quadtree *Tree;
        Tree = create_Tree(atoi(argv[2]), 0, 0, width, height, pixels);

        uint32_t nr_colors = 0;  // nr. de blocuri cu info. utila
        uint32_t nr_nodes = 0;   // nr. total de noduri al arborelui
        count(Tree, &nr_colors, &nr_nodes);
            
        /* crearea vectorului de tip struct Quadtree asociat arborelui*/
        int index = 0;
        QuadtreeNode *array = calloc(nr_nodes, sizeof(QuadtreeNode));
        add(array, Tree, &index, height*width);

        /* scrierea in fisierul comprimat */
        FILE *fout = fopen(argv[argc-1],"wb");
        if(fout == NULL) {printf("Error: %s\n", argv[argc-1]); return;}
        fwrite(&nr_colors, sizeof(uint32_t), 1, fout);
        fwrite(&nr_nodes, sizeof(uint32_t), 1, fout);
        fwrite(array, sizeof(QuadtreeNode), nr_nodes, fout);
        fclose(fout);

        /* eliberarea memoriei */
        free(array);
        free_Tree(Tree);
        for(i = 0; i < width; i++)
            free(pixels[i]);
        free(pixels);

    }

    if(strcmp("-m", argv[1])==0) {
        FILE *fin = fopen(argv[argc-2], "rb");
        if(fin == NULL) {printf("Error: %s\n", argv[argc-2]); return;}

        /* citirea antetului */
        char type[2];
        int width, height, maxx;
        fscanf(fin,"%s%d%d%d", type, &width, &height, &maxx);
        if(width * height) fseek(fin, 1, SEEK_CUR);

        /* citirea unei imagini intr-o matrice de pixeli */
        int i, j;
        pixel **pixels = calloc(width, sizeof(pixel *));
        for(i = 0; i < width; i++) {
            pixels[i] = calloc(height, sizeof(pixel));
            for(j = 0; j < height; j++)
                fread(&pixels[i][j], sizeof(pixel), 1, fin);
        }
        fclose(fin);

        /* crearea arborelui cuaternar de compresie */
        Quadtree *Tree;
        Tree = create_Tree(0, 0, 0, width, height, pixels);

        if(strcmp(argv[2], "h") == 0)
            h(Tree);
    }
}

