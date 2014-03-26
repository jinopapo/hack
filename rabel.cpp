#include "inpgm.h"

#include <stdio.h>
#include <algorithm>
#include <vector>

struct point
{
  double x;
  double y;
  double z;
};

void binary(double* I,unsigned int nelem);
void edge(const double* I,double *E,double *B,unsigned int sx,unsigned int sy,unsigned int dim);
void label(const double* I,unsigned int* L,unsigned int sx,unsigned int sy,unsigned int dim);
std::vector<point> sample(double *B,unsigned int sx,unsigned int sy);


int main(int argc,char * argv[])
{
  char *ifname;

  if(argc != 2){
    printf("Input image name");
    exit(-1);
  }

  double *I;
  uint8_t *_I;
  unsigned int *L;
  double *E;
  uint8_t *_E;
  double *B;
  unsigned int sy,sx,dim,mode;
  unsigned int npix,nelem;
  std::vector<point> sam;

  ifname = argv[1];
  imread(&_I,&sy,&sx,&dim,&mode,ifname);

  npix  = sy*sx;
  nelem = npix*dim;

  zeros( &I, nelem, "double" );
  zeros( &L, npix, "unsigned" );
  zeros( &E, nelem, "double" );
  zeros( &B, nelem, "double" );
  im2double(I,_I,nelem);

  binary(I,nelem);
  edge(I,E,B,sx,sy,dim);
  sam = sample(B,sx,sy);
  //label(E,L,sx,sy,dim);

  zeros(&_E,nelem,"uint8");
  im2uint8(_E,E,nelem);
  imwrite(_E,sy,sx,dim,mode,"../hack/hoge.pgm");

  return 0;
}

void binary(double* I,unsigned int nelem)
{
  for(unsigned int i = 0;i < nelem;i++){
    if(I[i] > 0.5){
      I[i] = 1;
    }else{
      I[i] = 0;
    }
  }
}

std::vector<point> sample(double *B,unsigned int sx,unsigned int sy)
{
  unsigned int rx = 0;
  unsigned int ry = 0;
  int u = 1;
  int v = 1;
  point p;
  std::vector<point> sample;
  while(ry < sy){
    while(rx < sx){
      if(rx == sx -1){
        ry++;
        u = -1;
        v = 1;
      }
      else if(ry == sy -1){
        rx++;
        u = 1;
        v = -1;
      }      else if(rx == 0){
        ry++;
        u = 1;
        v = -1;
      }else if(ry == 0){
        rx++;
        u = -1;
        v = 1;
      }
      rx += u;
      ry += v;
      if(B[ry * sx + rx] != 0){
        p.x = rx;
        p.y = ry;
        p.z = B[ry * sx + rx];
        sample.push_back(p);
      }
    }
  }
  return sample;
}

void edge(const double* I,double* E,double* B,unsigned int sx,unsigned int sy,unsigned int dim)
{
  unsigned int ix,iy,vec;
  const double* pI;
  unsigned int rx,ry;
  int u,v,vu,vv;
  int sam = 0;
  int h = 0;
  for(iy = 0;iy < sy;iy++)
  {
    pI = I + iy * sx;
    for(ix = 1;ix < sx;ix++)
    {
      if(pI[ix - 1] - pI[ix] > 0 && E[iy * sx + ix] != 1)
      {
        vec = 4;
        rx = ix;
        ry = iy;
        h++;
        do{
          E[ry * sx + rx] = 1;
          switch(vec){
          case 1:
            u = 1;
            v = -1;
            vu = -1;
            vv = -1;
            vec = 7;
            break;
          case 2:
            u = -1;
            v = -1;
            vu = -1;
            vv = 1;
            vec = 1;
            break;
          case 3:
            u = -1;
            v = -1;
            vu = -1;
            vv = 1;
            vec = 1;
            break;
          case 4:
            u = -1;
            v = 1;
            vu = 1;
            vv = 1;
            vec = 3;
            break;
          case 5:
            u = -1;
            v = 1;
            vu = 1;
            vv = 1;
            vec = 3;
            break;
          case 6:
            u = 1;
            v = 1;
            vu = 1;
            vv = -1;
            vec = 5;
            break;
          case 7:
            u = 1;
            v = 1;
            vu = 1;
            vv = -1;
            vec = 5;
            break;
          case 8:
            u = 1;
            v = -1;
            vu = -1;
            vv = -1;
            vec = 7;
            break;
          }
          while(I[ry * sx + rx] != I[(ry + v) * sx + (rx + u)]){
            if(u != vu){
              u += vu;
              if(u == vu){
                vv = -vv;
              }
            }
            else if(v != vv){
              v += vv;
              if(v == vv){
                vu = -vu;
              }
            }
            vec++;
            if(vec == 9){
              vec = 1;
            }
          }
          ry += v;
          rx += u;
          sam++;
          if(sam % 5 == 0){
            B[ry * sx + rx] = h;
          }
        }while(E[ry * sx + rx] != 1);
      }
    }
  }

  /*double K[9] = { 0,-1, 0,
                 -1, 4,-1,
                  0,-1, 0};
  int v,u,ry = 1,rx = 1;
  unsigned int iy,ix,iy_begin =ry,iy_end = sy - ry,ix_begin = rx,ix_end = sx - rx;
  for(iy = iy_begin;iy < iy_end;iy++)
  {
    for(ix = ix_begin;ix < ix_end;ix++)
    {
      double sum = 0;
      unsigned int k = 0;
      for(v = -ry; v <= ry; v++)
      {
        for( u = -rx; u <= rx; u++)
        {
          unsigned int j = (iy + v) * sx + (ix + u);
          sum += K[k] * I[j];
          k++;
        }
      }
      if(sum < 0){
      sum = 0;
      }else if(sum > 0 ){
          sum = 1;
      }
      unsigned int i = iy * sx + ix;
      E[i] = 1 - sum;
    }
    }*/
}


