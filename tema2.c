#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
FILE *out_b;
//structura pentru pixel:

typedef struct pixel {
    unsigned char red, green, blue;
} Pixel;

//structura pentru arbore:

typedef struct quadtree_node {
    int x;
    int y;
    int width;
    int height;
    struct pixel color; // culorile rosu, verde, albastru;
    //pointeri pentru nodurile copil:
    struct quadtree_node *ss; // copil stanga-sus;
    struct quadtree_node *ds; // copil dreapta-sus;
    struct quadtree_node *dj; // copil dreapta jos;
    struct quadtree_node *sj; // copil stanga-jos;
} QuadTreeNode;

//structura pentru nodul din coada:
typedef struct queue_node {
    QuadTreeNode *elem;
    struct queue_node *next;
} QueueNode;

//structura pentru coada:
typedef struct queue {
    long size;
    QueueNode *front;
    QueueNode *rear;
} Queue;

Queue* initQueue() {
    Queue *q = calloc(1, sizeof(struct queue));
    return q;
}

int isEmptyQueue(Queue *q) {
    return q == NULL || q->front == NULL || q->size == 0;
}
//  QueueNode *initNod( )
void enqueue(Queue *q, QuadTreeNode *nod) {
    QueueNode *newNode;
    newNode = (QueueNode*)calloc(1, sizeof(QueueNode));
    newNode->elem = nod;
    newNode->next = NULL;
    if (isEmptyQueue(q)) {
        q->front = newNode;
        q->rear = newNode;
        q->size++;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
        q->size++;
    }
}

void dequeue(Queue *q) {
    QueueNode *tmp;
    if (!isEmptyQueue(q)) {
        tmp = q->front;
        q->front = q->front->next;
        free(tmp);
        q->size--;
    }
}
//functie de parcurgere pe nivel folosindu-ma de coada pentru a retine copii:
void print_nivel_2(QuadTreeNode *root) {
    if (root == NULL)
        return;
    Queue *q = initQueue();
    enqueue(q, root);
    QuadTreeNode* node;
    while (!isEmptyQueue(q)) {
    if(q->front != NULL) {
       node = q->front->elem;
    if (node->dj == NULL && node->ds == NULL && node->ss == NULL && node->sj == NULL) {
        unsigned char c = 1;
        fwrite(&c, sizeof(unsigned char), 1, out_b);
        fwrite(&node->color.red, sizeof(unsigned char), 1, out_b);
        fwrite(&node->color.green, sizeof(unsigned char), 1, out_b);
        fwrite(&node->color.blue, sizeof(unsigned char), 1, out_b);
    }
    else {
        unsigned char d = 0;
        fwrite(&d, sizeof(unsigned char), 1, out_b);
        if (node->ss != NULL) {
            enqueue(q, node->ss);
        }
        if (node->ds != NULL) {
            enqueue(q, node->ds);
        }
        if (node->dj != NULL) {
            enqueue(q, node->dj);
        }
        if (node->sj != NULL) {
            enqueue(q, node->sj);
        }
    }
    }
    dequeue(q);
    }
}
//functie de alocare a memoriei pentru un nod;

QuadTreeNode* create_quadtree_node(int x, int y, int width, int height, Pixel pixel) {
    QuadTreeNode* node = calloc(1, sizeof(QuadTreeNode));
    node->x = x;
    node->y = y;
    node->width = width;
    node->height = height;
    node->color.red = pixel.red;
    node->color.green = pixel.green;
    node->color.blue = pixel.blue;
    node->ss = NULL;
    node->ds = NULL;
    node->sj = NULL;
    node->dj = NULL;
    return node;
}

//functie pentru calcularea pragului ce determina daca se divizeaza sau nu matricea

int prag_divizare(Pixel **img, int size, int x, int y, int prag) {
    int i, j;
    unsigned long long red1 = 0, green1 = 0, blue1 = 0, red, green, blue;
    unsigned long long  mean, mean1 = 0, a, b, c;
    for (i = x; i < size + x; i++) {
        for (j = y; j < size + y; j++) {
            red1 = red1 + img[i][j].red;
            green1 = green1 + img[i][j].green;
            blue1 = blue1 + img[i][j].blue;
        }
    }
    red = red1 / (size * size);
    green = green1 / (size * size);
    blue = blue1 / (size * size);
    for (i = x; i < size + x; i++) {
        for (j = y; j < size +y; j++) {
            a = red - img[i][j].red;
            b = green - img[i][j].green;
            c = blue - img[i][j].blue;
            mean1 = mean1 + ((a * a) + (b * b) + (c * c));
        }
    }
    mean = mean1 / (3 * size * size);
    // printf("%lld ", mean);
    if (mean <= prag) {
        //nu o sa mai fie nevoie de divizare
        return 0;
    } else {
        //este nevoie de divizare
        return 1;
    }
}

