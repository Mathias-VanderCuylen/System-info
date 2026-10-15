#include <stdio.h>
#include <stdint.h>

void init();
void my_free(void *pointer);
void *my_malloc(size_t size);

#define MY_HEAP_SIZE 64000
uint8_t MY_HEAP[MY_HEAP_SIZE];
uint8_t PADDING = 2;

int main() {
    init();
    
    uint8_t *ptr1 = (uint8_t*)my_malloc(1);
    uint8_t *ptr1_cp = ptr1;
    for (int i = 0; i < 1; i++) {
        *ptr1_cp = i;
        ptr1_cp++;
    }

    uint8_t *ptr2 = (uint8_t*)my_malloc(17);
    uint8_t *ptr2_cp = ptr2;
    for (int i = 0; i < 17; i++) {
        *ptr2_cp = i;
        ptr2_cp++;
    }

    uint8_t *ptr3 = (uint8_t*)my_malloc(13);
    uint8_t *ptr3_cp = ptr3;
    for (int i = 0; i < 13; i++) {
        *ptr3_cp = i;
        ptr3_cp++;
    }
    
    for (int i = 0; i < 40; i++) {
        printf("%d ", MY_HEAP[i]);
    }
    printf("\n");

    my_free(ptr1);
    printf("%p", ptr1);
    my_free(ptr2);

    uint8_t *ptr4 = (uint8_t*)my_malloc(35);
    uint8_t *ptr4_cp = ptr4;
    for (int i = 0; i < 14; i++) {
        *ptr4_cp = i;
        ptr4_cp++;
    }
    printf("\n");

    for (int i = 0; i < 250; i++) {
        printf("%d ", MY_HEAP[i]);
    }
    printf("\n");

    return 0;
}

// le type uint8_t n'accepte que 8 bits, il est donc nécessaire de stocker les métadonnées sur 2 cases
// la 1ere case stocke les 8 bits de poids forts et la 2e les 8 bits de poids faible
void init() {
    MY_HEAP[0] = (uint8_t)(MY_HEAP_SIZE >> 8);
    MY_HEAP[1] = (uint8_t)(MY_HEAP_SIZE & 0b11111111);
}

void my_free(void *pointer) {
    // Si le pointeur donné en argumment est NULL, aucune action n'est faite
    if (pointer == NULL)
        return;
    uint8_t *metadata = (uint8_t*)pointer - 1;
    pointer = NULL;
    // on met le bit de poids faible à 0 pour indiquer que le block est libre
    *metadata = *metadata & 0b11111110;
}

void *my_malloc(size_t size) {
    if (size == 0)
        return NULL;

    // On créer la taille du block à allouer en ajoutant la taille des métadonnées.
    size_t actual_size = size + 2;

    // Test si la taille du block est un multiple de PADDING, sinon on ajoute le padding nécessaire 
    if (actual_size % PADDING != 0)
        actual_size += PADDING - (actual_size % PADDING);

    if (actual_size > MY_HEAP_SIZE)
        return NULL;

    int i = 0;
    int free_block_size = 0;
    int i_first_free_block = -1;
    // vérifie qu'on se trouve toujours dans la mémoire disponible, que le block sur lequel on se trouve n'est pas aloué et que le block sur lequel on se trouve est assez grand pour être aloué
    while (i < MY_HEAP_SIZE && free_block_size < (int)actual_size) {
        int block_size = (MY_HEAP[i] << 8) + (MY_HEAP[i+1] & 0b11111110);
        if ((MY_HEAP[i+1] & 0b00000001) == 0) {          // bloc libre
            if (free_block_size == 0)
                i_first_free_block = i;
            free_block_size += block_size;
        } else {                               // bloc occupé : reset du run
            free_block_size = 0;
            i_first_free_block = -1;
        }
        i += block_size;
    }
    
    if (i_first_free_block == -1)
        return NULL;

    // Calcule de la place qu'il reste dans le block après celui qu'on va allouer
    size_t next_block_size = free_block_size - actual_size;

    MY_HEAP[i_first_free_block]     = (uint8_t)(actual_size >> 8);
    MY_HEAP[i_first_free_block + 1] = (uint8_t)(actual_size & 0xFF) | 1;  // bit 0 = 1 → occupied
    // Si on n'alloue pas l'entiereté de la heap on va ajouter une nouvelle métadonnée et marquer le block comme utilisé.
    if (next_block_size > 0) {
        // crée la prochaine métadonnée
        MY_HEAP[i_first_free_block + actual_size] = (uint8_t)(next_block_size >> 8);
        MY_HEAP[i_first_free_block + actual_size + 1] = (uint8_t)(next_block_size & 0b11111111);
    }
    

    // retourne le pointeur de la première donnée (après la métadonnée)
    return &MY_HEAP[i_first_free_block + 2];
}