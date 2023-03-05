#include <stdlib.h>

#include "weightedStringPointers.h"
#include "tstring.h"

/*allocate wsp*/
void allocateWeightedStringPointers(WeightedStringPointers *in, int len, int weight) {
    in->pointers = malloc(sizeof(String*) * len + 1);
    in->weight = weight;
    in->len = len;
}

/*comparing function*/
int cmpfun(void const *a, void const *b) {
    const WeightedStringPointers *oa = a;
    const WeightedStringPointers *ob = b;
    return(oa->weight - ob->weight);
}

/*merge in1 and in2 into out, free in1, in2*/
void mergeAt(WeightedStringPointers *out, WeightedStringPointers *in1, WeightedStringPointers *in2) {
    int i;
    allocateWeightedStringPointers(out, in1->len + in2->len, in1->weight + in2->weight);
    out->len = 0;

    for(i=0; i<in1->len; i++) {
        pushString(in1->pointers[i], '0');
        pushPointer(out, in1->pointers[i]);
    }
    freePointer(in1);

    for(i=0; i<in2->len; i++) {
        pushString(in2->pointers[i], '1');
        pushPointer(out, in2->pointers[i]);
    }
    freePointer(in2);
}

/*add pointer*/
void pushPointer(WeightedStringPointers *in, String *pointer) {
    in->pointers[in->len] = pointer;
    in->len++;
}

/*free pointer*/
void freePointer(WeightedStringPointers *in) {
    free(in->pointers);
}