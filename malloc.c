#include <stdio.h>
#include <stdint.h>

void init();
void my_free(void *pointer);
void *my_malloc(size_t size);

#define MY_HEAP_SIZE 64000
#define MAX_BLOCK_SIZE 250
uint8_t MY_HEAP[MY_HEAP_SIZE];
uint8_t PADDING = 2;

int main() {
    init();
    
    uint8_t *ptr1 = (uint8_t*)my_malloc(10);
    uint8_t *ptr2 = (uint8_t*)my_malloc(20);
    uint8_t *ptr3 = (uint8_t*)my_malloc(30);
    

    return 0;
}

//le type uint8_t n'accepte que les nombre jusqu'à 255, on fait alors 256 blocks de 250 bytes maximum
void init() {
    for (int i = 0; i < MY_HEAP_SIZE; i += MAX_BLOCK_SIZE)
        MY_HEAP[i] = (uint8_t)MAX_BLOCK_SIZE;
}

void my_free(void *pointer) {
    uint8_t *metadata = (uint8_t*)pointer--;
    *metadata--;
    pointer = nullptr;
}

void *my_malloc(size_t size) {
    // à modif
    size_t actual_size = size % PADDING != 0 ? size : size + (size % PADDING); // ne fonctionne que pour PADDING = 2, à changer
    // on ajoute la taille des métadonnées à la taille nécessaire
    actual_size += 1;
    if (actual_size > MAX_BLOCK_SIZE)
        return NULL;

    int i = 0;
    // vérifie qu'on se trouve toujours dans la mémoire disponible, que le block sur lequel on se trouve n'est pas aloué et que le block sur lequel on se trouve est assez grand pour être aloué
    while (i < MY_HEAP_SIZE && (MY_HEAP[i] % 2 == 1 || MY_HEAP[i] < actual_size)) {
        // passe d'une métadonnée à la suivante
        i += (MY_HEAP[i] ^ 1);
    }
    
    if (i == MY_HEAP_SIZE)
        return NULL;

    // crée la prochaine métadonnée 
    if ((i + actual_size) % MAX_BLOCK_SIZE != 0)
        MY_HEAP[i+actual_size] = MY_HEAP[i] - actual_size;
    
    // marque le block comme utilisé
    MY_HEAP[i] ^ 1;

    // retourne le pointeur de la première donnée (après la métadonnée)
    return &MY_HEAP[i+1];
}