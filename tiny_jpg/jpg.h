#pragma once

#include "../core/core.h"

//PROGRESSIVE JPG IS NOT SUPPORTED!

typedef struct 
{
    uint8_t bits[ 16 ];
    uint8_t hval[ 256 ];
    uint8_t size[ 256 ];
    uint16_t code[ 256 ];
} jpeg_huffman_table_t;

typedef struct 
{
    uint8_t* ptr;
    FILE* file;
    int width;
    int height;
    uint8_t* data; //ycbcr
    int data_precision;
    int num_components;
    int restart_interval;
    struct 
    {
        int id;
        int h;
        int v;
        int t;
        int td;
        int ta;
    } component_info[ 3 ];
    jpeg_huffman_table_t hac[ 4 ];
    jpeg_huffman_table_t hdc[ 4 ];
    int qtable[ 4 ][ 64 ];
    struct 
    {
        int ss, se;
        int ah, al;
    } scan;
    int dc[ 3 ];
    int curbit;
    uint8_t curbyte;
} jpeg_file_desc;

//convert_to: 1 - grayscale; 2 - rgb;
jpeg_file_desc* jpeg_load( const char* filename, FILE* f, int convert_to );
