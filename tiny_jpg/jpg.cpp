#include "jpg.h"
#include "../memory/memory.h"
#include "../log/log.h"

//PROGRESSIVE JPG IS NOT SUPPORTED!

int jpeg_read_byte( jpeg_file_desc* jp )
{
    fread( &jp->curbyte, 1, 1, jp->file );
    jp->curbit = 0;
    return jp->curbyte;
}

int jpeg_read_word( jpeg_file_desc* jp ) 
{
    uint16_t i;
    fread( &i, 2, 1, jp->file );
    i = ( ( i << 8 ) & 0xFF00 ) + ( ( i >> 8 ) & 0x00FF );
    return i;
}

int jpeg_read_bit( jpeg_file_desc* jp ) 
{
    int i;
    if( jp->curbit == 0 ) 
    {
        jpeg_read_byte( jp );
        if( jp->curbyte == 0xFF ) 
	{
            while( jp->curbyte == 0xFF ) jpeg_read_byte( jp );
            if( jp->curbyte >= 0xD0 && jp->curbyte <= 0xD7 )
                mem_set( jp->dc, sizeof( int ) * 3, 0 );
            if( jp->curbyte == 0 ) 
		jp->curbyte = 0xFF;
            else 
		jpeg_read_byte( jp );
        }
    }
    i = ( jp->curbyte >> ( 7 - jp->curbit++ ) ) & 0x01;
    if( jp->curbit == 8 ) jp->curbit = 0;
    return i;
}

int jpeg_read_bits( int num, jpeg_file_desc* jp ) 
{
    int i, j;
    for( i = 0, j = 0; i < num; i++ ) 
    {
        j <<= 1;
        j |= jpeg_read_bit( jp );
    }
    return j;
}

int jpeg_bit2int( int bit, int i )
{
    if( ( i & ( 1 << ( bit - 1 ) ) ) > 0 ) return i;
    return -( i ^ ( ( 1 << bit ) - 1 ) );
}

int jpeg_huffmancode( jpeg_huffman_table_t* table, jpeg_file_desc* jp ) 
{
    int i, size, code;
    for( size = 1, code = 0, i = 0; size < 17; size++ ) 
    {
        code <<= 1;
        code |= jpeg_read_bit( jp );
        while( table->size[ i ] <= size ) 
	{
            if( table->code[ i ] == code ) return table->hval[ i ];
            i++;
        }
    }
    return code;
}

#define PRECISION 9
#define CONV( num ) (int)( (float)num * (float)( 1 << PRECISION ) )
#define AMUL( num ) ( num >> PRECISION )

void jpeg_idct( int* data )
{
    int t0, t1, t2, t3, t4, t5, t6, t7;
    int t10, t11, t12, t13;
    int z5, z10, z11, z12, z13;
    int* dataptr;
    int i;
    dataptr = data;
    for( i = 0; i < 8; i++ ) 
    {
        t0 = dataptr[ 8 * 0 ];
        t1 = dataptr[ 8 * 2 ];
        t2 = dataptr[ 8 * 4 ];
        t3 = dataptr[ 8 * 6 ];
        t10 = t0 + t2;
        t11 = t0 - t2;
        t13 = t1 + t3;
        t12 = - t13 + AMUL( (t1 - t3) * CONV( 1.414213562 ) );
        t0 = t10 + t13;
        t3 = t10 - t13;
        t1 = t11 + t12;
        t2 = t11 - t12;
        t4 = dataptr[ 8 * 1 ];
        t5 = dataptr[ 8 * 3 ];
        t6 = dataptr[ 8 * 5 ];
        t7 = dataptr[ 8 * 7 ];
        z13 = t6 + t5;
        z10 = t6 - t5;
        z11 = t4 + t7;
        z12 = t4 - t7;
        t7 = z11 + z13;
        t11 = AMUL( (z11 - z13) * CONV( 1.414213562 ) );
        z5 = AMUL( (z10 + z12) * CONV( 1.847759065 ) );
        t10 = - z5 + AMUL( z12 * CONV( 1.082392200 ) );
        t12 = z5 - AMUL( z10 * CONV( 2.613125930 ) );
        t6 = t12 - t7;
        t5 = t11 - t6;
        t4 = t10 + t5;
        dataptr[ 8 * 0 ] = t0 + t7;
        dataptr[ 8 * 7 ] = t0 - t7;
        dataptr[ 8 * 1 ] = t1 + t6;
        dataptr[ 8 * 6 ] = t1 - t6;
        dataptr[ 8 * 2 ] = t2 + t5;
        dataptr[ 8 * 5 ] = t2 - t5;
        dataptr[ 8 * 4 ] = t3 + t4;
        dataptr[ 8 * 3 ] = t3 - t4;
        dataptr++;
    }
    dataptr = data;
    for( i = 0; i < 8; i++ ) 
    {
        t10 = dataptr[ 0 ] + dataptr[ 4 ];
        t11 = dataptr[ 0 ] - dataptr[ 4 ];
        t13 = dataptr[ 2 ] + dataptr[ 6 ];
        t12 = - t13 + AMUL( ( dataptr[ 2 ] - dataptr[ 6 ] ) * CONV( 1.414213562 ) );
        t0 = t10 + t13;
        t3 = t10 - t13;
        t1 = t11 + t12;
        t2 = t11 - t12;
        z13 = dataptr[ 5 ] + dataptr[ 3 ];
        z10 = dataptr[ 5 ] - dataptr[ 3 ];
        z11 = dataptr[ 1 ] + dataptr[ 7 ];
        z12 = dataptr[ 1 ] - dataptr[ 7 ];
        t7 = z11 + z13;
        t11 = AMUL( ( z11 - z13 ) * CONV( 1.414213562 ) );
        z5 = AMUL( ( z10 + z12 ) * CONV( 1.847759065 ) );
        t10 = - z5 + AMUL( z12 * CONV( 1.082392200 ) );
        t12 = z5 - AMUL( z10 * CONV( 2.613125930 ) );
        t6 = t12 - t7;
        t5 = t11 - t6;
        t4 = t10 + t5;
        dataptr[ 0 ] = t0 + t7;
        dataptr[ 7 ] = t0 - t7;
        dataptr[ 1 ] = t1 + t6;
        dataptr[ 6 ] = t1 - t6;
        dataptr[ 2 ] = t2 + t5;
        dataptr[ 5 ] = t2 - t5;
        dataptr[ 4 ] = t3 + t4;
        dataptr[ 3 ] = t3 - t4;
        dataptr += 8;
    }
}

