/*
 * Copyright (C) 1999-2010  Terence M. Welsh
 *
 * This file is part of Cyclone.
 *
 * Cyclone is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * Cyclone is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * OpenGL ES 1.0 CM port by Mike Gorchak
 */

// Cyclone screen saver

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "rgbhsl.h"

#include "glues_project.h"
#include "glues_quad.h"

class cyclone;
class particle;

#define PIx2 6.28318530718f
#define PI 3.14159265359f
#define wide 200
#define high 200

// Parameters edited in the dialog box
int dCyclones;
int dParticles;
int dSize;
int dComplexity;
int dSpeed;
int dStretch;
// Other globals
float aspectRatio;
cyclone **cyclones;
particle **particles;
float fact[13];
float frameTime = 0.0f;

GLUquadricObj *qobj;

unsigned int x_seed;

unsigned int x_rand()
{
    return x_seed = (x_seed * 1103515245 + 12345);
}

void x_srand(unsigned int val)
{
    x_seed=val;
}

// Useful random number macros
// Don't forget to initialize with srand()
inline int rsRandi(int x){
	return x_rand() % x;
}
inline float rsRandf(float x){
	return x * ((float)x_rand() / (float)4294967295);
}

class rsTimer{
public:
	// Time since last call to tick()
	float elapsed_time;
	// Time waited so far by wait()
	float waited_time;
	// Wait() would take a hair too long, if we didn't keep track of the
	// extra time it was spending.  That's what wait_overhead is for.
	float wait_function_overhead;
	struct timeval prev_tv;

	rsTimer(){
		elapsed_time = 0.0f;
		waited_time = 0.0f;
		wait_function_overhead = 0.0f;
		gettimeofday(&prev_tv, NULL);
	}

	~rsTimer(){}

	// return time elapsed since last call to tick()
	inline float tick(){
		struct timeval curr_tv;
		gettimeofday(&curr_tv, NULL);
		float elapsed_time = (curr_tv.tv_sec - prev_tv.tv_sec)
			+ ((curr_tv.tv_usec - prev_tv.tv_usec) * 0.000001f);
		prev_tv = curr_tv;
		return elapsed_time;
	}

	// Wait until target_time has elapsed, then return.
	// If you call this function after target_time has already elapsed,
	// it will return immediately.  Think of target_time as a lower limit
	// on frame time.
	// Returns actual time elapsed since last call to wait().
	inline float wait(float target_time){
		waited_time = tick();
		float actual_waited_time(waited_time + wait_function_overhead);
		if(actual_waited_time < target_time){
			usleep(long(1000000.0f * (target_time - actual_waited_time)));
			waited_time += tick();
		}
		wait_function_overhead += 0.05f * (waited_time - target_time);
		return waited_time;
	}
};

// useful vector functions
float length(float *xyz){
	return(float(sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2])));
}
float normalize(float *xyz){
	float length = float(sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]));
	if(length == 0.0)
		return(0.0);
	xyz[0] /= length;
	xyz[1] /= length;
	xyz[2] /= length;
	return(length);
}
float dot(float *xyz1, float *xyz2){
	return(xyz1[0] * xyz2[0] + xyz1[1] * xyz2[1] + xyz1[2] * xyz2[2]);
}
void cross(float *xyz1, float *xyz2, float *xyzOut){
	xyzOut[0] = xyz1[1] * xyz2[2] - xyz2[1] * xyz1[2];
	xyzOut[1] = xyz1[2] * xyz2[0] - xyz2[2] * xyz1[0];
	xyzOut[2] = xyz1[0] * xyz2[1] - xyz2[0] * xyz1[1];
}


// useful factorial function
int factorial(int x){
    int returnval = 1;

    if(x == 0)
        return(1);
    else{
        do{
            returnval *= x;
            x -= 1;
        }
        while(x!=0);
    }
    return(returnval);
}


class cyclone{
public:
	float **targetxyz;
	float **xyz;
	float **oldxyz;
	float *targetWidth;
	float *width;
	float *oldWidth;
	float targethsl[3];
	float hsl[3];
	float oldhsl[3];
	float **xyzChange;
	float **widthChange;
	float hslChange[2];

	cyclone();
	virtual ~cyclone(){};
	void update();
};

