// Name: Matthew Jimenez
// Quarter, Year: Winter 2020
// Lab: 7
//
// This file is to be modified by the student.
// main.cpp
////////////////////////////////////////////////////////////
#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <vector>
#include <cstdio>
#include <math.h>
#include "vec.h"
#include <iostream>

using namespace std;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
vector<vec2> controlPoints;

float factorial(int n) {
	int sum = 1;
	for (int i = n; i > 0; i--) {
		sum *= i;
	}
	return sum;
}

float combination(int n, int k) {
	float combo = factorial(n) / (factorial(n - k) * factorial(k));
	return combo;
}

float binomial(int n, int k, float t) {
	float bin = 0;
	bin = combination(n, k)*pow(t, k)*pow(1 - t, n - k);
	return bin;
}

void GL_render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();

    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f,0.0f,0.0f);
    vec2 actual;
    float sumX = 0.0;
    float sumY = 0.0;
    //update
    for(float t = 0.0; t < 1.0; t += .01){
        sumX = 0.0;
        sumY = 0.0;
        for(unsigned int i = 0; i < controlPoints.size();i++){
            double px = binomial(controlPoints.size(),i,t) * controlPoints[i][0];
            double py = binomial(controlPoints.size(),i,t) * controlPoints[i][1];
            sumX += px;
            sumY += py;
            actual = vec2(sumX,sumY);
        }
        glVertex2f(actual[0],actual[1]);
    }
    glEnd();
    glFlush();
}

void GL_mouse(int button,int state,int x,int y)
{
    y=WINDOW_HEIGHT-y;
    GLdouble mv_mat[16];
    GLdouble proj_mat[16];
    GLint vp_mat[4];
    glGetDoublev(GL_MODELVIEW_MATRIX,mv_mat);
    glGetDoublev(GL_PROJECTION_MATRIX,proj_mat);
    glGetIntegerv(GL_VIEWPORT,vp_mat);

    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
        double px,py,dummy_z; // we don't care about the z-value but need something to pass in
        gluUnProject(x,y,0,mv_mat,proj_mat,vp_mat,&px,&py,&dummy_z);
        vec2 temp = vec2(px,py);
        controlPoints.push_back(temp);
        glutPostRedisplay();
    }
}

//Initializes OpenGL attributes
void GLInit(int* argc, char** argv)
{
    glutInit(argc, argv);
    //glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    //glMatrixMode(GL_PROJECTION_MATRIX);
    //glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glutCreateWindow("CS 130 - <mjime051>");
    glutDisplayFunc(GL_render);
    glutMouseFunc(GL_mouse);
}

int main(int argc, char** argv)
{
    GLInit(&argc, argv);
    glutMainLoop();
    return 0;
}