int jpeg_readmarkers( jpeg_file_desc* jp ) 
{
    int marker, length, i, j, k, l, m;
    jpeg_huffman_table_t* hptr;
    for(;;) 
    {
        marker = jpeg_read_byte( jp );
        if( marker != 0xFF ) return 0;
        marker = jpeg_read_byte( jp );
        if( marker != 0xD8 ) 
	{
            length = jpeg_read_word( jp );
            length -= 2;
            switch( marker ) 
	    {
                case 0xC0: //sequental:
                    jp->data_precision = jpeg_read_byte( jp );
                    jp->height = jpeg_read_word( jp );
                    jp->width = jpeg_read_word( jp );
                    jp->num_components = jpeg_read_byte( jp );
                    if( length - 6 != jp->num_components * 3 ) return 0;
                    for( i = 0; i < jp->num_components; i++ ) 
		    {
                        jp->component_info[ i ].id = jpeg_read_byte( jp );
                        j = jpeg_read_byte( jp );
                        jp->component_info[ i ].h = ( j >> 4 ) & 0x0F;
                        jp->component_info[ i ].v = j & 0x0F;
                        jp->component_info[ i ].t = jpeg_read_byte( jp );
                    }
                    break;
                case 0xC1:
                case 0xC2: //progressive:
                case 0xC3:
                case 0xC5:
                case 0xC6:
                case 0xC7:
                case 0xC8:
                case 0xC9:
                case 0xCA:
                case 0xCB:
                case 0xCD:
                case 0xCE:
                case 0xCF:
                    return 0;
                    break;
                case 0xC4:
                    while( length > 0 ) 
		    {
                        k = jpeg_read_byte( jp );
                        if( k & 0x10 ) 
			    hptr = &jp->hac[ k & 0x0F ];
                        else 
			    hptr = &jp->hdc[ k & 0x0F ];
                        for( i = 0, j = 0; i < 16; i++ ) 
			{
                            hptr->bits[ i ] = jpeg_read_byte( jp );
                            j += hptr->bits[ i ];
                        }
                        length -= 17;
                        for( i = 0; i < j; i++ )
                            hptr->hval[ i ] = jpeg_read_byte( jp );
                        length -= j;
                        for( i = 0, k = 0, l = 0; i < 16; i++ ) 
			{
                            for( j = 0; j < hptr->bits[ i ]; j++, k++ ) 
			    {
                                hptr->size[ k ] = i + 1;
                                hptr->code[ k ] = l++;
                            }
                            l <<= 1;
                        }
                    }
                    break;
                case 0xDB:
                    while( length > 0 ) 
		    {
                        j = jpeg_read_byte( jp );
                        k = ( j >> 4 ) & 0x0F;
                        for( i = 0; i < 64; i++ ) 
			{
                            if( k ) 
				jp->qtable[ j ][ i ] = jpeg_read_word( jp );
                            else
				jp->qtable[ j ][ i ] = jpeg_read_byte( jp );
                        }
                        length -= 65;
                        if( k ) length -= 64;
                    }
                    break;
                case 0xD9:
                    return 0;
                    break;
                case 0xDA:
                    j = jpeg_read_byte( jp );
                    for( i = 0; i < j; i++ ) 
		    {
                        k = jpeg_read_byte( jp );
                        m = jpeg_read_byte( jp );
                        for( l = 0; l < jp->num_components; l++ )
			{
                            if( jp->component_info[ l ].id == k ) 
			    {
                                jp->component_info[ l ].td = ( m >> 4 ) & 0x0F;
                                jp->component_info[ l ].ta = m & 0x0F;
                            }
			}
                    }
                    jp->scan.ss = jpeg_read_byte( jp );
                    jp->scan.se = jpeg_read_byte( jp );
                    k = jpeg_read_byte( jp );
                    jp->scan.ah = ( k >> 4 ) & 0x0F;
                    jp->scan.al = k & 0x0F;
                    return 1;
                    break;
                case 0xDD:
                    jp->restart_interval = jpeg_read_word( jp );
                    break;
                default:
                    fseek( jp->file, length, 1 );
                    break;
            }
        }
    }
}

