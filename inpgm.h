#ifndef __INPGM_H__
#define __INPGM_H__

#include <stdlib.h>
#include <stdint.h>

void imread(uint8_t* *_I,unsigned int *_sy,unsigned int *_sx, unsigned int *_dim, unsigned int *_mode, const char *fname );
void im2double( double *dst, const uint8_t *src, unsigned int   size );
void zeros( void *I, unsigned int   nelem, const char *type );
void im2uint8( uint8_t *dst, const double *src, unsigned int   size );
void imwrite( const uint8_t *I, unsigned int   sy, unsigned int   sx, unsigned int   dim, unsigned int   mode, const char *fname );
#endif
