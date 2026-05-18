#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <GL/glut.h>

#include <time.h>

// Timing variables
clock_t m_tTime;
int m_iNbFrames;
float m_fFPS;
clock_t ttime;

#define NUM_CIRCLES 2000


typedef struct scrPt_st {
         GLint x, y;
} scrPt;

typedef struct color_st {
	GLfloat r,g,b;
} color;

scrPt circle_centers[NUM_CIRCLES];
GLint circle_radii[NUM_CIRCLES];
color circle_colors[NUM_CIRCLES];

   void setPixel (GLint x, GLint y)
   {
      glBegin (GL_POINTS);
         glVertex2i (x, y);
      glEnd ( );
   }

   void circleMidpoint (scrPt circCtr, GLint radius)
   {
      scrPt circPt;

      GLint p = 1 - radius;         // Initial value of midpoint parameter.

      circPt.x = 0;             // Set coordinates for top point of circle.
      circPt.y = radius;

      void circlePlotPoints (scrPt, scrPt);

      /* Plot the initial point in each circle quadrant. */
      circlePlotPoints (circCtr, circPt);

      /* Calculate next points and plot in each octant. */
      while (circPt.x < circPt.y) {
         circPt.x++;
         if (p < 0)
            p += 2 * circPt.x + 1;
         else {
            circPt.y--;
            p += 2 * (circPt.x - circPt.y) + 1;
         }
         circlePlotPoints (circCtr, circPt);
      }
   }

   void circlePlotPoints (scrPt circCtr, scrPt circPt)
   {
      setPixel (circCtr.x + circPt.x, circCtr.y + circPt.y);
      setPixel (circCtr.x - circPt.x, circCtr.y + circPt.y);
      setPixel (circCtr.x + circPt.x, circCtr.y - circPt.y);
      setPixel (circCtr.x - circPt.x, circCtr.y - circPt.y);
      setPixel (circCtr.x + circPt.y, circCtr.y + circPt.x);
      setPixel (circCtr.x - circPt.y, circCtr.y + circPt.x);
      setPixel (circCtr.x + circPt.y, circCtr.y - circPt.x);
      setPixel (circCtr.x - circPt.y, circCtr.y - circPt.x);
   }


void init(void)
{
	int i;
	srand(time(NULL)); 
	glClearColor(1.0,1.0,1.0,1.0);
	glMatrixMode (GL_PROJECTION);
	gluOrtho2D(0.0,500.0,0.0,500.0);
	m_iNbFrames = 0;
	m_tTime = clock();
	m_fFPS = 0;

}

void renderText(int x, int y, char *text)
{
	char *p;
	glRasterPos2f(x,y);
	for (p = text; *p; p++) 	
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*p);
}


void display(void)
{
	int i;
	char s[80];
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0,0.0,0.0);

	for (i=0;i<NUM_CIRCLES;i++)
	{
		circle_centers[i].x = rand()%500;
		circle_centers[i].y = rand()%500;
		circle_radii[i] = rand()%100;;
		circle_colors[i].r = (rand()%255)/255.0;
		circle_colors[i].g = (rand()%255)/255.0;
		circle_colors[i].b = (rand()%255)/255.0;
	}

	for (i=0;i<NUM_CIRCLES;i++)
	{
		glColor3f(circle_colors[i].r,circle_colors[i].g,circle_colors[i].b);
		circleMidpoint(circle_centers[i],circle_radii[i]);
	}

	if( m_iNbFrames++ > 25 )
	{
		m_fFPS = (float)m_iNbFrames / ( clock() - m_tTime ) * CLOCKS_PER_SEC;
		m_iNbFrames = 0;
		m_tTime = clock();
	}

	// Showing FPS
	glColor3f( 0.0f, 0.0f, 0.0f );
	sprintf(s,"FPS: %f",m_fFPS);
	renderText(10,10,s);

	glutSwapBuffers();

}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(50,100);
	glutInitWindowSize(500,500);
	glutCreateWindow("An Example OpenGL Program");

	init();

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutMainLoop();
	return 0;
}