//functie care imi construieste arborele
QuadTreeNode* arbore_compresie(Pixel **img, int x, int y, int size, int prag) {
    Pixel pixel;
    QuadTreeNode* root = create_quadtree_node(x, y, size, size, pixel);
    if (prag_divizare(img, size, x, y, prag) == 1) {
        //apelez recursiv functia pentru fiecare submatrice nou formata;
        root->ss = arbore_compresie(img, x , y, size / 2, prag);
        root->ds = arbore_compresie(img, x, y + (size/ 2), size / 2, prag);
        root->dj = arbore_compresie(img, x + (size / 2), y + (size / 2), size / 2, prag);
        root->sj = arbore_compresie(img, x + (size / 2), y, size / 2, prag);
    } else {
        //recalculez red, green, blue;
        unsigned long long red1 = 0, green1 = 0, blue1 = 0, red, green, blue;
        int i, j;
        for (i = x; i < size + x; i++) {
        for (j = y; j < size + y; j++) {
            red1 = red1 + img[i][j].red;
            green1 = green1 + img[i][j].green;
            blue1 = blue1 + img[i][j].blue;
        }
    }
    red = red1 / (size * size);
    green = green1 / (size * size);
    blue = blue1 / (size * size);
    root->color.red = red;
    root->color.green = green;
    root->color.blue = blue;
    }
    return root;
}

//functie care imi calculeaza numarul de niveluri din arbore:

int nr_niveluri(QuadTreeNode *root) {
    if (root == NULL) {
        return 0;
    }
    
    int ss_nivele = nr_niveluri(root->ss);
    int ds_nivele = nr_niveluri(root->ds);
    int dj_nivele = nr_niveluri(root->dj);
    int sj_nivele = nr_niveluri(root->sj);
    
    int max_child_nivele = ss_nivele;
    if (ds_nivele > max_child_nivele) {
        max_child_nivele = ds_nivele;
    }
    if (dj_nivele > max_child_nivele) {
        max_child_nivele = dj_nivele;
    }
    if (sj_nivele > max_child_nivele) {
        max_child_nivele = sj_nivele;
    }
    
    return 1 + max_child_nivele;
}

//functie care calculeaza numarul de frunze dintr-un arbore:

int nr_frunze(QuadTreeNode *root) {
    if (root == NULL) {
        return 0;
    }
    if (root->ss == NULL ) {
        // este un nod frunza
        return 1;
    }
    //calculez recursiv numarul de noduri frunza ale copiilor;
    int count = 0;
    count = count + nr_frunze(root->ss);
    count = count + nr_frunze(root->ds);
    count = count + nr_frunze(root->dj);
    count = count + nr_frunze(root->sj);
    return count;
}

//functie care realizeaza parcurgerea pe nivel
//primul nod pe care il gasesc care nu are copii
//este nodul corespunzator zonei din imaginea nedivizata
//cu dimensiunea laturii patratului cea mai mare;

int parcurgere_nivel(QuadTreeNode *root, int nr_nivel) {
    int ss, ds, dj, sj;
    if (root->ss == NULL) {
        return nr_nivel;
    } else {
        ss = parcurgere_nivel(root->ss, nr_nivel - 1);
        ds = parcurgere_nivel(root->ds, nr_nivel - 1);
        dj = parcurgere_nivel(root->dj, nr_nivel - 1);
        sj = parcurgere_nivel(root->sj, nr_nivel - 1);
        int max_niv = ss;
        if (ds > max_niv)
            max_niv = ds;
        if (dj > max_niv)
            max_niv = dj;
        if (sj > max_niv)
            max_niv = sj;
        return max_niv;
    }
}

int main(int argc, char const *argv[]) {
    if (strcmp(argv[1], "-c1") == 0 || strcmp(argv[1], "-c2") == 0) {
        FILE *fp = fopen(argv[3], "rb");
        char tip[5], u;
        int width, height, valmaxcolor, i = 0, j = 0;
        //citire antet:
        fscanf(fp, "%s", tip);
        fscanf(fp, "%d" "%d", &width, &height);
        fscanf(fp, "%d" "%c", &valmaxcolor, &u);
        //citire imagine:
        //aloc memorie pentru imagine;
        Pixel **img = malloc(sizeof(Pixel *) * height);
        //citirea datelor pentru pixeli;
        unsigned char red, green, blue;
        for (i = 0; i < height; i++) {
            img[i] = malloc(sizeof(Pixel) * width);
            for (j = 0; j < width; j++) {
                fread(&red, sizeof(unsigned char), 1, fp);
                fread(&green, sizeof(unsigned char), 1, fp);
                fread(&blue, sizeof(unsigned char), 1, fp);
                img[i][j].red = red;
                img[i][j].green = green;
                img[i][j].blue = blue;
            }
        }
        //construirea arborelui de compresie:
        int prag = atoi(argv[2]);
        int size = height;
        QuadTreeNode *root = arbore_compresie(img, 0, 0, size, prag);
        if (strcmp(argv[1], "-c1") == 0) {
        int nr_nivele = nr_niveluri(root);
        int nr_frz = nr_frunze(root);
        //nivelul pe care se gaseste nodul meu:
        int parc_niv = parcurgere_nivel(root, nr_nivele);
        int nr_nivele_parcurse = nr_nivele - parc_niv;
        // acum determinam dimensiunea laturii patratului:
        int dim = size / (pow(2, nr_nivele_parcurse));
            FILE *out;
            out = fopen(argv[4], "wt");
            fprintf(out, "%d\n", nr_nivele);
            fprintf(out, "%d\n", nr_frz);
            fprintf(out, "%d\n", dim);
            fclose(out);
        }
        if (strcmp(argv[1], "-c2") == 0) {
            out_b = fopen(argv[4], "wb");
            fwrite(&root->width, sizeof(unsigned int), 1, out_b);
            print_nivel_2(root);
            fclose(out_b);
        }
        fclose(fp);
        }
    return 0;
}