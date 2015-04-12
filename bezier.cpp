
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <iostream>     // std::cout
#include <algorithm>    // std::max

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>

#include "triangle.cpp"
#include "curve.cpp"
#include "vector.cpp"
#include "surfacepatch.cpp"


using namespace std;

class Viewport;

class Viewport {
  public:
    int w, h; // width and height
};

/*Print Methods.*/
void print(string mes) {
	cout << mes << endl;
}
void print(string mes, int v) {
	cout << mes << v << endl;
}
void print(vector<float> v) {
	cout << "( " << v[0] << ", " << v[1] << ", " << v[2] << ") " << endl;
}

Viewport	viewport;
int windowID;

//List of all the Surface Patches
vector<SurfacePatch> list_of_SPatches;
vector<Vector> points_to_Render;
bool isUniform = false;
bool isAdaptive = false;
float step_size = 0.0f;
//The number of divisions per side pf surface patch
int numSubdivisions = 0;


void initScene(){ 
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent

  //myReshape(viewport.w,viewport.h);
}

/*Resizes Viewport.*/
void myReshape(int w, int h) {
  viewport.w = w;
  viewport.h = h;

  glViewport (0,0,viewport.w,viewport.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // gluOrtho2D(0, viewport.w, 0, viewport.h);
 glOrtho(-5, 5, -5, 5, 5, -5);

}

/*Exits on Spacebar.*/
 void keyBoardFunc(unsigned char key, int x, int y) {  
 // the x and y ar the position of the mouse on the window when the key is pressed?
 //Decimal ASCII is 32 or ASCII character is 'SP' for space bar
  {
    switch(key)
    {
      case 32:
      //glutDestroyWindow(windowID);
      exit(0);
    }
  }
}


void setPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b) {
  //So we can add the changes here for caclulating the color of the pixel maybe
  glColor3f(r, g, b);
  glVertex2f(x + 0.5, y + 0.5);   // The 0.5 is to target pixel
  // centers 
  // Note: Need to check for gap
  // bug on inst machines.
}

void triangle(Vector v1, Vector v2, Vector v3) {
	v1.print(); v2.print(); v3.print(); print("");
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v2.x, v2.y, v2.z);
	glVertex3f(v3.x, v3.y, v3.z);
	glEnd();
}

/*Uniform Subdivision.*/
Vector bezcurveinterp(Curve curve, float u) {
	// first, split each of the three segments # to form two new ones AB and BC
	//A B and C are vectors with (x,y,z)
	print("bezcurveinterp");
	Vector A, B, C, D, E, temp1, temp2 = Vector();
	curve.print();
	
	//A = curve[0] * (1.0-u) + curve[1] * u
	temp1.scalar_multiply(curve.pt1, (1.0-u)); temp2.scalar_multiply(curve.pt2, u);
	A.add(temp1, temp2);
	print("A = "); A.print();

	// B = curve[1] * (1.0-u) + curve[2] * u
	temp1.scalar_multiply(curve.pt2, (1.0-u)); temp2.scalar_multiply(curve.pt3, u);
	B.add(temp1, temp2);
	print("B = "); B.print();

	// C = curve[2] * (1.0-u) + curve[3] * u
	temp1.scalar_multiply(curve.pt3, (1.0-u)); temp2.scalar_multiply(curve.pt4, u);
	C.add(temp1, temp2);
	print("C = "); C.print();
	
	
	// now, split AB and BC to form a new segment DE 
	// D = A * (1.0-u) + B * u
	temp1.scalar_multiply(A, (1.0-u)); temp2.scalar_multiply(B, u);
	D.add(temp1, temp2);
	print("D = "); D.print();

	temp1.scalar_multiply(B, (1.0-u)); temp2.scalar_multiply(C, u);
	E.add(temp1, temp2);
	print("E ="); E.print();
	

	// finally, pick the right point on DE, # this is the point on the curve
	Vector p, dPdu;	
	temp1.scalar_multiply(D, (1.0-u)); temp2.scalar_multiply(E, u);
	p.add(temp1, temp2);
	print("p = "); p.print();
	// // compute derivative also 
	// dPdu[0] = 3 * (E[0] - D[0]);
	// dPdu[1] = 3 * (E[1] - D[1]);
	// dPdu[2] = 3 * (E[2] - D[2]);
	return p;
}

