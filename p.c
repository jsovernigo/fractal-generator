

/*

  c console program
  -----------------------------------------
  1.ppm file code is  based on the code of Claudio Rocchini
  http://en.wikipedia.org/wiki/Image:Color_complex_plot.jpg
  create 8 bit color graphic file ,  portable graymap file = PGM 
  see http://en.wikipedia.org/wiki/Portable_pixmap
  to see the file use external application ( graphic viewer)
  I think that creating graphic can't be simpler
  ---------------------------
  2. first it creates data array which is used to store 1 byte color values of pixels,
  fills tha array with data and after that writes the data (array)  to binary pgm file in one step.
  It alows free ( non sequential) acces to "pixels"
    
  -------------------------------------------
  Adam Majewski   fraktal.republika.pl 
 
  Sobel filter 
  Gh = sum of six values ( 3 values of matrix are equal to 0 ). Each value is = pixel_color * filter_coefficients 



  gcc e.c -lm -Wall -march=native -fopenmp
  time ./a.out
  convert oar0.000933333.pgm a.png


  ----------------------------------
  File oar0.000933333.pgm saved. 

  real	24m42.020s

  c = -0.040429288233396  +0.786653655622161 i    okres = 10000

==============================================
File oar0.000186667.pgm saved. 

real	1828m56.372s
user	14512m23.201s
sys	2m15.393s

 
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <omp.h> // OpenMP; needs also -fopenmp

/* iXmax/iYmax =  */

int iXmax = 5000; /* height of image in pixels */
int iYmax = 5000;
/* fc(z) = z*z + c */
int denominator =3; /* denominator of internal angle */
int periodOfParent=3;
int periodOfChild ; // = denominator*periodOfParent =  period of child component , not parent 

// it is impotran for quolity and time 
double AR; // PixelWidth /*   radius of circle around attractor ZA = target set for attracting points */
double  AR2; // (AR*AR)


/* escape time to infinity */
int GiveExtLastIteration(double _Zx0, double _Zy0,double C_x, double C_y, int iMax, double _ER2)
{ 
  int i;
  double Zx, Zy;
  double Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */
  Zx=_Zx0; /* initial value of orbit  */
  Zy=_Zy0;
  Zx2=Zx*Zx;
  Zy2=Zy*Zy;
  for (i=0;i<iMax && ((Zx2+Zy2)<_ER2);i++)
    {
      Zy=2*Zx*Zy + C_y;
      Zx=Zx2-Zy2 +C_x;
      Zx2=Zx*Zx;
      Zy2=Zy*Zy;
    };
  return i;
}


/* find attractor ZA  using forward iteration of critical point Z = 0  */
/* if period is >1 gives one point from attracting cycle */
double complex GiveAttractor(double complex C , double ER2, int _IterationMax)
{
  int Iteration;
  double Cx,Cy; /* C = Cx+Cy*I */
  double Zx, Zy; /* z = zx+zy*i */
  double Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */
  Cx = creal(C);
  Cy = cimag(C);
  /* -- find attractor ZA  using forward iteration of critical point Z = 0  */
  Zx=0.0;
  Zy=0.0;
  Zx2=Zx*Zx;
  Zy2=Zy*Zy;
  for (Iteration=0;Iteration<_IterationMax && ((Zx2+Zy2)<ER2);Iteration++)
    {
      Zy=2*Zx*Zy + Cy;
      Zx=Zx2-Zy2 + Cx;
      Zx2=Zx*Zx;
      Zy2=Zy*Zy;
    };
  return Zx+Zy*I;
}

