
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
bool isUniform = false;
bool isAdaptive = false;
float step_size = 0.0f;


void initScene(){ }

/*Resizes Viewport.*/
void myReshape(int w, int h) {
  viewport.w = w;
  viewport.h = h;

  glViewport (0,0,viewport.w,viewport.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, viewport.w, 0, viewport.h);

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

/*Uniform Subdivision.*/
Vector bezcurveinterp(Curve curve, float u) {
	// first, split each of the three segments # to form two new ones AB and BC
	//A B and C are vectors with (x,y,z)
	print("bezcurveinterp");
	Vector A, B, C, D, E = Vector();
	curve.print();

	A = (curve.pt1 * (1.0-u)) + (curve.pt2 * u);
	print("A = "); A.print();
	// A.x = curve.pt1.x * (1.0-u) + curve.pt2.x * u;
	// A.y = curve.pt1.y * (1.0-u) + curve.pt2.y * u;
	// A.z = curve.pt1.z * (1.0-u) + curve.pt2.z * u;
	// print("A = "); print(A);
	// B.x = curve.pt2.x * (1.0-u) + curve.pt3.x * u;
	// B.y = curve.pt2.y * (1.0-u) + curve.pt3.y * u;
	// B.z = curve.pt2.z * (1.0-u) + curve.pt3.z * u;
	B = (curve.pt2 * (1.0-u)) + (curve.pt3 * u);
	
	print("B = "); B.print();

	C = (curve.pt3 * (1.0-u)) + (curve.pt4 * u);
	print("C = "); C.print();
	// print("B = "); print(B);
	// C.x = curve.pt3.x * (1.0-u) + curve.pt4.x * u;
	// C.y = curve.pt3.y * (1.0-u) + curve.pt4.y * u;
	// C.z = curve.pt3.z * (1.0-u) + curve.pt4.z * u;
	
	// now, split AB and BC to form a new segment DE 
	// D.x = A.x * (1.0-u) + B.x * u;
	// D.y = A.y * (1.0-u) + B.y * u;
	// D.z = A.z * (1.0-u) + B.z * u;
	// print("D = "); print(D);

	// E.x = B.x * (1.0-u) + C.x * u;
	// E.y = B.y * (1.0-u) + C.y * u;
	// E.z = B.z * (1.0-u) + C.z * u;
	// print("E ="); print(E);
	// finally, pick the right point on DE, # this is the point on the curve
	Vector p, dPdu;
	// dPdu.push_back(0.0f); dPdu.push_back(0.0f); dPdu.push_back(0.0f);
	// p.x = D.x * (1.0-u) + E.x * u;
	// py = Dy * (1.0-u) + Ey * u;
	// p.z = D.z * (1.0-u) + E.z * u;
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
/*Uniform Subdivision.*/
void subdividePatch(SurfacePatch sp, float step) {
	//compute how many subdivisions there # are for this step size
	float epsilon = 1.0f; //BecauseI I don't know the real value
	int numdiv = ((1 + epsilon) / step);
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
			//savesurfacepointandnormal(p,n)
		}
	}
}

/*Draws.*/
void myDisplay() {
	glClear(GL_COLOR_BUFFER_BIT);				// clear the color buffer
	glMatrixMode(GL_MODELVIEW);			        // indicate we are specifying camera transformations
	glLoadIdentity();

 	// /* Draws the triangles.*/
 	// circle(viewport.w / 2.0 , viewport.h / 2.0 , min(viewport.w, viewport.h) / 2.05);
 
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
			SurfacePatch sp = list_of_SPatches[i];
			print("Patch to be subdivided");
			sp.print(); sp.makeCurves();
			subdividePatch(list_of_SPatches[i], step_size);
		}
		//subdivide each patch
	}

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

}








