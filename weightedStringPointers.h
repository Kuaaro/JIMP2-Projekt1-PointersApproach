#ifndef WEIGHTED_STRING_POINTERS_H
#define WEIGHTED_STRING_POINTERS_H

#include "tstring.h"

#include <stdlib.h>

typedef struct {
    int len, weight;
    String **pointers;
} WeightedStringPointers;

void allocateWeightedStringPointers(WeightedStringPointers *in, int len, int weight);

int cmpfun(void const *a, void const *b);

void mergeAt(WeightedStringPointers *out, WeightedStringPointers *in1, WeightedStringPointers *in2);

void pushPointer(WeightedStringPointers *in, String *pointer);

void freePointer(WeightedStringPointers *in);
#endif