*******************************************************************************

	Author: 	Giuliano Sovernigo
	Email: 		juliansovernigo@gmail.com

	This program is a fractal generator!  It uses libpng to write a fractal
	pattern to a png file.  You can specify colour, type, complex roots, and
	template file!  It is not complete yet, and there are some features on the
	way, such as three-colour-fading that will be neat.

*******************************************************************************

Included in the folder should be the following files:
	- fractals.c
	- Makefile
	- pngwriter
	- README.txt
	- saved_images
	- temp1920x1080.png
	- temp2436x1125.png
	- temp3840x1440.png
	- temp3840x2160.png

The included png files are common high-res templates that you can use to create
your own fractals.  Note that the program needs a pre-existing png file to use
as a template to write to, and it will not produce stand-alone files... I am
working on that one.

***********
 Compiling
***********

1. type make.
2. obsever the new file that has been created.

***********
 Execution
***********

The minimum flag that needs to be given to invoke the program is the `-f` flag.
This flag specifies which file is to be used as the png template and the output
file.  If the program does not have this flag, it will complain and fail to do
anything really.

FLAGS:
	-r [0..255]			This flag specifies the red component.  Default 255.
	-g [0..255]			This flag specifies the green component.  Default 255.
	-b [0..255]			This flag specifies the blue component.  Default 255.
	-intensity [10..inf)Specifies the colour intensity.  Default 10.
	-julia				Specifies a Julia fractal output.  Without, it produces
						a Mandelbrot Set.
	-real [-2..2]		Specify the real component of the fractal (julia only)
	-imaginary [-2..2]	Specify the imaginary component (julia only)

