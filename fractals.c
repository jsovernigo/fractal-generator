/**
 *	Giuliano Sovernigo
 *
 *	This is the primary file in the fractal writer program.
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

// library specific includes.
#include <png.h>
#include <math.h>

// This is the *precision* of the fractal... decrease to speed up, but decrease your intensity!
#define MAX 1000

png_byte color_type;
png_byte bit_depth;
int number_of_passes;
int intensity = 10;

/**
 *	this function produces colour->black scaling adjustments.  It is hard to describe, 
 *	but essentially:
 *	as a number increases from 0->100, it loses very little colour, until it reaches
 *	a critical point determined by the intensity, which represents the power of the
 *	polynomial expression.  This produces a slow fade from white, then a rapid drop off to black.
 */
#define MUTATE(x) pow(x, intensity)

/**
 * This could be a function, but it really does not deserve a whole stack frame.
 * it sets a pixel pointer, ptr, to a specific colour, denoted by the colour struct.
 */
#define SET_PIXEL(ptr, colour) ptr[0] = colour.r; ptr[1] = colour.g; ptr[2] = colour.b; ptr[3] = colour.a;


struct colour
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};


/**
 *	make_colour
 *	produces a colour struct, given r, g, b, a, values.
 *	IN:			r		uint8_t - the red component.
 *				g		uint8_t - the green component.
 *				b		uint8_t - the blue component.
 *				a		uint8_t - the alpha contenet.
 *	OUT:		a new colour struct, built by the function.
 *	POST:		colour returned.
 *	ERROR:		be wary of uint8_t overflow!!
 */
struct colour make_colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	struct colour new_colour;
	new_colour.r = r;
	new_colour.g = g;
	new_colour.b = b;
	new_colour.a = a;

	return new_colour;
}


/**
 *	fade
 *	calculates a percentage fade for two colours.
 *	IN:			from		struct colour - the starting colour.
 *				to			struct colour - the ending colour
 *				percentage	double - the percentage fade from from to to.
 *	OUT:		a new colour that represents the faded colour between from and to.
 *	POST:		colour returned.
 *	ERROR:		none.
 */
struct colour fade(struct colour from, struct colour to, double percentage)
{
	struct colour n;

	n.r = (uint8_t)( (double)from.r + (double) (to.r - from.r) * ((double)percentage));
	n.g = (uint8_t)( (double)from.g + (double) (to.g - from.g) * ((double)percentage));
	n.b = (uint8_t)( (double)from.b + (double) (to.b - from.b) * ((double)percentage));

	// capping statements for red, green, and blue
	if (n.r > 255)
	{
		n.r = 255;
	}
	else if (n.r < 0)
	{
		n.r = 0;
	}

	if (n.g > 255)
	{
		n.g = 255;
	}
	else if (n.g < 0)
	{
		n.g = 0;
	}

	if (n.b > 255)
	{
		n.b = 255;
	}
	else if (n.b < 0)
	{
		n.b = 0;
	}

	n.a = 255;

	return n;
}


/**
 *	write_progress
 *	writes the progress of the iterations to the screen.
 *	TODO this is a target for an upgrade to an in-place bar later.
 *	IN:			max			int - the "one hundred"
 *				done		done - the current amount of max done.
 *	OUT:		the double percentage of progress.
 *	POST:		percentage and newline written to the screen.
 *	ERROR:		done > max.
 */
double write_progress(int max, int done)
{
	double percent;

	percent = ((double) done) / ((double) max);
	printf("[%3.2lf%%]\n", percent * 100.00);
	return percent;
}


/**
 *	get_rowpointers
 *	mallocs and returns a set of row pointers, based on the width and height passed in.
 *	IN:			width		int - the integer width of the image that you want to construct.
 *				height		int - the height of the image, in integer form.
 *	OUT:		an allocated array of byte pointers that you can write to.
 *	POST:		malloc'd memory returned.
 *	ERROR:		width/height negative.
 */
png_bytep* get_rowpointers(int width, int height)
{
	int i;
	png_bytep* row_pointers;

	row_pointers = malloc(sizeof(png_bytep) * height);
	if (row_pointers == NULL)
	{
		return NULL;
	}

	for (i = 0; i < height; i++)
	{
		row_pointers[i] = malloc(0x04 * width);
	}

	return row_pointers;
}


/**
 *	write_png_file
 *	writes the png information in row_pointers to the defined file.
 *	IN:		fname			char* - the file output name.
 *			width			int - the width of the image
 *			height			int - the height of the image.
 *
 */
int write_png_file(char* fname, int width, int height, png_bytep* row_pointers)
{
	FILE* fp;
	png_structp png_ptr;
	png_infop info_ptr;

	fp = fopen(fname, "wb+");

	if (!fp)
	{
		return -1;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		return -2;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		return -3;
	}

	png_init_io(png_ptr, fp);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		return -4;
	}

	png_set_IHDR(png_ptr, info_ptr, width, height, 8, 6, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		return -5;
	}
	png_write_image(png_ptr, row_pointers);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		return -6;
	}

	png_write_end(png_ptr, NULL);

	fclose(fp);

	return 0;
}

