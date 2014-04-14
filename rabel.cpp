#include "inpgm.h"

#include <stdio.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cmath>

#define samv 10

struct point
{
  double x;
  double y;
  double z;
};

void binary(double* I,unsigned int nelem);
void edge(const double* I,double *E,double *B,unsigned int sx,unsigned int sy,unsigned int dim);
void sample(unsigned int sy,unsigned int sx,double *B);
void label(unsigned int sy,unsigned int sx,double *B,int h);

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
          //if(sam % samv == 0){
            B[ry * sx + rx] = h;
            //}
        }while(E[ry * sx + rx] != 1);
        label(sy,sx,B,h);
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
  bool search = false;
  unsigned int maxy,maxx;
  double h = 1;
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
          of << x/sx;
          of.put(' ');
          of << h/50;
          of.put(' ');
          of << y/sy << std::endl;
          B[iy * sx + ix] = 1;
        }else{
          maxx != 0 ? x = maxx : x = 0.01;
          maxy != 0 ? y = maxy : y = 0.01;
          of << x/sx;
          of.put(' ');
          of << B[maxy * sx + maxx]/50;
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

void label(unsigned int sy,unsigned int sx,double *B,int h)
{
  bool search = false;
  bool label = false;
  unsigned int line;
  for(unsigned int y = 0;y < sy;y++){
    line = 0;
    search = false;
    for(unsigned int x = 0;x < sx;x++){
      if(x != 0 && B[y * sx + x] > B[y * sx + x - 1] && B[y * sx + x] == h)line++;
      if(line != 0 && line % 2 == 0)search = true;
    }
    if(search){
      for(unsigned int x = 0;x < sx;x++){
        if(!label){
          if(B[y * sx + x] == h)label = true;
        }else{
          if(B[y * sx + x] == h && B[y * sx + x + 1] != B[y * sx + x])label = false;
          B[y * sx + x] = h;
        }
      }
      label = false;
    }
  }
}
