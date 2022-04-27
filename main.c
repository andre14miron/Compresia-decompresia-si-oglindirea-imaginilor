#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
    uint32_t area;
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
    Tree->area = w * h;
  
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

void h(Quadtree *Tree)
{
    if(Tree->child1 == NULL) return;
    h(Tree->child1);
    h(Tree->child2);
    h(Tree->child3);
    h(Tree->child4);
    Quadtree *temp = Tree->child1;
    Tree->child1 = Tree->child2;
    Tree->child2 = temp;

    temp = Tree->child3;
    Tree->child3 = Tree->child4;
    Tree->child4 = temp;
}

void v(Quadtree *Tree)
{
    if(Tree->child1 == NULL) return;
    v(Tree->child1);
    v(Tree->child2);
    v(Tree->child3);
    v(Tree->child4);
    Quadtree *temp = Tree->child1;
    Tree->child1 = Tree->child4;
    Tree->child4 = temp;

    temp = Tree->child2;
    Tree->child2 = Tree->child3;
    Tree->child3 = temp;
}

Quadtree *Tree_from_array(QuadtreeNode *array, int index)
{
    Quadtree *Tree = calloc(1, sizeof(Quadtree));
    Tree->Red = array[index].red;
    Tree->Green = array[index].green;
    Tree->Blue = array[index].blue;
    Tree->area = array[index].area;
    
    if(array[index].top_left == -1)
        Tree->child1 = Tree->child2 = Tree->child3 = Tree->child4 = NULL;
    else {
        Tree->child1 = Tree_from_array(array, array[index].top_left);
        Tree->child2 = Tree_from_array(array, array[index].top_right);
        Tree->child3 = Tree_from_array(array, array[index].bottom_right);
        Tree->child4 = Tree_from_array(array, array[index].bottom_left);
    }
    return Tree;
}

void image_from_tree(pixel **pixels, Quadtree *Tree, int x, int y, uint32_t size)
{
    if(Tree->child1 == NULL) {
        int i, j;
        for(i = x; i < x+size; i++)
            for(j = y; j < y+size; j++) {
                pixels[i][j].Red = Tree->Red;
                pixels[i][j].Green = Tree->Green;
                pixels[i][j].Blue = Tree->Blue;
            }
    }
    else {
        image_from_tree(pixels, Tree->child1, x, y, size/2);
        image_from_tree(pixels, Tree->child2, x, y+size/2, size/2);
        image_from_tree(pixels, Tree->child3, x + size/2, y + size/2, size/2);
        image_from_tree(pixels, Tree->child4, x+size/2, y, size/2);
    }
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
        if(fout == NULL) {printf("Error: %s\n", argv[argc-1]); return 1;}
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

    /* cerinta 2 : decompresia unei imagini */
    if(strcmp("-d", argv[1]) == 0) {
        FILE *fin = fopen(argv[argc-2], "rb");
        if(fin == NULL) {printf("Error: %s\n", argv[argc-2]); return 1;}

        uint32_t nr_colors = 0;  // nr. de blocuri cu info. utila
        uint32_t nr_nodes = 0;   // nr. total de noduri al arborelui
        fread(&nr_colors, sizeof(uint32_t), 1, fin);
        fread(&nr_nodes, sizeof(uint32_t), 1, fin);

        QuadtreeNode *array = calloc(nr_nodes, sizeof(QuadtreeNode));
        fread(array, sizeof(QuadtreeNode), nr_nodes, fin);
        fclose(fin);

        Quadtree *Tree = Tree_from_array(array, 0);

        int i, j;
        uint32_t size = sqrt(Tree->area);
        pixel **pixels = calloc(size, sizeof(pixel *));
        for(i = 0; i < size; i++) 
            pixels[i] = calloc(size, sizeof(pixel));
        image_from_tree(pixels, Tree, 0, 0, size);

        FILE *fout = fopen(argv[argc-1], "wb");
        if(fout == NULL) {printf("Error: %s\n", argv[argc-1]); return 1;}
        fprintf(fout, "P6\n");
        fprintf(fout, "%d %d\n", size, size);
        fprintf(fout, "255\n");
       
        for(i=0; i<size; i++)
            fwrite(pixels[i], sizeof(pixel), size, fout);
        
        fclose(fout);

        /* eliberarea memoriei */
        free(array);
        free_Tree(Tree);
        for(i = 0; i < size; i++)
            free(pixels[i]);
        free(pixels);
    }

    if(strcmp("-m", argv[1])==0) {
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
        Tree = create_Tree(atoi(argv[3]), 0, 0, width, height, pixels);

        if(strcmp(argv[2], "h") == 0)
            h(Tree);
        if(strcmp(argv[2], "v") == 0)
            v(Tree);

        image_from_tree(pixels, Tree, 0, 0, width);

        /* scrierea in fisierul comprimat */
        FILE *fout = fopen(argv[argc-1],"wb");
        if(fout == NULL) {printf("Error: %s\n", argv[argc-1]); return 1;}
        fprintf(fout, "%s\n", type);
        fprintf(fout, "%d %d\n", width, height);
        fprintf(fout, "%d\n", maxx);

        for(i=0; i<width; i++)
            fwrite(pixels[i], sizeof(pixel), height, fout);
        
        fclose(fout);

        /* eliberarea memoriei */
        free_Tree(Tree);
        for(i = 0; i < width; i++)
            free(pixels[i]);
        free(pixels);
    }
}

