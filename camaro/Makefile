CC = gcc -Wall

all:
	$(CC) camaro.c glues_project.c main.c -o camaro -I/usr/include -I/usr/include/GLES -L/usr/lib -lGLES_CM -lgf

clean:
	@echo Cleaning up...
	@rm -f camaro
	@echo Done.
