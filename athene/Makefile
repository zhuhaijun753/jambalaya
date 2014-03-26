CC = gcc -Wall

all:
	$(CC) athene.c glues_project.c main.c -o athene -I/usr/include -I/usr/include/GLES -L/usr/lib -lGLES_CM -lgf

clean:
	@echo Cleaning up...
	@rm -f athene
	@echo Done.