/**
 *	construct_mandelbrot
 *	builds a mandelbrot set in the row pointers, colouring using the recursive depth model.
 *	IN:		row_pointers	png_bytep* - the list of png image row pointers.
 *			width			int - the width of the image.
 *			height			int - the height of the image.
 *			r				uint8_t - the red component
 *			g				uint8_t - the green component
 *			b				uint8_t - the blue component
 *	OUT:	nothing returned.
 *	POST:	a mandelbrot set has been written to the row_pointers that you provided.
 *	ERROR:	row_pointers is null.
 */
void construct_mandelbrot(png_bytep* row_pointers, int width, int height, uint8_t r, uint8_t g, uint8_t b)
{
	int i;
	int j;

	for (i = 0; i < height; i++)
	{
		png_byte* row;
		row = row_pointers[i];

		for (j = 0; j < width; j++)
		{
			int iteration;
			double x;
			double y;
			double c_re;
			double c_im;

			png_byte* ptr;
			ptr = &(row[j*4]);

			c_re = (j - width/2.0) * 4.0/width;
			c_im = (i - height/2.0) * 4.0/width;

			x = 0;
			y = 0;

			iteration = 0;

			while(x * x + y * y <= 4 && iteration < MAX)
			{
				double x_new;
				x_new = x * x - y * y + c_re;
				y = 2 * x * y + c_im;
				x = x_new;

				iteration++;
			}

			// NOT PART OF THE SET
			if (iteration < MAX)
			{
				double percent;
				struct colour from;
				struct colour to;

				struct colour faded;

				to = make_colour(0,0,0,255);
				from = make_colour(r, g, b, 255);

				percent = (((double) MAX - (double) iteration )/ (double)MAX);

				faded = fade(from, to, MUTATE(percent));

				SET_PIXEL(ptr, faded);
			}
			else
			{
				struct colour c;
				c = make_colour(0, 0, 0, 255);
				SET_PIXEL(ptr, c);
			}
		}

		write_progress(height, i);
	}

	return;
}


/**
 *	construct_julia
 *	builds a julia set in the row pointers provided, with the complex orbit
 *	described by the complex number provided.
 *	IN:		row_pointers	png_bytep* - the list of png image row pointers.
 *			width			int - the width of the image.
 *			height			int - the height of the image.
 *			c_re			double - the real component of the complex orbit.
 *			c_im			double - the imaginary component of the orbit.
 *			r				uint8_t - the red component
 *			g				uint8_t - the green component
 *			b				uint8_t - the blue component
 *	OUT:	nothing returned.
 *	POST:	a julia set has been written to the row_pointers provided.
 *	ERROR:	row_pointers is null.
 */
void construct_julia(png_bytep* row_pointers, int width, int height, double c_re, double c_im, uint8_t r, uint8_t g, uint8_t b)
{
	int i;

	// iterate through the rows
	for (i = 0; i < height; i++)
	{
		int j;

		png_byte* row;
		row = row_pointers[i];

		// iterate through the columns.
		for (j = 0; j < width; j++)
		{
			int iteration;
			double z_re;
			double z_im;

			png_byte* ptr;
			ptr = &(row[j*4]);

			// this forces the values here to bound within the [-2,2] range we need.
			z_re = (j - width/2.0) * 4.0/width;
			z_im = (i - height/2.0) * 4.0/width;

			iteration = 0;

			// while we stay bounded to the set
			while (z_re * z_re + z_im * z_im <= 4 && iteration < MAX)
			{
				double new_real;
				new_real = z_re * z_re - z_im * z_im+ c_re;
				z_im = 2 * z_re * z_im + c_im;

				z_re = new_real;

				iteration ++;
			}

			// external to the set
			if (iteration < MAX)
			{
				double percent;
				struct colour from;
				struct colour to;

				struct colour faded;

				to = make_colour(0,0,0,255);
				from = make_colour(r, g, b, 255);

				percent = (((double) MAX - (double) iteration )/ (double)MAX);

				faded = fade(from, to, MUTATE(percent));

				SET_PIXEL(ptr, faded);
			}
			else
			{
				struct colour c;
				c = make_colour(0, 0, 0, 255);
				SET_PIXEL(ptr, c);
			}
		}

		write_progress(height, i);
	}

	return;
}


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


/**
 *	main
 *	This is the main function of the program.  It handles the ordering of events
 *	and calls the png write procedures.
 *	IN:		argc		int - the argument count from command line.
 *			argv		char* - the argument array as passed in by the terminal.
 *	OUT:	returns 0 on complete execution, positive values otherwise.
 *	POST:	a file has been written, named by the passed in file name, or the
 *			execution parameters as specified by the width, height, and roots.
 *	ERROR:	many.
 */
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
		construct_mandelbrot(row_pointers, width, height, r, g, b);
	}
	else
	{
		construct_julia(row_pointers, width, height, c_re, c_im, r, g, b);
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
