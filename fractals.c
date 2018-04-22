/**
 *  Easy Factal Generator
 *  Copyright (C) 2018  Giuliano Sovernigo
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include "fractals.h"

struct colour make_colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	struct colour new_colour;
	new_colour.r = r;
	new_colour.g = g;
	new_colour.b = b;
	new_colour.a = a;

	return new_colour;
}

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

double write_progress(int max, int done)
{
	double percent;

	percent = ((double) done) / ((double) max);
	printf("[%3.2lf%%]\n", percent * 100.00);
	return percent;
}

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

void construct_mandelbrot(png_bytep* row_pointers, int width, int height, uint8_t r, uint8_t g, uint8_t b, int intensity)
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

void construct_julia(png_bytep* row_pointers, int width, int height, double c_re, double c_im, uint8_t r, uint8_t g, uint8_t b, int intensity)
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
