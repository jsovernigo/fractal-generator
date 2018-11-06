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

#ifndef __LIBFRACTAL__
#define __LIBFRACTAL__

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

struct thread_context {
	int width;
	int height;
	int i;
	int j;
	int r;
	int g;
	int b;
	int intensity;
	png_byte* ptr;
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
struct colour make_colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

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
struct colour fade(struct colour from, struct colour to, double percentage);

/**
 *	write_progress
 *	writes the progress of the iterations to the screen.
 *	todo this is a target for an upgrade to an in-place bar later.
 *	IN:			max			int - the "one hundred"
 *				done		done - the current amount of max done.
 *	OUT:		the double percentage of progress.
 *	POST:		percentage and newline written to the screen.
 *	ERROR:		done > max.
 */
double write_progress(int max, int done);

/**
 *	get_rowpointers
 *	mallocs and returns a set of row pointers, based on the width and height passed in.
 *	IN:			width		int - the integer width of the image that you want to construct.
 *				height		int - the height of the image, in integer form.
 *	OUT:		an allocated array of byte pointers that you can write to.
 *	POST:		malloc'd memory returned.
 *	ERROR:		width/height negative.
 */
png_bytep* get_rowpointers(int width, int height);

/**
 *	write_png_file
 *	writes the png information in row_pointers to the defined file.
 *	IN:		fname			char* - the file output name.
 *			width			int - the width of the image
 *			height			int - the height of the image.
 *
 */
int write_png_file(char* fname, int width, int height, png_bytep* row_pointers);

/**
 *	TODO
 */
void* mandelbrot_thread_handler(void* args);

/**
 *	construct_mandelbrot
 *	builds a mandelbrot set in the row pointers, colouring using the recursive depth model.
 *	IN:		row_pointers	png_bytep* - the list of png image row pointers.
 *			width			int - the width of the image.
 *			height			int - the height of the image.
 *			r				uint8_t - the red component
 *			g				uint8_t - the green component
 *			b				uint8_t - the blue component
 *			intensity       int - The exponent used to modify brightness of each pixel
 *	OUT:	nothing returned.
 *	POST:	a mandelbrot set has been written to the row_pointers that you provided.
 *	ERROR:	row_pointers is null.
 */
void construct_mandelbrot(png_bytep* row_pointers, int width, int height, uint8_t r, uint8_t g, uint8_t b, int intensity);


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
 *			intensity       int - The exponent used to modify brightness of each pixel
 *	OUT:	nothing returned.
 *	POST:	a julia set has been written to the row_pointers provided.
 *	ERROR:	row_pointers is null.
 */
void construct_julia(png_bytep* row_pointers, int width, int height, double c_re, double c_im, uint8_t r, uint8_t g, uint8_t b, int intensity);

#endif
