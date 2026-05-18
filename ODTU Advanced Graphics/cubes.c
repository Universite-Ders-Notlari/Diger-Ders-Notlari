
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#include <time.h>

typedef struct {
        float x; float y; float z;
} POS;

typedef struct
{
	double x,y,z;
} Vec3;

typedef struct {
float r; float g; float b;
} COLOR;

typedef struct {
        COLOR c;
        POS p;
        float maxDisp;
        float currDisp;
        int goingUp;
} CUBEINFO;

CUBEINFO cubes[100];
int numCubes = 0;

float camRadius;
float camHeight;
float camAngle;

Vec3 cross(Vec3 a, Vec3 b)
{
	Vec3 tmp;
	
	tmp.x = a.y*b.z-b.y*a.z;
	tmp.y = b.x*a.z-a.x*b.z;
	tmp.z = a.x*b.y-b.x*a.y;
	
	return tmp;
}

double length(Vec3 v)
{
	return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

Vec3 normalize(Vec3 v)
{
	Vec3 tmp;
	double d;
	
	d=length(v);
	tmp.x = v.x/d;
	tmp.y = v.y/d;
	tmp.z = v.z/d;
	
	return tmp;
}

void initCubes(int n)
{
     int i;
     numCubes = n;
     srand(time(NULL));
     for (i=0;i<n;i++)
     {
         cubes[i].c.r = (rand()%255)/255.0;
         cubes[i].c.g = (rand()%255)/255.0;
         cubes[i].c.b = (rand()%255)/255.0;
         
         cubes[i].p.x = -100+(rand()%200);
         cubes[i].p.y = 0+(rand()%50);
         cubes[i].p.z = -100+(rand()%200);
         
         cubes[i].maxDisp = 10+(rand()%30);
         cubes[i].currDisp = 0;
         cubes[i].goingUp = 1;
     }
}

void initScene() {

    glClearColor(0.0,0.0,0.0,0.0);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(60,1,1,1000);
	glMatrixMode(GL_MODELVIEW);
	initCubes(20);
	camRadius = 300;
	camHeight = 100;
	camAngle = 270;
}

void drawCube(COLOR c)
{
     glScalef(10,10,10);
     glBegin(GL_QUADS);
        //front face
        glColor3f(c.r,c.g,c.b);
        glVertex3f(-1,-1,1);
        glVertex3f(1,-1,1);
        glVertex3f(1,1,1);
        glVertex3f(-1,1,1);
        
        //back face
        glColor3f(0.6*c.r,0.6*c.g,0.6*c.b);
        glVertex3f(-1,1,-1);
        glVertex3f(1,1,-1);
        glVertex3f(1,-1,-1);
        glVertex3f(-1,-1,-1);

        //top face        
        glColor3f(1.2*c.r,1.2*c.g,1.2*c.b);
        glVertex3f(-1,1,1);
        glVertex3f(1,1,1);
        glVertex3f(1,1,-1);
        glVertex3f(-1,1,-1);
        
        //bottom face
        glColor3f(0.6*c.r,0.6*c.g,0.6*c.b);
        glVertex3f(-1,-1,-1);
        glVertex3f(1,-1,-1);
        glVertex3f(1,-1,1);
        glVertex3f(-1,-1,1);

        // left face
        glColor3f(0.8*c.r,0.8*c.g,0.8*c.b);
        glVertex3f(1,-1,1);
        glVertex3f(1,-1,-1);
        glVertex3f(1,1,-1);
        glVertex3f(1,1,1);

        // right face        
        glColor3f(0.8*c.r,0.8*c.g,0.8*c.b);
        glVertex3f(-1,1,1);
        glVertex3f(-1,1,-1);
        glVertex3f(-1,-1,-1);
        glVertex3f(-1,-1,1);
     glEnd();
}
void renderScene() {
     
    int i,j;
    float camx,camy,camz;
    float rmatrix[16];
    Vec3 n,u,v;


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    
    camx = camRadius*cos(camAngle*M_PI/180.0);
    camy = camHeight;
    camz = -camRadius*sin(camAngle*M_PI/180.0);
    
    n.x = camx; n.y = camy; n.z = camz;
    
    n = normalize(n);
    
    v.x = 0; v.y = 1; v.z = 0;
    
    u = normalize(cross(v,n));
    
    v = normalize(cross(n,u));
    
    rmatrix[0]=u.x; rmatrix[1]=v.x; rmatrix[2]=n.x; rmatrix[3]=0;
    rmatrix[4]=u.y; rmatrix[5]=v.y; rmatrix[6]=n.y; rmatrix[7]=0;
    rmatrix[8]=u.z; rmatrix[9]=v.z; rmatrix[10]=n.z; rmatrix[11]=0;
    rmatrix[12]=0; rmatrix[13]=0; rmatrix[14]=0; rmatrix[15]=1;
    
    glMultMatrixf(rmatrix);
    glTranslatef(-camx,-camy,-camz);
    
    for (i=0;i<numCubes;i++)
    {
        glPushMatrix();
        glTranslatef(cubes[i].p.x,cubes[i].p.y,cubes[i].p.z);
        glTranslatef(0,cubes[i].currDisp,0);
        drawCube(cubes[i].c);
        glPopMatrix();
    }
        

	glutSwapBuffers();


}

void animate()
{
     int i;
     
     for (i=0;i<numCubes;i++)
     {
         cubes[i].p.y+=0.01*cubes[i].goingUp;
         cubes[i].currDisp+=0.01*cubes[i].goingUp;
         if ((cubes[i].currDisp>0 && cubes[i].currDisp>=cubes[i].maxDisp) || (cubes[i].currDisp<0 && -cubes[i].currDisp>=cubes[i].maxDisp)) cubes[i].goingUp*=-1;
     }
     camAngle+=0.01;
     if (camAngle>=360) camAngle = 0;         
     glutPostRedisplay();
              
}

/*-------------------
Keyboard
-------------------*/
void processNormalKeys(unsigned char key, int x, int y) {
	
	if (key == 27) {
		exit(0);
	}
	if (key == 'q' || key == 'Q')
	{
            camHeight+=5.0;
    }
	if (key == 'a' || key == 'A')
	{
            camHeight-=5.0;
    }
	if (key == 'p' || key == 'P')
	{
            camRadius+=5.0;
    }
	if (key == 'l' || key == 'L')
	{
            camRadius-=5.0;
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
	glutCreateWindow("Cubes");

	glutKeyboardFunc(processNormalKeys);
	glutDisplayFunc(renderScene);
	
	glutIdleFunc(animate);

	initScene();

	glutMainLoop();

	return(0);
}
