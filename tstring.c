#include "tstring.h"

#include <stdio.h>
#include <stdlib.h>

/*allocate String with n spaces for chars*/
void allocateString(String *in, int n) {
    in->text = malloc(sizeof(char) * n);
    in->alloc = n;
    in->len = 0;
}

/*double allocation of String*/
void doubleStringSize(String *in) {
    in->alloc *= 2;
    in->text = realloc(in->text, sizeof(char) * in->alloc);
}

/*pushes char at the end of String*/
void pushString(String *in, char c) {
    if(in->len==in->alloc)
        doubleStringSize(in);
    in->text[in->len++] = c;
}

/*removes n first characters of String*/
void pullString(String *in, int n) {
    int i;
    for(i=n; i<in->len; i++)
        in->text[i-n] = in->text[i];
    in->len -= n;
}

/*reverses String*/
void reverseString(String *in) {
    int i;
    char c;
    for(i=0; i<in->len/2; i++) {
        c = in->text[i];
        in->text[i] = in->text[in->len - i - 1];
        in->text[in->len - i - 1] = c;
    }
}

/*merges String in to String out*/
void mergeString(String *in, String *out) {
    int i;
    for(i=0; i<in->len; i++)
        pushString(out, in->text[i]);
}

/*writes String to file in binary*/
void writeOut(FILE *f, String *in) {
    int i, j, k;
    #ifdef DEBUG
    char c[8];
    #endif
    while(in->len>=8) {
        k = 1;
        j = 0;
        for(i=7; i>=0; i--) {
            j += (in->text[i] - 48) * k;
            k *= 2;
            #ifdef DEBUG
            c[i] = in->text[i];
            #endif
        }
        fwrite(&j, sizeof(char), 1, f);
        #ifdef DEBUG
        printf("%s %d\n", c, j);
        #endif
        pullString(in, 8);
    }
}

/*frees String*/
void freeString(String *in) {
    free(in->text);
}