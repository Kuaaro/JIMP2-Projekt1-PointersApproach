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

int main(int argc, char **argv) {
    /*allocate*/
    WeightedStringPointers wsp[256], temp_wsp;
    String mainStrings[256], temp_string, small_temp_string;
    int i, j, k;
    FILE *f_in, *f_out;

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

    /*writing out char codes*/
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

    for(i=0; i<sizeof(argv[1])/sizeof(char); i++) {
        mergeString(&mainStrings[(int)(argv[1][i])], &temp_string);
        if(temp_string.len >= 8)
            writeOut(f_out, &temp_string);
    }

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

    for(i=0; i<256; i++) {
        #ifdef DEBUG
        if(mainStrings[i].len)
            printf("%d: %c | %s\n", i, i > 31 ? i : 0, mainStrings[i].text);
        #endif
        freeString(&mainStrings[i]);
    }

    printf("%s: Plik %s zostal pomslnie skompresowany do pliku out.huff\n", argv[0], argv[1]);

    #ifdef TIME
    t1 = clock();
    printf("Czas: %f\n", (double)(t1-t0) / CLOCKS_PER_SEC);
    #endif

    #ifdef SIZE
    in *= 8;
    printf("Rozmiar danych przed kompresja: %d bitow\nRozmiar danych po kompresji: %d bitow\nStopien kompresji: %f%%\n", in, out, ((double)((in - out) * 100))/in);
    #endif

    return 0;
}