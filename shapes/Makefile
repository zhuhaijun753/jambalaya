CC = gcc -Wall

all:
	$(CC) shapes.c main.c glues_quad.c glues_project.c glut_shapes.c -o shapes -I/usr/include -I/usr/include/GLES -L/usr/lib -lGLUES_CM -lGLES_CM -lgf

clean:
	@echo Cleaning up...
	@rm shapes
	@echo Done.
