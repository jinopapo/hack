#include "inpgm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define GRAY 1

void skip_comment(FILE *pf);

void imread(uint8_t* *_I,unsigned int *_sy,unsigned int *_sx, unsigned int *_dim, unsigned int *_mode, const char *fname )
{
  FILE *pf;
  unsigned int dim,mode;
  unsigned int iFormat;
  unsigned int sy,sx,tone;
  unsigned int nPix;
  unsigned int szImgByte;

  if((pf = fopen(fname,"r")) ==NULL)
  {
    printf("Can not open file\n");
    exit(-1);
  }

  fscanf( pf,"P%d\n",&iFormat);
  switch(iFormat)
  {
  case 5:
    mode = GRAY;
    dim = 1;
    break;
  default:
    printf("Do not support format\n");
    exit(-1);
    break;
  }

  skip_comment(pf);

  fscanf(pf,"%d %d %d ",&sx,&sy,&tone);

  nPix = sy * sx;
  szImgByte = nPix * dim * sizeof(uint8_t);

  uint8_t *I = (uint8_t*)malloc( nPix * dim * sizeof(uint8_t) );

  fread(I,szImgByte,1,pf);

  *_I = I;
  *_sy = sy;
  *_sx = sx;
  *_dim = dim;
  *_mode = mode;
}

void skip_comment( FILE *pf )
{
  char c;

  while( ( c = fgetc( pf ) ) != EOF )
  {
    // 数字なら１文字戻して終了
    if( isdigit( c ) )
    {
      fseek( pf, -1, SEEK_CUR );
      return;
    }
    // コメントなら１行分読みとばす．
    else if ( c == '#' )
    {
      while ( ( c = fgetc( pf ) ) != '\n' && c != EOF );
    }
    else
    {
      printf("Do not support format\n");
      exit(-1);
    }
  }
}

void im2double( double *dst, const uint8_t *src, unsigned int   size )
{
  unsigned int   i;
  double a = 1.0/255.0;
  for ( i = 0; i < size; ++i ) dst[i] = a*(double)src[i];
}

void zeros( void *I, unsigned int   nelem, const char *type )
{
  if( strcmp( "uint8",  type ) == 0 ) { *((uint8_t**) I) = (uint8_t*) malloc( nelem*sizeof(uint8_t)  ); }
  else if ( strcmp( "uint16", type ) == 0 ) { *((uint16_t**)I) = (uint16_t*)malloc( nelem*sizeof(uint16_t) ); }
  else if ( strcmp( "uint32", type ) == 0 ) { *((uint32_t**)I) = (uint32_t*)malloc( nelem*sizeof(uint32_t) ); }
  else if ( strcmp( "int8",   type ) == 0 ) { *((int8_t**)  I) = (int8_t*)  malloc( nelem*sizeof(int8_t)   ); }
  else if ( strcmp( "int16",  type ) == 0 ) { *((int16_t**) I) = (int16_t*) malloc( nelem*sizeof(int16_t)  ); }
  else if ( strcmp( "int32",  type ) == 0 ) { *((int32_t**) I) = (int32_t*) malloc( nelem*sizeof(int32_t)  ); }
  else if ( strcmp( "single", type ) == 0 ) { *((float**)   I) = (float*)   malloc( nelem*sizeof(float)    ); }
  else if ( strcmp( "double", type ) == 0 ) { *((double**)  I) = (double*)  malloc( nelem*sizeof(double)   ); }
  else if ( strcmp( "uint64", type ) == 0 ) { *((uint64_t**)   I) = (uint64_t*)   malloc( nelem*sizeof(float)    ); }
  else if ( strcmp( "unsigned", type ) == 0 ) { *((unsigned int**)   I) = (unsigned int*)   malloc( nelem*sizeof(unsigned int)    ); }

}



void imwrite( const uint8_t *I, unsigned int   sy, unsigned int   sx, unsigned int   dim, unsigned int   mode, const char *fname )
{
	FILE *pf;

	// ファイルオープン
#ifdef __WIN__
	if ( ( pf = fopen( fname, "wb" ) ) == NULL ) // windows 用
#else
	if ( ( pf = fopen( fname, "w" ) ) == NULL ) // gcc 用
#endif
	{	fprintf( stderr, " Can not open the file %s\n", fname );
		exit( -1 );
	}

	// 画像のデータ量（バイト）
	unsigned int   nPix = sy*sx;
	unsigned int   szImgByte = nPix*dim*sizeof(uint8_t);

	//
	// ヘッダ部の書き出し
	//
	unsigned int   iFormat; // フォーマット番号の設定
	switch ( mode )
	{
          //case MODE_RGB    : iFormat = 6; break;
	case GRAY   : iFormat = 5; break;
          //case MODE_BINARY : iFormat = 4; break;
	}

	fprintf( pf, "P%d\n", iFormat );
	fprintf( pf, "%d %d\n", sx, sy ); // サイズの書き出し

	switch ( mode )
	{
          //case MODE_RGB:
	case GRAY:
		fprintf( pf, "%d\n", 255 ); // 階調数の書き出し
	}

	switch ( mode )
	{
          //case MODE_RGB:  // そのまま書き出し
	case GRAY:
		fwrite( I, szImgByte, 1, pf );
		break;

                /*case MODE_BINARY: // ８バイト分のデータを８ビットにまとめる

		uint8_t  c = 0;  // ８ビット値
		uint8_t  k = 0;  // ビットカウンタ
		unsigned int   ix = 0; // 横幅カウンタ

		for ( unsigned int   i = 0; i < nPix; ++i )
		{
			c |= ( I[i] & 1 ) << ( 7 - k ); // 出現順に上位ビットに配置

			// 8 ビット毎に書き出しとクリア
			if ( ++k >= 8 ) { fputc( c, pf );  c = 0;  k = 0; }

			// 横幅毎にクリア
			if ( ++ix >= sx ) { fputc( c, pf );  ix = 0;  c  = 0;  k  = 0; }
		}
		break;*/
	}


	if ( ferror( pf ) )
	{	printf( "\n Fail to write the image data\n\n" );
		fclose( pf );
		exit( -1 );
	}

	// ファイルクローズ
	fclose( pf );
}



void im2uint8( uint8_t *dst, const double *src, unsigned int   size )
{
	unsigned int   i;
	double t;
	for ( i = 0; i < size; ++i )
	{
		t = 255.0 * src[i];
		// 0 から 255 の範囲に補正
		dst[i] = (t<0)?(0):((t>255)?(255):((uint8_t)t));
	}
}
