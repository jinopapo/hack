#include "inpgm.h"

#include <stdio.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cmath>

#define samv 20

struct point
{
  double x;
  double y;
  double z;
};

void binary(double* I,unsigned int nelem);
void edge(const double* I,double *E,double *B,unsigned int sx,unsigned int sy,unsigned int dim);
void label(const double* I,unsigned int* L,unsigned int sx,unsigned int sy,unsigned int dim);
void sample(unsigned int sy,unsigned int sx,double *B);

int main(int argc,char * argv[])
{

  if(argc != 2){
    printf("Input image name");
    exit(-1);
  }

  char *ifname;
  double *I;
  uint8_t *_I;
  unsigned int *L;
  double *E;
  uint8_t *_E;
  double *B;
  uint8_t *_B;
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
  sample(sy,sx,B);
  //label(E,L,sx,sy,dim);

  zeros(&_E,nelem,"uint8");
  zeros(&_B,nelem,"uint8");
  im2uint8(_B,B,nelem);
  imwrite(_B,sy,sx,dim,mode,"../hack/hoge.pgm");

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


void edge(const double* I,double* E,double* B,unsigned int sx,unsigned int sy,unsigned int dim)
{
  unsigned int ix,iy,vec;
  const double* pI;
  unsigned int rx,ry;
  int u,v,vu,vv;
  int sam = 0;
  int h = 2;
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
          if(sam % samv == 0){
            B[ry * sx + rx] = h;
          }
        }while(E[ry * sx + rx] != 1);
      }
    }
  }
}

void sample(unsigned int sy,unsigned int sx,double *B)
{
  std::ofstream of("toukou.dat");
  int v,u;
  sx % samv != 0 ? v = 0 : v = 1;
  sy % samv != 0 ? u = 0 : u = 1;
  of << ((sx/samv) + 1 - v) * ((sy/samv) + 1 - u) << std::endl;
  of << ((sx/samv - v) * (sy/samv - u)) * 2 << std::endl;
  unsigned int iy,ix;
  double x,y;
  unsigned int line = 0;
  bool search = false;
  unsigned int maxy,maxx;
  double h = 1;
  double maxh = 0;
  for(iy = 0;iy < sy;iy++){
    for(ix = 0;ix < sx;ix++){
      maxy = 0;
      maxx = 0;
      if(ix % samv == 0 && iy % samv == 0){
        if(ix != 0 && iy < sy - samv/2){
          for(unsigned int i = iy;i < iy + samv/2;i++){
            for(unsigned int j = ix - samv/2;j < ix;j++){
              if(B[i * sx + j] != 0){
                search = true;
                if(B[maxy * sx + maxx] < B[i * sx + j]){
                  if(maxx != 0 && maxy != 0){
                    B[maxy * sx + maxx] = 0;
                  }
                  maxy = i;
                  maxx = j;
                }
              }
            }
          }
        }
        if(iy != 0 && ix < sx - samv/2){
          for(unsigned int i = iy - samv/2;i < iy;i++){
            for(unsigned int j = ix;j < ix + samv/2;j++){
              if(B[i * sx + j] != 0){
                search = true;
                if(B[maxy * sx + maxx] < B[i * sx + j]){
                  if(maxx != 0 && maxy != 0){
                    B[maxy * sx + maxx] = 0;
                  }
                  maxy = i;
                  maxx = j;
                }
              }
            }
          }
        }
        if(iy != 0 && ix != 0){
          for(unsigned int i = iy - samv/2;i < iy;i++){
            for(unsigned int j = ix - samv/2;j < ix;j++){
              if(B[i * sx + j] != 0){
                search = true;
                if(B[maxy * sx + maxx] < B[i * sx + j]){
                  if(maxx != 0 && maxy != 0){
                    B[maxy * sx + maxx] = 0;
                  }
                  maxy = i;
                  maxx = j;
                }
              }
            }
          }
        }
        if(iy < sy - samv/2 && ix < sx - samv/2){
          for(unsigned int i = iy;i < iy + samv/2;i++){
            for(unsigned int j = ix;j < ix + samv/2;j++){
              if(B[i * sx + j] != 0){
                search = true;
                if(B[maxy * sx + maxx] < B[i * sx + j]){
                  if(maxx != 0 && maxy != 0){
                    B[maxy * sx + maxx] = 0;
                  }
                  maxy = i;
                  maxx = j;
                }
              }
            }
          }
        }
        if(!search){
          ix != 0  ? x = ix : x = 0.01;
          iy != 0  ? y = iy : y = 0.01;
          //if(line > 1)h--;
          of << x/sx;
          of.put(' ');
          of << h/50;
          of.put(' ');
          of << y/sy << std::endl;
          B[iy * sx + ix] = 1;
          line = 0;
        }else{
          //if(line == 0){
            maxh < B[maxy * sx + maxx] ? h++ : h--;
            //}
          maxh = h;
          line++;
          maxx != 0 ? x = maxx : x = 0.01;
          maxy != 0 ? y = maxy : y = 0.01;
          of << x/sx;
          of.put(' ');
          of << h/50;
          of.put(' ');
          of << y/sy << std::endl;
        }
        search = false;
      }
    }
  }
  for(unsigned int i = 0;i < ((sx/samv) + 1 - v) * ((sy/samv) + 1 - u);i++){
    if(fmod(i + 1,sx/samv + !v) != 0 && i + sx/samv + 1 - v < ((sx/samv) + 1 - v) * ((sy/samv) + 1 - u)){
      of << 3;
      of.put(' ');
      of << i + 1;
      of.put(' ');
      of << i;
      of.put(' ');
      of << i + sx/samv + 1 - v << std::endl;
      of << 3;
      of.put(' ');
      of << i + 1;
      of.put(' ');
      of << i + sx/samv + 1 - v;
      of.put(' ');
      of << i + sx/samv + 2 - v  << std::endl;
    }
  }
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


