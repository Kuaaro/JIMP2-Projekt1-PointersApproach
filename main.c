#include "tstring.h"
#include "weightedStringPointers.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef TIME
#include <time.h>
#endif

#ifdef SIZE
    int in = 0, out = 1;
#endif

#ifdef TREE
int cmpfun2(void const * a, void const * b) {
    const String *ao = a;
    const String *bo = b;
    int i, j = 1, ac = 0, bc = 0;

    for(i=ao->len<bo->len ? bo->len-1: ao->len-1; i>=0; i--) {
        if(ao->len > i)
            ac += (ao->text[i]-48)*j;
        if(bo->len > i)
            bc += (bo->text[i]-48)*j;
        j *= 2;
    }
    return bc-ac;
}
#endif



int main(int argc, char **argv) {
    /*allocate*/
    WeightedStringPointers wsp[256], temp_wsp;
    String mainStrings[256], temp_string, small_temp_string;
    int i, j, k;
    FILE *f_in, *f_out;

    #ifdef TREE
    String *temp_string_arr;
    int *dep, str_len, dep_len = 0, forward, ct;
    #endif

    #ifdef TIME
        clock_t t0, t1;
        t0 = clock();
    #endif

    /*idiotproofing*/
    if(argc==1) {
        printf("%s: Nie podano nazwy pliku\n", argv[0]);
        return 1;
    }

    if((f_in = fopen(argv[1], "rb")) == NULL) {
        printf("%s: Nie mozna otworzyc pliku %s\n", argv[0], argv[1]);
        return 1;
    }

    /*2nd allocations*/
    for(i=0; i<256; i++) {
        allocateString(&mainStrings[i], 1);
        allocateWeightedStringPointers(&wsp[i], 1, 0);
        wsp[i].len = 0;
        pushPointer(&wsp[i], &mainStrings[i]);
    }

    /*go through file*/
    while((i = getc(f_in)) != EOF) {
        wsp[i].weight++;
        #ifdef SIZE
        in++;
        #endif

    }

    /*go through file name*/
    for(i=0; i<sizeof(argv[1])/sizeof(char); i++)
        wsp[(int)(argv[1][i])].weight++;
    wsp[47].weight++;

    /*sort wsp*/
    qsort(wsp, 256, sizeof(WeightedStringPointers), cmpfun);

    /*check where wsp weights are larger than 0, frees everything else*/
    for(j=0; j<256; j++) {
        if(wsp[j].weight)
            break;
        freePointer(&wsp[j]);
    }

    /*2nd idioproofing, first if for empty files, 2nd for file containing only \n characters*/
    if(j==256) {
        printf("%s: Plik %s jest pusty\n", argv[0], argv[1]);
        fclose(f_in);
        for(i=0; i<256; i++)
            freeString(&mainStrings[i]);
        return 1;
    } else if(j==255) {
        pushString(wsp[255].pointers[0], '0');
        freePointer(&wsp[255]);
    }

    /*huffman*/
    for(i=j; i<255; i++) {
        mergeAt(&temp_wsp, &wsp[i], &wsp[i+1]);
        for(j=i+1; j<255; j++)
            if(wsp[j+1].weight < temp_wsp.weight) {
                wsp[j] = wsp[j+1];
            } else {
                break;
            }
        wsp[j] = temp_wsp;
    }

    freePointer(&temp_wsp);
    j=0;

    /*goes through string, gets max len and reverses all Strings*/
    for(i=0; i<256; i++) {
        j = j<mainStrings[i].len ? mainStrings[i].len : j;
        reverseString(&mainStrings[i]);
    }

    allocateString(&temp_string, (j>8 ? j : 8)+7);
    allocateString(&small_temp_string, 8);
    f_out = fopen("out.huff", "wb");

    /*writes out char codes*/
    for(i=0; i<256; i++) {
        k = mainStrings[i].len;
        for(j=0; j<8; j++) {
            pushString(&small_temp_string, k%2 + '0');
            k /= 2;
        }

        reverseString(&small_temp_string);
        mergeString(&small_temp_string, &temp_string);
        pullString(&small_temp_string, 8);
        if(temp_string.len >= 8)
            writeOut(f_out, &temp_string);
        mergeString(&mainStrings[i], &temp_string);
        if(temp_string.len >= 8)
            writeOut(f_out, &temp_string);
    }

    freeString(&small_temp_string);

    /*writes out file name*/
    for(i=0; i<sizeof(argv[1])/sizeof(char); i++) {
        mergeString(&mainStrings[(int)(argv[1][i])], &temp_string);
        if(temp_string.len >= 8)
            writeOut(f_out, &temp_string);
    }

    /*adds '\' char, since '\' can't be used in file names, both on windows and linux, can be recognized be decompressor as end of file name*/
    mergeString(&mainStrings[47], &temp_string);
    if(temp_string.len >= 8)
            writeOut(f_out, &temp_string);

    /*compressing file*/
    rewind(f_in);
    while((i = getc(f_in)) != EOF) {
        mergeString(&mainStrings[i], &temp_string);
        #ifdef SIZE
        out += mainStrings[i].len;
        #endif
        if(temp_string.len >= 8)
            writeOut(f_out, &temp_string);
    }

    /*fulfillment*/
    pushString(&temp_string, '1');
    if(temp_string.len >= 8)
        writeOut(f_out, &temp_string);
    while(temp_string.len%8!=0) {
        pushString(&temp_string, '0');
        #ifdef SIZE
            out++;
        #endif
    }
    writeOut(f_out, &temp_string);

    /*frees*/
    freeString(&temp_string);

    fclose(f_in);
    fclose(f_out);

    #ifdef DEBUG
    printf("\n\n");
    #endif

    #ifdef DEBUG
    for(i=0; i<256; i++) {
        if(mainStrings[i].len)
            printf("%d: %c | %s\n", i, i > 31 ? i : 0, mainStrings[i].text);
    }
    #endif

    printf("%s: Plik %s zostal pomslnie skompresowany do pliku out.huff\n", argv[0], argv[1]);

    #ifdef TIME
    t1 = clock();
    printf("Czas: %f\n", (double)(t1-t0) / CLOCKS_PER_SEC);
    #endif

    #ifdef SIZE
    in *= 8;
    printf("Rozmiar danych przed kompresja: %d bitow\nRozmiar danych po kompresji: %d bitow\nStopien kompresji: %f%%\n", in, out, ((double)((in - out) * 100))/in);
    #endif

    #ifdef TREE
    /*whole ifdef creates visualization of binary tree, chars from 32 to 126 are printed as chars with '', otherwise they are numbers without '', not needed for decompression*/
    str_len = 0;
    for(i=0; i<256; i++)
        if(mainStrings[i].len)
            str_len++;
    temp_string_arr = malloc(sizeof(String) * str_len);
    dep = calloc(str_len*2-1, sizeof(int));

    j = 0;
    for(i=0; i<256; i++)
        if(mainStrings[i].len)
            temp_string_arr[j++] = mainStrings[i];

    qsort(temp_string_arr, str_len, sizeof(String), cmpfun2);

    for(i=0; i<2*str_len-1; i++) {
        if(i%2==0) {
            for(j=0; j<(temp_string_arr[i/2].len+1)*2; j++)
                if(j<temp_string_arr[i/2].len*2) {
                    ct=0;
                    for(k=0; k<dep_len; k++)
                        if(dep[k]==j+1)
                            ct++;
                    if(ct%2)
                        printf("|");
                    else
                        printf(" ");
                } else if(j == temp_string_arr[i/2].len*2){
                    for(k=0; k<256; k++)
                        if(mainStrings[k].text==temp_string_arr[i/2].text)
                            break;
                    if(k>31 && k<127)
                        printf("_ '%c' - %s\n", k, temp_string_arr[i/2].text);
                    else
                        printf("_ %d - %s\n", k, temp_string_arr[i/2].text);
                }
            dep[dep_len++] = j-2;
            forward = (temp_string_arr[i/2].len-1)*2;
        } else {
            for(j=dep_len-2; j>=0; j-=2)
                if(dep[j]==forward)
                    forward-=2;
                else if(dep[j]<forward)
                    break;
            dep[dep_len++] = forward;
            for(j=0; j<(temp_string_arr[i/2].len)*2+1; j++)
                if(j!=forward) {
                    ct=0;
                    for(k=0; k<dep_len-1; k++)
                        if(dep[k]==j+1)
                            ct++;
                    if(ct%2)
                        printf("|");
                    else
                        printf(" ");
                }
                else {
                    printf("_|\n");
                    break;
                }
        }
    }

    free(dep);
    free(temp_string_arr);
    #endif


    for(i=0; i<256; i++)
        freeString(&mainStrings[i]);
    return 0;
}