/* Uniform Subdivision.*/
// // given a control patch and (u,v) values, find # the surface point and normal 
Vector bezpatchinterp(SurfacePatch patch, float u, float v) {
	//# build control points for a Bezier curve in v 
	print("bezpatchinterp");
	print("The Surface Patch"); patch.print();
	print("VCurves"); patch.vcurves[0].print(); patch.vcurves[1].print();
	patch.vcurves[2].print(); patch.vcurves[3].print();
	// SurfacePatch
	Curve vcurve, ucurve;
	vcurve.pt1 = bezcurveinterp(patch.vcurves[0], u);
	vcurve.pt2 = bezcurveinterp(patch.vcurves[1], u); 
	vcurve.pt3 = bezcurveinterp(patch.vcurves[2], u); 
	vcurve.pt4 = bezcurveinterp(patch.vcurves[3], u);

	//build control points for a Bezier curve in u 
	ucurve.pt1 = bezcurveinterp(patch.ucurves[0], v);
	ucurve.pt2 = bezcurveinterp(patch.ucurves[1], v);
	ucurve.pt3 = bezcurveinterp(patch.ucurves[2], v);
	ucurve.pt4 = bezcurveinterp(patch.ucurves[3], v);
	// evaluate surface and derivative for u and v 
	Vector p, dPdv;
	// p, dPdv = bezcurveinterp(vcurve, v);
	// p, dPdu = bezcurveinterp(ucurve, u);
	p = bezcurveinterp(vcurve, v);
	p = bezcurveinterp(ucurve, u);
	// // # take cross product of partials to find normal 
	// n = cross(dPdu, dPdv);
	// n = n / length(n);
	return p;
}

/* Saves the point in structure to be converted to triangles then rendered.*/
void savesurfacepointandnormal(Vector p) {
	points_to_Render.push_back(p);
}

/*Uniform Subdivision.*/
void subdividePatch(SurfacePatch sp, float step) {
	//compute how many subdivisions there # are for this step size
	float epsilon = 0.80f; //BecauseI I don't know the real value
	int numdiv = ((1 + epsilon) / step);
	numSubdivisions = numdiv;
	cout << "NumDiv = " << numdiv << endl;
	//for each parametric value of u 
	for (int iu = 0; iu < numdiv; iu++) {
		float u = iu * step;
		print("u = ", u);
		// for each parametric value of v 
		for (int iv = 0; iv < numdiv; iv++) {
			float v = iv * step;
			print("v = ", v);
		// evaluate surface
			Vector p = Vector();
			Vector n = Vector();
			p = bezpatchinterp(sp, u, v);
			savesurfacepointandnormal(p);
		}
	}
}

/*Draws.*/
void myDisplay() {
	glClear(GL_COLOR_BUFFER_BIT);				// clear the color buffer
	glMatrixMode(GL_MODELVIEW);			        // indicate we are specifying camera transformations
	glLoadIdentity();

 	// /* Draws the triangles.*/
 	triangle(points_to_Render[0], points_to_Render[1], points_to_Render[numSubdivisions]);
 	// circle(viewport.w / 2.0 , viewport.h / 2.0 , min(viewport.w, viewport.h) / 2.05);
  //  glColor3f(1.0f,0.0f,0.0f);                   // setting the color to pure red 90% for the rect   
  // glBegin(GL_POLYGON);
  // glVertex3f(-1.0f, -1.0f, 0.0f);               // bottom left corner of rectangle
  // glVertex3f(-1.0f, 1.0f, 0.0f);               // top left corner of rectangle
  // glVertex3f( -0.9f, 1.0f, 0.0f);               // top right corner of rectangle
  // glVertex3f( -0.9f, -1.0f, 0.0f);               // bottom right corner of rectangle
  // glEnd();
	glFlush();
	glutSwapBuffers();					// swap buffers (we earlier set double buffer)
}