void label(const double* I,unsigned int* L,unsigned int sx,unsigned int sy,unsigned int dim)
{
  unsigned int iy,ix;
  const unsigned int nPix = sy * sx;
  unsigned int iReg = 0;
  std::vector< unsigned int> Merge( nPix, 0 );
  for(iy = 0;iy < sy; iy++)
  {
    const double *pI = I + iy * sx;
    unsigned int *pL = L + iy * sx;
    unsigned int *pC = L + (iy  - 1)* sx;
    for(ix = 0;ix < sx;ix++)
    {
      if( pI[ix] == 0){continue;}

      unsigned int jReg1 = 0,jReg2 = 0,jReg3  = 0,jReg4 = 0;
      unsigned int jReg_min = nPix;
      if(0 < iy)
      {
        if ( 0 < ix )
        {
          jReg1 = pC[ ix-1 ];
          if ( jReg1 != 0 )
          {
            jReg_min = std::min( jReg_min, jReg1 );
          }
        }

        jReg2 = pC[ ix ];
        if ( jReg2 != 0 )
        {
          jReg_min = std::min( jReg_min, jReg2 );
        }

        if ( ix < sx-1 )
        {
          jReg3 = pC[ ix+1 ];
          if ( jReg3 != 0 )
          {
            jReg_min = std::min( jReg_min, jReg3 );
          }
        }
      }

      if  ( 0 < ix )
      {
        jReg4 = pL[ ix-1 ];
        if  ( jReg4 != 0 )
        {
          jReg_min = std::min( jReg_min, jReg4 );
        }
      }

      if( jReg_min != nPix )
      {
        pL[ ix ] = jReg_min;
      }
      else
      {
        pL[ ix ] = ++iReg;
        Merge[ iReg ]  = iReg;
      }

      if  ( jReg1 != 0 && jReg_min <= Merge[ jReg1 ] ){ Merge[ jReg1 ] = jReg_min; }
      if  ( jReg2 != 0 && jReg_min <= Merge[ jReg2 ] ){ Merge[ jReg2 ] = jReg_min; }
      if  ( jReg3 != 0 && jReg_min <= Merge[ jReg3 ] ){ Merge[ jReg3 ] = jReg_min; }
      if  ( jReg4 != 0 && jReg_min <= Merge[ jReg4 ] ){ Merge[ jReg4 ] = jReg_min; }

    }
  }
  for ( unsigned int   i = 0; i < nPix; ++i )
  {
    unsigned int   idx;
    unsigned int   idx_parent;

    idx_parent = Merge[ i ];

    if      ( idx_parent == 0 ) continue;
    else if ( idx_parent == i ) continue;

    for ( unsigned int   j = 0; j < nPix; ++j )
    {
      idx = idx_parent;
      idx_parent = Merge[ idx ];

      if ( idx_parent == idx )
      {
        Merge[ i ] = idx_parent;
        break;
      }
    }
  }

  {
    std::vector< unsigned int   > Order( nPix, 0 );

    unsigned int   idx,  idx_new,  count=0;

    for ( unsigned int   i = 0; i < nPix; ++i )
    {
      idx = Merge[ i ];
      if ( idx == 0 ) continue;

      idx_new = Order[ idx ];

      if ( idx_new == 0 )
      {
        Order[ idx ] = ++count;
        idx_new = count;
      }

      Merge[ i ] = idx_new;
    }
  }

  for ( iy = 0; iy < sy; ++iy )
  {
    unsigned int  *pL = L + iy*sx;
    for  ( ix = 0; ix < sx; ++ix )
    {
      unsigned int  *p = &pL[ ix ];
      *p = Merge[ (int)*p ];
    }
  }

}