cyclone::cyclone(){
	int i;

	// Initialize position stuff
	targetxyz = new float*[dComplexity+3];
	xyz = new float*[dComplexity+3];
	oldxyz = new float*[dComplexity+3];
	for(i=0; i<int(dComplexity)+3; i++){
		targetxyz[i] = new float[3];
		xyz[i] = new float[3];
		oldxyz[i] = new float[3];
	}
	xyz[dComplexity+2][0] = rsRandf(float(wide*2)) - float(wide);
	xyz[dComplexity+2][1] = float(high);
	xyz[dComplexity+2][2] = rsRandf(float(wide*2)) - float(wide);
	xyz[dComplexity+1][0] = xyz[dComplexity+2][0];
	xyz[dComplexity+1][1] = rsRandf(float(high / 3)) + float(high / 4);
	xyz[dComplexity+1][2] = xyz[dComplexity+2][2];
	for(i=dComplexity; i>1; i--){
		xyz[i][0] = xyz[i+1][0] + rsRandf(float(wide)) - float(wide / 2);
		xyz[i][1] = rsRandf(float(high * 2)) - float(high);
		xyz[i][2] = xyz[i+1][2] + rsRandf(float(wide)) - float(wide / 2);
	}
	xyz[1][0] = xyz[2][0] + rsRandf(float(wide / 2)) - float(wide / 4);
	xyz[1][1] = -rsRandf(float(high / 2)) - float(high / 4);
	xyz[1][2] = xyz[2][2] + rsRandf(float(wide / 2)) - float(wide / 4);
	xyz[0][0] = xyz[1][0] + rsRandf(float(wide / 8)) - float(wide / 16);
	xyz[0][1] = float(-high);
	xyz[0][2] = xyz[1][2] + rsRandf(float(wide / 8)) - float(wide / 16);
	// Initialize width stuff
	targetWidth = new float[dComplexity+3];
	width = new float[dComplexity+3];
	oldWidth = new float[dComplexity+3];
	width[dComplexity+2] = rsRandf(175.0f) + 75.0f;
	width[dComplexity+1] = rsRandf(60.0f) + 15.0f;
	for(i=dComplexity; i>1; i--)
		width[i] = rsRandf(25.0f) + 15.0f;
	width[1] = rsRandf(25.0f) + 5.0f;
	width[0] = rsRandf(15.0f) + 5.0f;
	// Initialize transition stuff
	xyzChange = new float*[dComplexity + 3];
	widthChange = new float*[dComplexity + 3];
	for(i=0; i<(dComplexity+3); i++){
		xyzChange[i] = new float[2];	// 0 = step   1 = total steps
		widthChange[i] = new float[2];
		xyzChange[i][0] = 0.0f;
		xyzChange[i][1] = 0.0f;
		widthChange[i][0] = 0.0f;
		widthChange[i][1] = 0.0f;
	}
	// Initialize color stuff
	hsl[0] = oldhsl[0] = rsRandf(1.0f);
	hsl[1] = oldhsl[1] = rsRandf(1.0f);
	hsl[2] = oldhsl[2] = 0.0f;  // start out dark
	targethsl[0] = rsRandf(1.0f);
	targethsl[1] = rsRandf(1.0f);
	targethsl[2] = 1.0f;
	hslChange[0] = 0.0f;
	hslChange[1] = 10.0f;
}

