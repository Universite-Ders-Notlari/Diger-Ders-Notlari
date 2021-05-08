
#include <windows.h>
#include <gl\glut.h>
#include <stdio.h>

typedef struct {
int x; int y;
} MYPOINT;

float cameraposz = -200;
float angle = 0;

float distance = 0;

int yon = 1;

float currx = 0;
float currz = 100;

typedef struct {
float r; float g; float b;
} COLOR;

int selectedPiece;

void initScene() {

    glClearColor(1.0,1.0,1.0,0.0);
}
void drawWorld()
{

    glColor3f(0.0,0.0,1.0);

        glBegin(GL_LINES);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(100.0,0.0,0.0);
    glEnd();

    glColor3f(0.0,1.0,0.0);

    glBegin(GL_LINES);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,100.0,0.0);
    glEnd();

    glColor3f(0.0,0.0,0.0);

    glBegin(GL_LINES);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,0.0,100.0);
    glEnd();

    glColor3f(0.2,0.5,0.2);

    glBegin(GL_QUADS);
         glVertex3f(-100.0,-1.0,100.0);
         glVertex3f(100.0,-1.0,100.0);
         glVertex3f(100.0,-1.0,-100.0);
         glVertex3f(-100.0,-1.0,-100.0);
    glEnd();


    glColor3f(1.0,0.0,0.0);
    
    //glRotatef(angle,0,1,0);
    
    glTranslatef(currx,0.0,currz);
    
    
    glBegin(GL_QUADS);
         glVertex3f(-10.0,1.0,10.0);
         glVertex3f(10.0,1.0,10.0);
         glVertex3f(10.0,1.0,-10.0);
         glVertex3f(-10.0,1.0,-10.0);
    glEnd();
}
void renderScene() {
     
    GLfloat matrix[16];
    int i,j;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0,0,600,600);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(100,1,1,1000);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0,-100,cameraposz);	
    drawWorld();
    
    glViewport(400,400,200,200);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(-100,100,-100,100,-100,100);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(90,1,0,0);	
    drawWorld();
        

	glutSwapBuffers();


}

void animate()
{

              //angle+=0.2;
              //cameraposz+=0.04;
              if (yon==1) { currz-=0.1; distance+=0.1; if (distance>=50) { yon=0; distance  = 0; } }
              if (yon==0) { currx+=0.1; distance+=0.1; if (distance>=50) { yon=1; distance  = 0; } }
              
              glutPostRedisplay();
              
}

/*-------------------
Keyboard
-------------------*/
void processNormalKeys(unsigned char key, int x, int y) {
	
	if (key == 27) {
		exit(0);
	}
}

/*---------
Main
---------*/

int main(int argc, char **argv)
{

	glutInitDisplayMode(GLUT_DEPTH  | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10,10);
	glutInitWindowSize(600,600);
	glutCreateWindow("Warm-up Homework");

	glutKeyboardFunc(processNormalKeys);
	glutDisplayFunc(renderScene);
	
	glutIdleFunc(animate);

	initScene();

	glutMainLoop();

	return(0);
}
