#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include <png.h>
#include <math.h>


#define RADIUS 10
#define C_X 0.912365
#define C_Y 0.194280
#define SIDE 1.3
#define M 700
#define NUM 255
#define MAX 1000

int intensity = 10;

#define MUTATE(x) pow(x, intensity)

#define SET_PIXEL(ptr, colour) ptr[0] = colour.r; ptr[1] = colour.g; ptr[2] = colour.b; ptr[3] = colour.a;

struct colour
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};


struct colour make_colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	struct colour new_colour;
	new_colour.r = r;
	new_colour.g = g;
	new_colour.b = b;
	new_colour.a = a;

	return new_colour;
}

struct colour random_colour(uint8_t opacity)
{
	struct colour new_colour;
	new_colour.r = rand() * 255;
	new_colour.g = rand() * 255;
	new_colour.b = rand() * 255;
	new_colour.a = opacity;

	return new_colour;
}

struct colour fade(struct colour from, struct colour to, double percentage)
{
	struct colour n;

	n.r = (uint8_t)( (double)from.r + (double) (to.r - from.r) * ((double)percentage));
	n.g = (uint8_t)( (double)from.g + (double) (to.g - from.g) * ((double)percentage));
	n.b = (uint8_t)( (double)from.b + (double) (to.b - from.b) * ((double)percentage));

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

png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;

double write_progress(int max, int done)
{
	double percent;

	percent = ((double) done) / ((double) max);
	printf("[%3.2lf%%]\n", percent * 100.00);
	return percent;
}

png_bytep* read_png_file(char* fname, int* width, int* height, size_t* bytes_per_row)
{
	int i;
	FILE* fp;
	png_bytep* row_pointers;
	unsigned char header[8];

	fp = fopen(fname, "rb");
	if (!fp)
	{
		return NULL;
	}

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
	{
		return NULL;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		return NULL;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		return NULL;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);

	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		return NULL;
	}

	row_pointers = malloc(sizeof(png_bytep) * *height);

	if (bit_depth == 16)
	{
		*bytes_per_row = 8;
	}
	else
	{
		*bytes_per_row = 4;
	}

	for (i = 0; i < *height; i++)
	{
		row_pointers[i] = malloc(*bytes_per_row * *width);
	}

	png_read_image(png_ptr, row_pointers);

	fclose(fp);

	return row_pointers;
}


int write_png_file(char* fname, int width, int height, size_t bytes_per_row, png_bytep* row_pointers)
{
	FILE* fp;

	fp = fopen(fname, "wb");

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

void write_mandelbrot(png_bytep* row_pointers, int width, int height, uint8_t r, uint8_t g, uint8_t b)
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

void write_julia(png_bytep* row_pointers, int width, int height, double c_re, double c_im, uint8_t r, uint8_t g, uint8_t b)
{
	int i;

	for (i = 0; i < height; i++)
	{
		int j;

		png_byte* row;
		row = row_pointers[i];

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


char* parse_args(int argc, char** argv, uint8_t* r, uint8_t* g, uint8_t* b, int* julia, double* c_re, double* c_im)
{
	int i;
	char* fname;
	fname = NULL;

	i = 1;
	while (i < argc)
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
		else if (strcmp(argv[i], "-julia") == 0)
		{
			*julia = 1;
		}
		else if (strcmp(argv[i], "-real") == 0)
		{
			*c_re = atof(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-imaginary") == 0)
		{
			*c_im = atof(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-f") == 0)
		{
			fname = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-intensity") == 0)
		{
			intensity = atoi(argv[i + 1]);
			i++;
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
	uint8_t r;
	uint8_t g;
	uint8_t b;
	int julia;
	double c_re; 
	double c_im;
	size_t bytes_per_row;
	char* fname;

	r = 255;
	g = 255;
	b = 255;

	julia = 0;
	c_re = -0.79;
	c_im = 0.15;

	png_bytep* row_pointers;

	srand(time(0));

	fname = parse_args(argc, argv, &r, &g, &b, &julia, &c_re, &c_im);
	if (fname == NULL)
	{
		puts("Minimum specification is `-file <file path>`.");
		exit(1);
	}

	row_pointers = read_png_file(fname, &width, &height, &bytes_per_row);

	if (!julia)
	{
		write_mandelbrot(row_pointers, width, height, r, g, b);
	}
	else
	{
		write_julia(row_pointers, width, height, c_re, c_im, r, g, b);
	}

	write_png_file(fname, width, height, bytes_per_row, row_pointers);

	for (i = 0; i < height; i++)
	{
		free(row_pointers[i]);
	}
	free(row_pointers);

	return 0;
}