void cyclone::update(){
	int i;
	int temp;
	float between;
	float diff;
	int direction;

	// update cyclone's path
	temp = dComplexity + 2;
	if(xyzChange[temp][0] >= xyzChange[temp][1]){
		oldxyz[temp][0] = xyz[temp][0];
		oldxyz[temp][1] = xyz[temp][1];
		oldxyz[temp][2] = xyz[temp][2];
		targetxyz[temp][0] = rsRandf(float(wide*2)) - float(wide);
		targetxyz[temp][1] = float(high);
		targetxyz[temp][2] = rsRandf(float(wide*2)) - float(wide);
		xyzChange[temp][0] = 0.0f;
		xyzChange[temp][1] = rsRandf(150.0f / float(dSpeed)) + 75.0f / float(dSpeed);
	}
	temp = dComplexity + 1;
	if(xyzChange[temp][0] >= xyzChange[temp][1]){
		oldxyz[temp][0] = xyz[temp][0];
		oldxyz[temp][1] = xyz[temp][1];
		oldxyz[temp][2] = xyz[temp][2];
		targetxyz[temp][0] = xyz[temp+1][0];
		targetxyz[temp][1] = rsRandf(float(high / 3)) + float(high / 4);
		targetxyz[temp][2] = xyz[temp+1][2];
		xyzChange[temp][0] = 0.0f;
		xyzChange[temp][1] = rsRandf(100.0f / float(dSpeed)) + 75.0f / float(dSpeed);
	}
	for(i=dComplexity; i>1; i--){
		if(xyzChange[i][0] >= xyzChange[i][1]){
			oldxyz[i][0] = xyz[i][0];
			oldxyz[i][1] = xyz[i][1];
			oldxyz[i][2] = xyz[i][2];
			targetxyz[i][0] = targetxyz[i+1][0] + (targetxyz[i+1][0] - targetxyz[i+2][0]) / 2.0f + rsRandf(float(wide / 2)) - float(wide / 4);
			targetxyz[i][1] = (targetxyz[i+1][1] + targetxyz[i-1][1]) / 2.0f + rsRandf(float(high / 8)) - float(high / 16);
			targetxyz[i][2] = targetxyz[i+1][2] + (targetxyz[i+1][2] - targetxyz[i+2][2]) / 2.0f + rsRandf(float(wide / 2)) - float(wide / 4);
			if(targetxyz[i][1] > high)
				targetxyz[i][1] = high;
			if(targetxyz[i][1] < -high)
				targetxyz[i][1] = -high;
			xyzChange[i][0] = 0.0f;
			xyzChange[i][1] = rsRandf(75.0f / float(dSpeed)) + 50.0f / float(dSpeed);
		}
	}
	if(xyzChange[1][0] >= xyzChange[1][1]){
		oldxyz[1][0] = xyz[1][0];
		oldxyz[1][1] = xyz[1][1];
		oldxyz[1][2] = xyz[1][2];
		targetxyz[1][0] = targetxyz[2][0] + rsRandf(float(wide / 2)) - float(wide / 4);
		targetxyz[1][1] = -rsRandf(float(high / 2)) - float(high / 4);
		targetxyz[1][2] = targetxyz[2][2] + rsRandf(float(wide / 2)) - float(wide / 4);
		xyzChange[1][0] = 0.0f;
		xyzChange[1][1] = rsRandf(50.0f / float(dSpeed)) + 30.0f / float(dSpeed);
	}
	if(xyzChange[0][0] >= xyzChange[0][1]){
		oldxyz[0][0] = xyz[0][0];
		oldxyz[0][1] = xyz[0][1];
		oldxyz[0][2] = xyz[0][2];
		targetxyz[0][0] = xyz[1][0] + rsRandf(float(wide / 8)) - float(wide / 16);
		targetxyz[0][1] = float(-high);
		targetxyz[0][2] = xyz[1][2] + rsRandf(float(wide / 8)) - float(wide / 16);
		xyzChange[0][0] = 0.0f;
		xyzChange[0][1] = rsRandf(100.0f / float(dSpeed)) + 75.0f / float(dSpeed);
	}
	for(i=0; i<(dComplexity+3); i++){
		between = xyzChange[i][0] / xyzChange[i][1] * PIx2;
		between = (1.0f - float(cos(between))) / 2.0f; 
		xyz[i][0] = ((targetxyz[i][0] - oldxyz[i][0]) * between) + oldxyz[i][0];
		xyz[i][1] = ((targetxyz[i][1] - oldxyz[i][1]) * between) + oldxyz[i][1];
		xyz[i][2] = ((targetxyz[i][2] - oldxyz[i][2]) * between) + oldxyz[i][2];
		xyzChange[i][0] += frameTime;
	}

	// Update cyclone's widths
	temp = dComplexity + 2;
	if(widthChange[temp][0] >= widthChange[temp][1]){
		oldWidth[temp] = width[temp];
		targetWidth[temp] = rsRandf(225.0f) + 75.0f;
		widthChange[temp][0] = 0.0f;
		widthChange[temp][1] = rsRandf(50.0f / float(dSpeed)) + 50.0f / float(dSpeed);
	}
	temp = dComplexity + 1;
	if(widthChange[temp][0] >= widthChange[temp][1]){
		oldWidth[temp] = width[temp];
		targetWidth[temp] = rsRandf(100.0f) + 15.0f;
		widthChange[temp][0] = 0.0f;
		widthChange[temp][1] = rsRandf(50.0f / float(dSpeed)) + 50.0f / float(dSpeed);
	}
	for(i=dComplexity; i>1; i--){
		if(widthChange[i][0] >= widthChange[i][1]){
			oldWidth[i] = width[i];
			targetWidth[i] = rsRandf(50.0f) + 15.0f;
			widthChange[i][0] = 0.0f;
			widthChange[i][1] = rsRandf(50.0f / float(dSpeed)) + 40.0f / float(dSpeed);
		}
	}
	if(widthChange[1][0] >= widthChange[1][1]){
		oldWidth[1] = width[1];
		targetWidth[1] = rsRandf(40.0f) + 5.0f;
		widthChange[1][0] = 0.0f;
		widthChange[1][1] = rsRandf(50.0f / float(dSpeed)) + 30.0f / float(dSpeed);
	}
	if(widthChange[0][0] >= widthChange[0][1]){
		oldWidth[0] = width[0];
		targetWidth[0] = rsRandf(30.0f) + 5.0f;
		widthChange[0][0] = 0.0f;
		widthChange[0][1] = rsRandf(50.0f / float(dSpeed)) + 20.0f / float(dSpeed);
	}
	for(i=0; i<(dComplexity+3); i++){
		between = widthChange[i][0] / widthChange[i][1];
		width[i] = ((targetWidth[i] - oldWidth[i]) * between) + oldWidth[i];
		widthChange[i][0] += frameTime;
	}

	// Update cyclones color
	if(hslChange[0] >= hslChange[1]){
		oldhsl[0] = hsl[0];
		oldhsl[1] = hsl[1];
		oldhsl[2] = hsl[2];
		targethsl[0] = rsRandf(1.0f);
		targethsl[1] = rsRandf(1.0f);
		targethsl[2] = rsRandf(1.0f) + 0.5f;
		if(targethsl[2] > 1.0f)
			targethsl[2] = 1.0f;
		hslChange[0] = 0.0f;
		hslChange[1] = rsRandf(30.0f) + 2.0f;
	}
	between = hslChange[0] / hslChange[1];
	diff = targethsl[0] - oldhsl[0];
	direction = 0;
	if((targethsl[0] > oldhsl[0] && diff > 0.5f) || (targethsl[0] < oldhsl[0] && diff < -0.5f))
		if(diff > 0.5f)
			direction = 1;
	hslTween(oldhsl[0], oldhsl[1], oldhsl[2],
			targethsl[0], targethsl[1], targethsl[2], between, direction,
			hsl[0], hsl[1], hsl[2]);
	hslChange[0] += frameTime;
}