/* attracting time to finite attractor ZA */
int GiveIntLastIteration(double _Zx0, double _Zy0,double C_x, double C_y, int iMax, double _AR2, double _ZAx, double _ZAy )
{ 
  int i;
  double Zx, Zy; /* z = zx+zy*i */
  double Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */
  double d, dX, dY; /* distance from z to Alpha  */
  Zx=_Zx0; /* initial value of orbit  */
  Zy=_Zy0;
  Zx2=Zx*Zx;
  Zy2=Zy*Zy;
  dX=Zx-_ZAx;
  dY=Zy-_ZAy;
  d=dX*dX+dY*dY;
  for (i=0;i<iMax && (d>_AR2);i++)
    {
      Zy=2*Zx*Zy + C_y;
      Zx=Zx2-Zy2 +C_x;
      Zx2=Zx*Zx;
      Zy2=Zy*Zy;
      dX=Zx-_ZAx;
      dY=Zy-_ZAy;
      d=dX*dX+dY*dY;
    };
  return i;
}



// gives c in hyperbolic component of Mandelbrot set
// #include <complex.h>
// turn is an internal angle in turns
// 0.0 <= radius <=1.0
double complex GiveC(int period, double radius, double turn)
{
  double Cx, Cy; /* C = Cx + Cy*i */
  double a = turn*(2*M_PI); // angle, from turns to radians
  switch( period )
    {
    case 1 :   Cx = radius*(0.5*cos(a) - 0.25*cos(2*a)); 
      Cy = radius*(0.5*sin(a) - 0.25*sin(2*a)); 
      break;

    case 2 :   Cx = radius*0.25*cos(a) - 1; 
      Cy = radius*0.25*sin(a) ; 
      break;

    default  : Cx=0; Cy=0; // 
      break;
    }		

  return Cx+Cy*I;
}

/* gives position of point (iX,iY) in 1D array  ; uses also global variables */
unsigned int f(unsigned int _iX, unsigned int _iY)
{return (_iX + (iYmax-_iY-1)*iXmax );}

/* --------------------------------------------------------------------------------------------------------- */

