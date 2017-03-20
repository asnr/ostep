#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mythreads.h"

#include "main-header.h"
#include "vector-header.h"

int retry = 0;

void vector_add(vector_t *v_dst, vector_t *v_src) {
  top:
    if (pthread_mutex_trylock(&v_dst->lock) != 0) {
	goto top;
    }
    if (pthread_mutex_trylock(&v_src->lock) != 0) {
	retry++;
	Pthread_mutex_unlock(&v_dst->lock);
	goto top;
    }
    int i;
    for (i = 0; i < VECTOR_SIZE; i++) {
	v_dst->values[i] = v_dst->values[i] + v_src->values[i];
    }
    Pthread_mutex_unlock(&v_dst->lock);
    Pthread_mutex_unlock(&v_src->lock);
}

void pretty_print_positive(int num) {
    if (num == 0) {
        printf("0");
        return;
    }
    
    int thousands = num / 1000;
    int remainder = num % 1000;
    if (thousands) {
        pretty_print_positive(thousands);
        printf(",%03d", remainder);
    } else {
        printf("%d", remainder);
    }
    
}

void fini() {
    printf("Retries: ");
    pretty_print_positive(retry);
    printf("\n");
}

#include "main-common.c"

