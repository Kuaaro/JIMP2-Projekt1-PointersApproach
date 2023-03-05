#ifndef TSTRING_H
#define TSTRING_H

#include <stdio.h>

typedef struct {
    int len, alloc;
    char *text;
} String;

void allocateString(String *in, int n);

void doubleStringSize(String *in);

void pushString(String *in, char c);

void pullString(String *in, int n);

void reverseString(String *in);

void mergeString(String *in, String *out);

void writeOut(FILE *f, String *in);

void freeString(String *in);
#endif