class particle{
public:
	float r, g, b;
	float xyz[3], lastxyz[3];
	float width;
	float step;
	float spinAngle;
	cyclone *cy;

	particle(cyclone *);
	virtual ~particle(){};
	void init();
	void update();
};

particle::particle(cyclone *c){
	cy = c;
	init();
}

void particle::init(){
	width = rsRandf(0.8f) + 0.2f;
	step = 0.0f;
	spinAngle = rsRandf(360);
	hsl2rgb(cy->hsl[0], cy->hsl[1], cy->hsl[2], r, g, b);
}

void particle::update(){
	int i;
	float scale=1.0f, temp;
	float newStep;
	float newSpinAngle;
	float cyWidth;
	float between;
	float dir[3];
	float crossVec[3];
	float tiltAngle;
	float up[3] = {0.0f, 1.0f, 0.0f};
	float blend;

	lastxyz[0] = xyz[0];
	lastxyz[1] = xyz[1];
	lastxyz[2] = xyz[2];
	if(step > 1.0f)
		init();
	xyz[0] = xyz[1] = xyz[2] = 0.0f;
	for(i=0; i<(dComplexity+3); i++){
		blend = fact[dComplexity+2] / (fact[i]
			* fact[dComplexity+2-i]) * powf(step, float(i))
			* powf((1.0f - step), float(dComplexity+2-i));
		xyz[0] += cy->xyz[i][0] * blend;
		xyz[1] += cy->xyz[i][1] * blend;
		xyz[2] += cy->xyz[i][2] * blend;
	}
	dir[0] = dir[1] = dir[2] = 0.0f;
	for(i=0; i<(dComplexity+3); i++){
		blend = fact[dComplexity+2] / (fact[i]
			* fact[dComplexity+2-i]) * powf(step - 0.01f, float(i))
			* powf((1.0f - (step - 0.01f)), float(dComplexity+2-i));
		dir[0] += cy->xyz[i][0] * blend;
		dir[1] += cy->xyz[i][1] * blend;
		dir[2] += cy->xyz[i][2] * blend;
	}
	dir[0] = xyz[0] - dir[0];
	dir[1] = xyz[1] - dir[1];
	dir[2] = xyz[2] - dir[2];
	normalize(dir);
	cross(dir, up, crossVec);
	tiltAngle = -acosf(dot(dir, up)) * 180.0f / PI;
	i = int(step * (float(dComplexity) + 2.0f));
	if(i >= (dComplexity + 2))
		i = dComplexity + 1;
	between = (step - (float(i) / float(dComplexity + 2))) * float(dComplexity + 2);
	cyWidth = cy->width[i] * (1.0f - between) + cy->width[i+1] * (between);
	newStep = (0.2f * frameTime * float(dSpeed)) / (width * width * cyWidth);
	step += newStep;
	newSpinAngle = (1500.0f * frameTime * float(dSpeed)) / (width * cyWidth);
	spinAngle += newSpinAngle;
	if(dStretch){
		scale = width * cyWidth * newSpinAngle * 0.02f;
		temp = cyWidth * 2.0f / float(dSize);
		if(scale > temp)
			scale = temp;
		if(scale < 3.0f)
			scale = 3.0f;
	}
	glColor4f(r, g, b, 1.0f);
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(xyz[0], xyz[1], xyz[2]);
		glRotatef(tiltAngle, crossVec[0], crossVec[1], crossVec[2]);
		glRotatef(spinAngle, 0, 1, 0);
		glTranslatef(width * cyWidth, 0, 0);
		if(dStretch)
			glScalef(1.0f, 1.0f, scale);
		gluSphere(qobj, float(dSize) / 4.0f, 3, 2);
	glPopMatrix();
}