int main(){
  
   
  unsigned int iX,iY, /* indices of 2D virtual array (image) = integer coordinate */
    i, /* index of 1D array  */
    iLength = iXmax*iYmax;/* length of array in bytes = number of bytes = number of pixels of image * number of bytes of color */
  /* world ( double) coordinate = parameter plane*/
  const double ZxMin=-1.4;
  const double ZxMax=1.4;
  const double ZyMin=-1.4;
  const double ZyMax=1.4;
  double PixelWidth=(ZxMax-ZxMin)/iXmax;
  double PixelHeight=(ZyMax-ZyMin)/iYmax;
  /* */
  double Zx, Zy;    /* Z=Zx+Zy*i   */
  double complex ZA;  /* atractor ZA = ZAx + ZAy*i */
  double complex C;  /* atractor C = Cx + Cy*i */
  /* */
  
  const double EscapeRadius=2.0; /* radius of circle around origin; its complement is a target set for escaping points */
  double ER2=EscapeRadius*EscapeRadius;
  
  const int IterationMax=60,
    IterationMaxBig= 100000001;
  int eLastIteration, iLastIteration;
  
  
  
  periodOfChild = denominator*periodOfParent; // 

  /* sobel filter */
  unsigned char G, Gh, Gv; 
  /* color; length of array should be >= periodOfChild  !!!!!  */
  unsigned char color[9]={255,230,200,180,150,120, 90, 60,30}; /* shades of gray used in image */
  const unsigned int MaxColorComponentValue=255; /* color component is coded from 0 to 255 ;  it is 8 bit color file */

  /* dynamic 1D arrays for colors ( shades of gray ) */
  unsigned char *data, *edge;
  data = malloc( iLength * sizeof(unsigned char) );
  edge = malloc( iLength * sizeof(unsigned char) );
  if (data == NULL || edge==NULL)
    {
      fprintf(stderr," Could not allocate memory. End of the program. ");
      getchar(); 
      return 1; 
    }
  else printf(" memory is OK\n");
 
  

  // computed with program Mandel by Wolf Jung 
  C = -0.040429288233396  +0.786653655622161*I; //GiveC(periodOfParent, 1.0, 1.0/denominator);
  printf(" Cx = %f \n", creal(C));
  printf(" Cy = %f \n", cimag(C));
  

  ZA = GiveAttractor( C, ER2, IterationMaxBig); /* find attractor ZA  using forward iteration of critical point Z = 0  */
  printf(" ZAx = %f \n", creal(ZA));
  printf(" ZAy = %f \n", cimag(ZA));


  AR = PixelWidth/3.0;
  AR2=AR*AR;


  printf(" fill the data array \n");
#pragma omp parallel for schedule(dynamic) private(i,iX,iY,Zy, Zx, eLastIteration,iLastIteration) shared(iYmax,iXmax, ER2)
 
  for(iY=0;iY<iYmax;++iY){ 
    Zy=ZyMin + iY*PixelHeight; /*  */
    if (fabs(Zy)<PixelHeight/2) Zy=0.0; /*  */
    printf(" row %u from %u \n",iY, iYmax);    
    for(iX=0;iX<iXmax;++iX){ 
      Zx=ZxMin + iX*PixelWidth;
      eLastIteration = GiveExtLastIteration(Zx, Zy, creal(C), cimag(C), IterationMax, ER2 );
      i= f(iX,iY); /* compute index of 1D array from indices of 2D array */
      if ( IterationMax != eLastIteration ) 
	{data[i]=245;} /* exterior */
      else /* interior */
	{ iLastIteration = GiveIntLastIteration(Zx, Zy, creal(C), cimag(C), IterationMaxBig, AR2, creal(ZA), cimag(ZA));
          data[i]=color[iLastIteration % periodOfChild];} /*  level sets of attraction time */
      /*  if (Zx>0 && Zy>0) data[i]=255-data[i];    check the orientation of Z-plane by marking first quadrant */
    }
  }


  printf(" find boundaries in data array using  Sobel filter\n");   
#pragma omp parallel for schedule(dynamic) private(i,iX,iY,Gv,Gh,G) shared(iYmax,iXmax, ER2)
  for(iY=1;iY<iYmax-1;++iY){ 
    for(iX=1;iX<iXmax-1;++iX){ 
      Gv= data[f(iX-1,iY+1)] + 2*data[f(iX,iY+1)] + data[f(iX-1,iY+1)] - data[f(iX-1,iY-1)] - 2*data[f(iX-1,iY)] - data[f(iX+1,iY-1)];
      Gh= data[f(iX+1,iY+1)] + 2*data[f(iX+1,iY)] + data[f(iX-1,iY-1)] - data[f(iX+1,iY-1)] - 2*data[f(iX-1,iY)] - data[f(iX-1,iY-1)];
      G = sqrt(Gh*Gh + Gv*Gv);
      i= f(iX,iY); /* compute index of 1D array from indices of 2D array */
      if (G==0) {edge[i]=255;} /* background */
      else {edge[i]=0;}  /* boundary */
    }
  }



  /* ---------- file  -------------------------------------*/
  printf(" save  data array to the file \n");
  FILE * fp;
  char name [10]; /* name of file */
  i = sprintf(name,"oar%2.9f",AR); /* result (is saved in i) but is not used */
  char *filename =strcat(name,".pgm");
  char *comment="# C=";/* comment should start with # */
  /* save image to the pgm file  */      
  fp= fopen(filename,"wb"); /*create new file,give it a name and open it in binary mode  */
  fprintf(fp,"P5\n %s\n %u\n %u\n %u\n",comment,iXmax,iYmax,MaxColorComponentValue);  /*write header to the file*/
  fwrite(edge,iLength,1,fp);  /*write image data bytes to the file in one step */
  printf("File %s saved. \n", filename);
  fclose(fp);


  /* --------------free memory ---------------------*/
  free(data);
  free(edge);
  
  

  return 0;
}