void parse_input(const char* input_file) {
	const int MAX_CHARS_PER_LINE = 512;
	const int MAX_TOKENS_PER_LINE = 20;
	const char* const DELIMITER = " ";

	ifstream fin;
	fin.open(input_file); // open a file

	if (!fin.good()) { 
		return; // exit if file not found
	}
	bool firstLine = false;
	//Number of curves per surface patch
	int num_curves = 0;
	SurfacePatch *sp = new SurfacePatch();

	 // read each line of the file
	while (!fin.eof()) {
		// read an entire line into memory
		char buf[MAX_CHARS_PER_LINE];
		fin.getline(buf, MAX_CHARS_PER_LINE);
    
		// parse the line into blank-delimited tokens
		int n = 0;
    
		// array to store memory addresses of the tokens in buf
		const char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
    

		int num_of_surface_patches = 0;

		// parse the line
		token[0] = strtok(buf, DELIMITER); // first token
		if (token[0]) { // zero if line is blank
		  for (n = 1; n < MAX_TOKENS_PER_LINE; n++) {
			token[n] = strtok(0, DELIMITER); // subsequent tokens
			if (!token[n]) break; // no more tokens
		  }
		}

		// process tokens

		for (int i = 0; i < n; i++) { // n = #of tokens
			if (firstLine) {
				if (num_curves == 0) {
					sp = new SurfacePatch();
				}
				Vector *pt1 = new Vector(); Vector *pt2 = new Vector(); Vector *pt3 = new Vector();
				Vector *pt4 = new Vector(); 
				pt1->x = (float) atof(token[i++]); pt1->y = (float) atof(token[i++]); pt1->z = (float) atof(token[i++]);
				pt2->x = (float) atof(token[i++]); pt2->y = (float) atof(token[i++]); pt2->z = (float) atof(token[i++]);
				pt3->x = (float) atof(token[i++]); pt3->y = (float) atof(token[i++]); pt3->z = (float) atof(token[i++]);
				pt4->x = (float) atof(token[i++]); pt4->y = (float) atof(token[i++]); pt4->z = (float) atof(token[i++]);
				Curve *c = new Curve();
				c->pt1= *pt1; c->pt2 = *pt2; c->pt3 = *pt3; c->pt4 = *pt4; 
				sp->cp[num_curves][0] = *pt1; sp->cp[num_curves][1] = *pt2;
				sp->cp[num_curves][2] = *pt3; sp->cp[num_curves][3] = *pt4;
				num_curves++;
				if (num_curves == 4) { 
					num_curves = 0; list_of_SPatches.push_back(*sp);
					sp->print();
				}
			}
		}
		firstLine = true;
}
}
//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
	// Parse input
	int i = 0;
	Triangle *tri = new Triangle;
	Vector *v = new Vector();
	// Parse the bezier file or OBJ file
	if(argc >= 2) {
		parse_input(argv[1]);

	}
	step_size = (float) atof(argv[2]);
	if (argc == 4) {
		if (strcmp(argv[3], "-a") == 0) {
			isAdaptive = true;
		}
	} else {
		isUniform = true;
	}

	cout << "Adaptive status = " << isAdaptive << " Uniform Status = " << isUniform << endl;
	cout << "Step Size = " << step_size << endl;

	// Given a patch preform uniform subdisision
	for (int i = 0; i < list_of_SPatches.size(); i++) {
		print("Number of patches = ", list_of_SPatches.size());
		if (isUniform) {
			list_of_SPatches[i].print(); list_of_SPatches[i].makeCurves();
			subdividePatch(list_of_SPatches[i], step_size);
		}
		//subdivide each patch
	}

	print("Number of points to render = ", points_to_Render.size());
	print("Number of subdivisions = ", numSubdivisions);
  //This initializes glut
  glutInit(&argc, argv);

  //This tells glut to use a double-buffered window with red, green, and blue channels 
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  // Initalize theviewport size
  viewport.w = 400;
  viewport.h = 400;

  //The size and position of the window
  glutInitWindowSize(viewport.w, viewport.h);
  glutInitWindowPosition(0,0);
  windowID = glutCreateWindow(argv[0]);  // saving the ID of the window possibly for quiting on spacebar

  initScene();							// quick function to set up scene

  glutDisplayFunc(myDisplay);				// function to run when its time to draw something
  glutReshapeFunc(myReshape);       // function to run when the window gets resized
  glutKeyboardFunc(keyBoardFunc);		// function to run to exit window with spacebar		

  glutMainLoop();							// infinite loop that will keep drawing and resizing
  // and whatever else
 	return 0;
}








