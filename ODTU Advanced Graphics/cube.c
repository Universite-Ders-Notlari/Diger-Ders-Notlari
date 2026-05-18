
#include <windows.h>
#include <gl\glut.h>
#include <stdio.h>

typedef struct {
int x; int y;
} MYPOINT;

typedef struct {
float r; float g; float b;
} COLOR;

int cube;
void initScene() {
    GLint pt[8][3] = {{0,0,0},{0,1,0},{1,0,0},{1,1,0},
                       {0,0,1},{0,1,1},{1,0,1},{1,1,1}};

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0,1.0,1.0,0.0);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(45.0, 1.0, 1.0, 20.0 ); 
	glMatrixMode(GL_MODELVIEW);

    cube = glGenLists(1);
    
    glNewList(cube,GL_COMPILE);
    
   glBegin(GL_QUADS);
      glColor3f(0.5,0.5,0.5);
      glVertex3iv(pt[7]);
      glVertex3iv(pt[5]);
      glVertex3iv(pt[4]);
      glVertex3iv(pt[6]);

      glColor3f(1.0,0.0,0.0);
      glVertex3iv(pt[6]);
      glVertex3iv(pt[2]);
      glVertex3iv(pt[3]);
      glVertex3iv(pt[7]);
      glColor3f(1.0,1.0,0.0);
      glVertex3iv(pt[5]);
      glVertex3iv(pt[1]);
      glVertex3iv(pt[0]);
      glVertex3iv(pt[4]);
      glColor3f(0.0,1.0,0.0);
      glVertex3iv(pt[7]);
      glVertex3iv(pt[3]);
      glVertex3iv(pt[1]);
      glVertex3iv(pt[5]);
      glColor3f(0.0,1.0,1.0);
      glVertex3iv(pt[4]);
      glVertex3iv(pt[0]);
      glVertex3iv(pt[2]);
      glVertex3iv(pt[6]);
      glColor3f(0.0,0.0,1.0);
      glVertex3iv(pt[2]);
      glVertex3iv(pt[0]);
      glVertex3iv(pt[1]);
      glVertex3iv(pt[3]);

     glEnd();

    glEndList();


}

void drawCube()
{
     glColor3f(1.0,0.0,0.0);
     glEnableClientState (GL_VERTEX_ARRAY);
     GLint pt[8][3] = {{0,0,0},{0,1,0},{1,0,0},{1,1,0},
                       {0,0,1},{0,1,1},{1,0,1},{1,1,1}};

     glVertexPointer (3, GL_INT, 0, pt);
     GLubyte vertIndex[24] ={6,2,3,7,5,1,0,4,7,3,1,5,4,0,2,6,2,0,1,3,7,5,4,6}; 
     glDrawElements (GL_QUADS, 24, GL_UNSIGNED_BYTE, vertIndex);

}
void drawTriangle(MYPOINT a, MYPOINT b, MYPOINT c, COLOR col)
{
     glColor3f(col.r,col.g,col.b);
     /* the vertices should be given in counterclockwise order */
     glBegin(GL_TRIANGLES);
         glVertex2i(a.x,a.y);
         glVertex2i(b.x,b.y);
         glVertex2i(c.x,c.y);
     glEnd();
}
void renderScene() {

    COLOR col;
    MYPOINT a,b,c;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

  glTranslatef(-2.0,-2.0,-22.0);
   glScalef(1.0,1.0,15.0);
//    drawCube();
    glCallList(cube);
    
    glLoadIdentity();
  glTranslatef(2.0,-2.0,-22.0);
   glScalef(0.7,0.7,10.0);
    glCallList(cube);
 
	glutSwapBuffers();


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

	initScene();

	glutMainLoop();

	return(0);
}