int g_jpeg_zigzag[ 64 ] = 
{
     0, 1, 5, 6,14,15,27,28,
     2, 4, 7,13,16,26,29,42,
     3, 8,12,17,25,30,41,43,
     9,11,18,24,31,40,44,53,
    10,19,23,32,39,45,52,54,
    20,22,33,38,46,51,55,60,
    21,34,37,47,50,56,59,61,
    35,36,48,49,57,58,62,63 
};
int g_aanscale[ 8 ] = 
{
    CONV( 1.0f ), CONV( 1.387039845f ), CONV( 1.306562965f ), CONV( 1.175875602f ),
    CONV( 1.0f ), CONV( 0.785694958f ), CONV( 0.541196100f ), CONV( 0.275899379f ) 
};

void jpeg_decompress( jpeg_file_desc* jp ) 
{
    int vector[ 64 ], dct[ 64 ];
    int x, y, i, j, k, l, c;
    int X, Y, H, V, plane, scaleh[ 3 ], scalev[ 3 ];
    scaleh[ 0 ] = 1;
    scalev[ 0 ] = 1;
    if( jp->num_components == 3 ) 
    {
        scaleh[ 1 ] = jp->component_info[ 0 ].h / jp->component_info[ 1 ].h;
        scalev[ 1 ] = jp->component_info[ 0 ].v / jp->component_info[ 1 ].v;
        scaleh[ 2 ] = jp->component_info[ 0 ].h / jp->component_info[ 2 ].h;
        scalev[ 2 ] = jp->component_info[ 0 ].v / jp->component_info[ 2 ].v;
    }
    mem_set( jp->dc, sizeof( int ) * 3, 0 );
    for( Y = 0; Y < jp->height; Y += jp->component_info[ 0 ].v << 3 ) 
    {
        if( jp->restart_interval > 0 ) jp->curbit = 0;
        for( X = 0; X < jp->width; X += jp->component_info[ 0 ].h << 3 ) 
	{
            for( plane = 0; plane < jp->num_components; plane++ )
	    {
                for( V = 0; V < jp->component_info[ plane ].v; V++ )
		{
                    for( H = 0; H < jp->component_info[ plane ].h; H++ ) 
		    {
                        i = jpeg_huffmancode( &jp->hdc[ jp->component_info[ plane ].td ], jp );
                        i &= 0x0F;
                        vector[ 0 ] = jp->dc[ plane ] + jpeg_bit2int( i, jpeg_read_bits( i, jp ) );
                        jp->dc[ plane ] = vector[ 0 ];
                        i = 1;
                        while( i < 64 ) 
			{
                            j = jpeg_huffmancode( &jp->hac[ jp->component_info[ plane ].ta ], jp );
                            if( j == 0 ) 
			    {
				while( i < 64 ) vector[ i++ ] = 0;
			    }
                            else 
			    {
                                k = i + ( ( j >> 4 ) & 0x0F );
                                while( i < k ) vector[ i++ ] = 0;
                                j &= 0x0F;
                                vector[ i++ ] = jpeg_bit2int( j, jpeg_read_bits( j, jp ) );
                            }
                        }
                        k = jp->component_info[ plane ].t;
                        for( y = 0, i = 0; y < 8; y++ )
			{
                            for( x = 0; x < 8; x++, i++ ) 
			    {
                                j = g_jpeg_zigzag[ i ];
                                dct[ i ] = AMUL( vector[ j ] * jp->qtable[ k ][ j ] * g_aanscale[ x ] * g_aanscale[ y ] );
                            }
			}
                        jpeg_idct( dct );
                        for( y = 0; y < 8; y++ )
			{
                            for( x = 0; x < 8; x++ ) 
			    {
                                c = ( AMUL( dct[ ( y << 3 ) + x ] ) >> 3 ) + 128;
                                if( c < 0 ) 
				    c = 0;
                                else 
				    if( c > 255 ) c = 255;
                                if( scaleh[ plane ] == 1 && scalev[ plane ] == 1 ) 
				{
                                    i = X + x + ( H << 3 );
                                    j = Y + y + ( V << 3 );
                                    if( i < jp->width && j < jp->height )
                                        jp->data[ ( ( j * jp->width + i ) * jp->num_components ) + plane ] = c;
                                }
                                else 
				{
				    for( l = 0; l < scalev[ plane ]; l++ )
				    {
					for( k = 0; k < scaleh[ plane ]; k++ ) 
					{
					    i = X + ( x + ( H << 3 ) ) * scaleh[ plane ] + k;
					    j = Y + ( y + ( V << 3 ) ) * scalev[ plane ] + l;
					    if( i < jp->width && j < jp->height )
						jp->data[ ( ( j * jp->width + i ) * jp->num_components ) + plane ] = c;
					}
				    }
                                }
                            }
			}
                    }
		}
	    }
        }
    }
}

