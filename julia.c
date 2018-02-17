/***********************************************
 * JuliazoomGen3.c
 *
 *  Generates a RAW format file of an image 
 *  of a Generalized Julia set (grayscale)
 *
 *  C source file by Alberto Strumia
 *
 ************************************************/



#include<stdio.h>

/* definition of constants */

#define RADIUS 10
#define C_X 0.912365
#define C_Y 0.194280
#define SIDE 1.3
#define M 700
#define NUM 255
/* alternative values Side 0.13, 0.05 */

/* main program */

int main(int argc, char** argv)
{

	int p, q, n, w;	
	double x, y, xx, yy, Incx, Incy;

	FILE *fp;
	fp = fopen("Julia.raw","w");

	for (p = 1; p <= M; p++)
	{
		Incy = - SIDE + 2*SIDE/M*p;

		printf("%i %%\n", p*100/M);

		for (q = 1; q <= M; q++)
		{
			Incx = - SIDE + 2*SIDE/M*q;

			x =  Incx;
			y =  Incy;
			w = 200;

			for ( n = 1; n <= NUM; ++n)
			{
				xx = (x*x - y*y)*(x*x - y*y) - 4*x*x*y*y - C_X;
				yy = 4*x*y*(x*x - y*y) - C_Y;

				x = xx;
				y = yy;

				if ( x*x + y*y > RADIUS )
				{
					w = n;
					n = NUM;
				}
			}
			fprintf(fp, "%c", w );
		}
	}

	fclose(fp);
}

/* end of main program */
