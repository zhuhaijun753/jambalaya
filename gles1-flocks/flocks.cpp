/*
 * Copyright (C) 1999-2010  Terence M. Welsh
 *
 * This file is part of Flocks.
 *
 * Flocks is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * Flocks is distributed in the hope that it will be useful,
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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "rgbhsl.h"

#include "glues_project.h"
#include "glues_quad.h"

#define R2D 57.2957795131f

class bug;

// Globals
float frameTime = 0.0f;
float aspectRatio;
int wide;
int high;
int deep;
bug *lBugs;
bug *fBugs;
float colorFade;

int dLeaders;
int dFollowers;
int dSize;
int dComplexity;
int dSpeed;
int dStretch;
int dColorfadespeed;
int dChromatek;

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

class bug{
public:
	int type;  // 0 = leader   1 = follower
	float h, s, l;
	float r, g, b;
	float halfr, halfg, halfb;
	float x, y, z;
	float xSpeed, ySpeed, zSpeed, maxSpeed;
	float accel;
	int right, up, forward;
	int leader;
	float craziness;  // How prone to switching direction is this leader
	float nextChange;  // Time until this leader's next direction change

	bug();
	virtual ~bug();
	void initLeader();
	void initFollower();
	void update(bug *bugs);
};

bug::bug(){
}

bug::~bug(){
}

void bug::initLeader(){
	type = 0;
	h = rsRandf(1.0);
	s = 1.0f;
	l = 1.0f;
	x = rsRandf(float(wide * 2)) - float(wide);
	y = rsRandf(float(high * 2)) - float(high);
	z = rsRandf(float(wide * 2)) + float(wide * 2);
	right = up = forward = 1;
	xSpeed = ySpeed = zSpeed = 0.0f;
	maxSpeed = 8.0f * float(dSpeed);
	accel = 13.0f * float(dSpeed);
	craziness = rsRandf(4.0f) + 0.05f;
	nextChange = 1.0f;
}

void bug::initFollower(){
	type = 1;
	h = rsRandf(1.0);
	s = 1.0f;
	l = 1.0f;
	x = rsRandf(float(wide * 2)) - float(wide);
	y = rsRandf(float(high * 2)) - float(high);
	z = rsRandf(float(wide * 5)) + float(wide * 2);
	right = up = forward = 0;
	xSpeed = ySpeed = zSpeed = 0.0f;
	maxSpeed = (rsRandf(6.0f) + 4.0f) * float(dSpeed);
	accel = (rsRandf(4.0f) + 9.0f) * float(dSpeed);
	leader = 0;
}

void bug::update(bug *bugs){
	int i;
	float scale[4]={1.0f, 1.0f, 1.0f, 1.0f};

	if(!type){  // leader
		nextChange -= frameTime;
		if(nextChange <= 0.0f){
			if(rsRandi(2))
				right ++;
			if(rsRandi(2))
				up ++;
			if(rsRandi(2))
				forward ++;
			if(right >= 2)
				right = 0;
			if(up >= 2)
				up = 0;
			if(forward >= 2)
				forward = 0;
			nextChange = rsRandf(craziness);
		}
		if(right)
			xSpeed += accel * frameTime;
		else
			xSpeed -= accel * frameTime;
		if(up)
			ySpeed += accel * frameTime;
		else
			ySpeed -= accel * frameTime;
		if(forward)
			zSpeed -= accel * frameTime;
		else
			zSpeed += accel * frameTime;
		if(x < float(-wide))
			right = 1;
		if(x > float(wide))
			right = 0;
		if(y < float(-high))
			up = 1;
		if(y > float(high))
			up = 0;
		if(z < float(-deep))
			forward = 0;
		if(z > float(deep))
			forward = 1;
		// Even leaders change color from Chromatek 3D
		if(dChromatek){
			h = 0.666667f * ((float(wide) - z) / float(wide + wide));
			if(h > 0.666667f)
				h = 0.666667f;
			if(h < 0.0f)
				h = 0.0f;
		}
	}
	else{  // follower
		if(!rsRandi(10)){
			float oldDistance = 10000000.0f, newDistance;
			for(i=0; i<dLeaders; i++){
				newDistance = ((bugs[i].x - x) * (bugs[i].x - x)
							+ (bugs[i].y - y) * (bugs[i].y - y)
							+ (bugs[i].z - z) * (bugs[i].z - z));
				if(newDistance < oldDistance){
					oldDistance = newDistance;
					leader = i;
				}
			}
		}
		if((bugs[leader].x - x) > 0.0f)
			xSpeed += accel * frameTime;
		else
			xSpeed -= accel * frameTime;
		if((bugs[leader].y - y) > 0.0f)
			ySpeed += accel * frameTime;
		else
			ySpeed -= accel * frameTime;
		if((bugs[leader].z - z) > 0.0f)
			zSpeed += accel * frameTime;
		else
			zSpeed -= accel * frameTime;
		if(dChromatek){
			h = 0.666667f * ((float(wide) - z) / float(wide + wide));
			if(h > 0.666667f)
				h = 0.666667f;
			if(h < 0.0f)
				h = 0.0f;
		}
		else{
			if(fabs(h - bugs[leader].h) < (colorFade * frameTime))
				h = bugs[leader].h;
			else{
				if(fabs(h - bugs[leader].h) < 0.5f){
					if(h > bugs[leader].h)
						h -= colorFade * frameTime;
					else
						h += colorFade * frameTime;
				}
				else{
					if(h > bugs[leader].h)
						h += colorFade * frameTime;
					else
						h -= colorFade * frameTime;
					if(h > 1.0f)
						h -= 1.0f;
					if(h < 0.0f)
						h += 1.0f;
				}
			}
		}
	}

	if(xSpeed > maxSpeed)
		xSpeed = maxSpeed;
	if(xSpeed < -maxSpeed)
		xSpeed = -maxSpeed;
	if(ySpeed > maxSpeed)
		ySpeed = maxSpeed;
	if(ySpeed < -maxSpeed)
		ySpeed = -maxSpeed;
	if(zSpeed > maxSpeed)
		zSpeed = maxSpeed;
	if(zSpeed < -maxSpeed)
		zSpeed = -maxSpeed;

	x += xSpeed * frameTime;
	y += ySpeed * frameTime;
	z += zSpeed * frameTime;
	if(dStretch){
		scale[0] = xSpeed * 0.04f;
		scale[1] = ySpeed * 0.04f;
		scale[2] = zSpeed * 0.04f;
		scale[3] = scale[0] * scale[0] + scale[1] * scale[1] + scale[2] * scale[2];
		if(scale[3] > 0.0f){
			scale[3] = float(sqrt(scale[3]));
			scale[0] /= scale[3];
			scale[1] /= scale[3];
			scale[2] /= scale[3];
		}
	}
	hsl2rgb(h, s, l, r, g, b);
	halfr = r * 0.5f;
	halfg = g * 0.5f;
	halfb = b * 0.5f;
	glColor4f(r, g, b, 1.0f);
		glPushMatrix();
			glTranslatef(x, y, z);
			if(dStretch){
				scale[3] *= float(dStretch) * 0.05f;
				if(scale[3] < 1.0f)
					scale[3] = 1.0f;
				glRotatef(float(atan2(-scale[0], -scale[2])) * R2D, 0.0f, 1.0f, 0.0f);
				glRotatef(float(asin(scale[1])) * R2D, 1.0f, 0.0f, 0.0f);
				glScalef(1.0f, 1.0f, scale[3]);
			}
			gluSphere(qobj, float(dSize) * 0.5f, dComplexity + 2, dComplexity + 1);
		glPopMatrix();
}


extern "C" void app_draw()
{
    int i;

    static rsTimer timer;
    frameTime = timer.tick();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -float(wide * 2));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update and draw leaders
    for(i=0; i<dLeaders; i++)
    {
        lBugs[i].update(lBugs);
    }
    // Update and draw followers
    for(i=0; i<dFollowers; i++)
    {
        fBugs[i].update(lBugs);
    }

    glFinish();
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    aspectRatio = float(width) / float(height);
    gluPerspective(50.0, aspectRatio, 0.1, 2000.0);
    glMatrixMode(GL_MODELVIEW);
	
    // calculate boundaries
    if(aspectRatio >= 1.0f)
    {
        high = deep = 160;
        wide = int(float(high) * aspectRatio);
    }
    else
    {
        wide = deep = 160;
        high = int(float(wide) / aspectRatio);
    }
}


extern "C" void app_init(int width, int height)
{
    int i;

    dLeaders = 4;
    dFollowers = 440;
    dSize = 8;
    dComplexity = 1;
    dSpeed = 10;
    dStretch = 20;
    dColorfadespeed = 15;
    dChromatek = 0;

    qobj = gluNewQuadric();

    reshape(width, height);

    x_srand((unsigned)time(NULL));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float ambient[4] = {0.25f, 0.25f, 0.25f, 0.0f};
    float diffuse[4] = {1.0f, 1.0f, 1.0f, 0.0f};
    float specular[4] = {1.0f, 1.0f, 1.0f, 0.0f};
    float position[4] = {500.0f, 500.0f, 500.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glEnable(GL_COLOR_MATERIAL);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);
    glColor4f(0.7f, 0.7f, 0.7f, 1.0f);

    lBugs = new bug[dLeaders];
    fBugs = new bug[dFollowers];
    for(i=0; i<dLeaders; i++)
    {
        lBugs[i].initLeader();
    }
    for(i=0; i<dFollowers; i++)
    {
        fBugs[i].initFollower();
    }

    colorFade = float(dColorfadespeed) * 0.01f;
}


void cleanUp()
{
    // Free memory
    gluDeleteQuadric(qobj);
    delete[] lBugs;
    delete[] fBugs;
}