void jpeg_ycbcr2rgb( uint8_t* dest, uint8_t* src, int width, int height )
{
    int Y, Cb, Cr, R, G, B;
    for( size_t i = 0; i < width * height * 3; i += 3 ) 
    {
        Y = src[ i ];
        Cb = src[ i + 1 ] - 128;
        Cr = src[ i + 2 ] - 128;
        R = (int)( Y + AMUL( CONV( 1.40200 ) * Cr ) );
        G = (int)( Y - AMUL( CONV( 0.34414 ) * Cb ) - AMUL( CONV( 0.71414 ) * Cr ) );
        B = (int)( Y + AMUL( CONV( 1.77200 ) * Cb ) );
        if( R < 0 ) R = 0; else if( R > 255 ) R = 255;
        if( G < 0 ) G = 0; else if( G > 255 ) G = 255;
        if( B < 0 ) B = 0; else if( B > 255 ) B = 255;
        dest[ i ] = R;
        dest[ i + 1 ] = G;
        dest[ i + 2 ] = B;
    }
}

void jpeg_ycbcr2grayscale( uint8_t* dest, uint8_t* src, int width, int height )
{
    for( int i = 0; i < width * height; i++ ) 
    {
        *dest = *src;
	dest++;
	src += 3;
    }
}

//convert_to: 1 - grayscale; 2 - rgb;
jpeg_file_desc* jpeg_load( const char* filename, FILE* f, int convert_to )
{
    jpeg_file_desc* jp = (jpeg_file_desc*)MEM_NEW( HEAP_DYNAMIC, sizeof( jpeg_file_desc ) );
    mem_set( jp, sizeof( jpeg_file_desc ), 0 );
    int rv = -1;
    while( 1 )
    {
	if( filename && !f )
	{
    	    f = fopen( filename, "rb" );
	}
        jp->file = f;
	if( jp->file == 0 )
	{
    	    slog( "JPEG decoder error: no file\n" );
    	    break;
	}
	jpeg_readmarkers( jp );
	if( jp->width == 0 || jp->height == 0 )
	{
	    slog( "JPEG decoder error: wrong image size %d %d %d\n", jp->width, jp->height, jp->num_components );
    	    break;
	}
	jp->data = (uint8_t*)MEM_NEW( HEAP_DYNAMIC, ( jp->width * jp->height * 3 ) );
	jpeg_decompress( jp );

	switch( convert_to )
	{
	    case 1: //GRAYSCALE
		if( jp->num_components == 3 )
		    jpeg_ycbcr2grayscale( jp->data, jp->data, jp->width, jp->height );
		break;
	    case 2: //RGB
		if( jp->num_components == 3 )
		    jpeg_ycbcr2rgb( jp->data, jp->data, jp->width, jp->height );
		break;
	}

	rv = 0;
	break;
    }
    if( rv )
    {
	mem_free( jp );
	jp = NULL;
    }
    return jp;
}