extern "C" void app_draw()
{
    int i, j;

    static rsTimer timer;
    frameTime = timer.tick();

    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(i=0; i<dCyclones; i++)
    {
        cyclones[i]->update();
        for(j=(i*dParticles); j<((i+1)*dParticles); j++)
        {
            particles[j]->update();
        }
    }
}


extern "C" void app_init(int width, int height)
{
	int i, j;

	dCyclones = 1;
	dParticles = 400;
	dSize = 7;
	dComplexity = 3;
	dSpeed = 10;
	dStretch = 1;

	x_srand((unsigned)time(NULL));

        qobj = gluNewQuadric();

        glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	aspectRatio = float(width) / float(height);
	gluPerspective(80.0, aspectRatio, 50, 3000);

	if(!rsRandi(500))
        {
            // Easter egg view
            glRotatef(90, 1, 0, 0);
            glTranslatef(0.0f, -(wide * 2), 0.0f);
	}
	else
        {
            // Normal view
	    glTranslatef(0.0f, 0.0f, -(wide * 2));
        }
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float ambient[4] = {0.25f, 0.25f, 0.25f, 0.0f};
	float diffuse[4] = {1.0f, 1.0f, 1.0f, 0.0f};
	float specular[4] = {1.0f, 1.0f, 1.0f, 0.0f};
	float position[4] = {float(wide * 2), -float(high), float(wide * 2), 0.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_COLOR_MATERIAL);
	glMaterialf(GL_FRONT, GL_SHININESS, 20.0f);
	glColor4f(0.7f, 0.7f, 0.7f, 1.0f);

	// Initialize cyclones and their particles
	for(i=0; i<13; i++)
		fact[i] = float(factorial(i));
	cyclones = new cyclone*[dCyclones];
	particles = new particle*[dParticles * dCyclones];
	for(i=0; i<dCyclones; i++){
		cyclones[i] = new cyclone;
		for(j=i*dParticles; j<((i+1)*dParticles); j++)
			particles[j] = new particle(cyclones[i]);
	}
}


void cleanUp()
{
    // Free memory
    gluDeleteQuadric(qobj);
    delete[] particles;
    delete[] cyclones;
}
