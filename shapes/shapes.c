#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GLES/gl.h>
#include "glues_project.h"
#include "glut_shapes.h"

GLfloat mat_ambient[4]={0.2f, 0.2f, 0.2f, 0.2f};
GLfloat mat_specular[4]={1.0f, 1.0f, 1.0f, 1.0f};
GLfloat mat_shininess[1]={100.0f};

GLfloat mat_red_diffuse[4]={0.7f, 0.0f, 0.1f, 1.0f};
GLfloat mat_green_diffuse[4]={0.0f, 0.7f, 0.1f, 1.0f};
GLfloat mat_blue_diffuse[4]={0.0f, 0.1f, 0.7f, 1.0f};
GLfloat mat_yellow_diffuse[4]={0.7f, 0.8f, 0.1f, 1.0f};
GLfloat mat_cyan_diffuse[4]={0.1f, 0.8f, 0.7f, 1.0f};
GLfloat mat_magenta_diffuse[4]={0.7f, 0.0f, 0.7f, 1.0f};
GLfloat mat_x_diffuse[4]={0.4f, 0.6f, 0.8f, 1.0f};
GLfloat mat_x2_diffuse[4]={0.8f, 0.6f, 0.4f, 1.0f};

GLfloat xrot=0.0f;
GLfloat yrot=0.0f;
GLfloat zrot=0.0f;

/* function to reset our viewport after a window resize */
void app_init(int width, int height)
{
    /* Height / width ration */
    GLfloat ratio;

    /* Enable Texture Mapping */
    glEnable(GL_TEXTURE_2D);

    /* Enable smooth shading */
    glShadeModel(GL_SMOOTH);

    /* Set the background black */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /* Depth buffer setup */
    glClearDepthf(1.0f);

    /* Enables Depth Testing */
    glEnable(GL_DEPTH_TEST);

    /* The Type Of Depth Test To Do */
    glDepthFunc(GL_LEQUAL);

    /* Enable lighting */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    /* Set material parameters */
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);

    /* Protect against a divide by zero */
    if (height==0)
    {
        height=1;
    }

    ratio=(GLfloat)width/(GLfloat)height;

    /* Setup our viewport. */
    glViewport(0, 0, (GLint)width, (GLint)height);

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* Set our perspective */
    gluPerspective(55.0f, (GLfloat)width/(GLfloat)height, 2.0f, 24.0f);

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode(GL_MODELVIEW);

    /* Reset The View */
    glLoadIdentity();
}

/* Here goes our drawing code */
void app_draw()
{
    GLenum error;

    /* Clear The Screen And Depth Buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Reset The Current Matrix */
    glLoadIdentity();
    glTranslatef(-8.0f, 6.0f, -15.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_red_diffuse);
    glutSolidSphere(1.5f, 40.0f, 40.0f);

    glLoadIdentity();
    glTranslatef(-3.0f, 6.0f, -15.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_red_diffuse);
    glutWireSphere(1.5f, 40.0f, 40.0f);

    glLoadIdentity();
    glTranslatef(2.0f, 6.0f, -15.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_blue_diffuse);
    glutSolidCone(1.5f, 2.0f, 40.0f, 40.0f);

    glLoadIdentity();
    glTranslatef(7.0f, 6.0f, -15.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_blue_diffuse);
    glutWireCone(1.5f, 2.0f, 40.0f, 40.0f);

    glLoadIdentity();
    glTranslatef(-8.0f, 2.0f, -15.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_green_diffuse);
    glutSolidCube(2.0f);

    glLoadIdentity();
    glTranslatef(-3.0f, 2.0f, -15.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_green_diffuse);
    glutWireCube(2.0f);

    glLoadIdentity();
    glTranslatef(2.0f, 2.0f, -15.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_yellow_diffuse);
    glutSolidTorus(0.5f, 1.0f, 40.0f, 40.0f);

    glLoadIdentity();
    glTranslatef(7.0f, 2.0f, -15.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_yellow_diffuse);
    glutWireTorus(0.5f, 1.0f, 40.0f, 40.0f);

    glLoadIdentity();
    glTranslatef(-8.0f, -2.0f, -15.0f);
    glScalef(0.7f, 0.7f, 0.7f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_cyan_diffuse);
    glutSolidDodecahedron();

    glLoadIdentity();
    glTranslatef(-3.0f, -2.0f, -15.0f);
    glScalef(0.7f, 0.7f, 0.7f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_cyan_diffuse);
    glutWireDodecahedron();

    glLoadIdentity();
    glTranslatef(2.0f, -2.0f, -15.0f);
    glScalef(1.5f, 1.5f, 1.5f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_magenta_diffuse);
    glutSolidOctahedron();

    glLoadIdentity();
    glTranslatef(7.0f, -2.0f, -15.0f);
    glScalef(1.5f, 1.5f, 1.5f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_magenta_diffuse);
    glutWireOctahedron();

    glLoadIdentity();
    glTranslatef(-8.0f, -6.0f, -15.0f);
    glScalef(1.2f, 1.2f, 1.2f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_x_diffuse);
    glutSolidIcosahedron();

    glLoadIdentity();
    glTranslatef(-3.0f, -6.0f, -15.0f);
    glScalef(1.2f, 1.2f, 1.2f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_x_diffuse);
    glutWireIcosahedron();

    glLoadIdentity();
    glTranslatef(2.0f, -6.0f, -15.0f);
    glScalef(1.5f, 1.5f, 1.5f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_x2_diffuse);
    glutSolidTetrahedron();

    glLoadIdentity();
    glTranslatef(7.0f, -6.0f, -15.0f);
    glScalef(1.5f, 1.5f, 1.5f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_x2_diffuse);
    glutWireTetrahedron();

    xrot+=0.4;
    yrot+=0.5;
    zrot+=0.6;

    /* Check if one of the calls was failed */
    error=glGetError();
    if (error!=GL_NO_ERROR)
    {
       printf("OpenGL ES error: %08X\n", error);
    }
}

void app_printhelp()
{
}

void app_options(int argc, char** argv)
{
}

void app_fini()
{
}
