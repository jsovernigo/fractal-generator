#include <png.h>

#include "fractals.h"

int intensity = 10;

/**
 *	parse_args
 *	parses the arguments passed in with the argument vector, and sets the flags for the program accordingly.
 *	IN:		argc		int - the agrument count
 *			argv		char* - the argument array.
 *			width		int* - the address of the width flag.
 *			height		int* - the address of the height flag.
 *			r			uint8_t* - the address of the red component flag
 *			g			uint8_t* - the address of the green component flag
 *			b			uint8_t* - the address of the blue component flag
 *			julia		int* - the address of the julia flag (sets the julia set vs mandelbrot construction)
 *			c_re		double* - the address of the real complex root.
 *			c_im		double* - the address of the imaginary complex root.
 *	OUT:	returns a valid file name if it was specified, or NULL if not specified.
 *	POST:	all address flags *may* have been updated.
 *	ERROR:
 */
char* parse_args(int argc, char** argv, int* width, int* height, uint8_t* r, uint8_t* g, uint8_t* b, int* julia, double* c_re, double* c_im)
{
	int i;
	char* fname;
	fname = NULL;

	i = 1;
	while (i < argc)
	{
		// this is for all arguments that require more than one item.
		if (i + 1 < argc)
		{
			if (strcmp(argv[i], "-r") == 0)
			{
				*r = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-g") == 0)
			{
				*g = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-b") == 0)
			{
				*b = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "--real") == 0)
			{
				*c_re =  atof(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "--imaginary") == 0)
			{
				*c_im =  atof(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-o") == 0)
			{
				fname = argv[i + 1];
				i++;
			}
			else if (strcmp(argv[i], "--intensity") == 0)
			{
				intensity = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-w") == 0)
			{
				*width = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-h") == 0)
			{
				*height = atoi(argv[i + 1]);
				i++;
			}

		}

		if (strcmp(argv[i], "--julia") == 0)
		{
			*julia = 1;
		}

		i++;
	}

	return fname;
}

int main(int argc, char** argv)
{
	int i;
	int width;
	int height;
	int julia;
	int no_file;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	double c_re;
	double c_im;
	char* fname;
	png_bytep* row_pointers;

	r = 255;
	g = 255;
	b = 255;

	julia = 0;
	c_re = -0.79;
	c_im = 0.15;
	width = 1920;
	height = 1080;
	no_file = 0;
	fname = NULL;

	fname = parse_args(argc, argv, &width, &height, &r, &g, &b, &julia, &c_re, &c_im);

	// this creates a default file name, if it does not get filled.
	if (fname == NULL)
	{
		no_file = 1;
		fname = malloc(sizeof(char) * 256);
		memset(fname, 0, 256);
		if (julia)
		{
			sprintf(fname, "(%dx%d)(%lfr%lfi).png", width, height, c_re, c_im);
		}
		else
		{
			sprintf(fname, "(%dx%d)(mandelbrot).png", width, height);
		}
	}

	row_pointers = get_rowpointers(width, height);

	// construct the series, write them to the bit map.
	if (!julia)
	{
		construct_mandelbrot(row_pointers, width, height, r, g, b, intensity);
	}
	else
	{
		construct_julia(row_pointers, width, height, c_re, c_im, r, g, b, intensity);
	}

	write_png_file(fname, width, height, row_pointers);


	/* -- Free allocated memory. -- */
	if (no_file)
	{
		free(fname);
	}

	for (i = 0; i < height; i++)
	{
		free(row_pointers[i]);
	}
	free(row_pointers);

	return 0;
}
