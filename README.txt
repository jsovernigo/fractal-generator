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
	- README.txt

The program will output the file to the file name provided, using the -o flag.

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
	-w [0..inf)			This flag specifies the width of the image.
	-h [0..inf)			This flag specifies the height of the image.
	-o <filename>		Mandatory flag, required to output the fractal to a file.
	--intensity [10..inf)Specifies the colour intensity.  Default 10.
	--julia				Specifies a Julia fractal output.  Without, it produces
						a Mandelbrot Set.
	--real [-2..2]		Specify the real component of the fractal (julia only)
	--imaginary [-2..2]	Specify the imaginary component (julia only)

