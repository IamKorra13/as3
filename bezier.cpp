
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

#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }

using namespace std;

//****************************************************
// Some Classes
//****************************************************

class Viewport;

class Viewport {
  public:
    int w, h; // width and height
};

class Light {
	public:
		Light();
		vector<float> color; //rgb
		vector<float> pos; //xyz
};

Light::Light() {
	color.push_back(0.0); color.push_back(0.0); color.push_back(0.0);
	pos.push_back(0.0); pos.push_back(0.0); pos.push_back(0.0);
}
//****************************************************
// Triangle Class
//****************************************************
class Triangle {
	public:
		Triangle();
		Triangle(vector<float>, vector<float>, vector<float>);
		vector<float> v1, v2, v3;
};

Triangle::Triangle() {
	v1.push_back(0.0f); v1.push_back(0.0f); v1.push_back(0.0f);
	v2.push_back(0.0f); v2.push_back(0.0f); v2.push_back(0.0f);
	v3.push_back(0.0f); v3.push_back(0.0f); v3.push_back(0.0f);
}

Triangle::Triangle(vector<float> iv1, vector<float> iv2, vector<float> iv3) {
	v1 = iv1; v2 = iv2; v3 = iv3;
}

//****************************************************
// Curve Class
//****************************************************
class Curve {
	public:
		Curve();
		vector<float> pt1;
		vector<float> pt2;
		vector<float> pt3;
		vector<float> pt4;
		void print();
};

Curve::Curve() {
	pt1.push_back(0.0f); pt1.push_back(0.0f); pt1.push_back(0.0f);
	pt2.push_back(0.0f); pt2.push_back(0.0f); pt2.push_back(0.0f);
	pt3.push_back(0.0f); pt3.push_back(0.0f); pt3.push_back(0.0f);
	pt4.push_back(0.0f); pt4.push_back(0.0f); pt4.push_back(0.0f);
}

void Curve::print() {
	cout << "( " << pt1[0] << ", " << pt1[1] << ", " << pt1[2] << ") ";
	cout << "( " << pt2[0] << ", " << pt2[1] << ", " << pt2[2] << ") ";
	cout << "( " << pt3[0] << ", " << pt3[1] << ", " << pt3[2] << ") ";
	cout << "( " << pt4[0] << ", " << pt4[1] << ", " << pt4[2] << ") ";
	cout << endl;
	
}


//****************************************************
// Bezier Surface Patches
//****************************************************
class SurfacePatch {
	public:
		SurfacePatch(); 
		void print();
		//Curve c1, c2, c3, c4;
		vector<Curve> control_points;
};

SurfacePatch::SurfacePatch() {
	for(int i = 0; i < 4; i++) {
		Curve *item = new Curve();
		control_points.push_back(*item);	}
}

void SurfacePatch::print() {
	cout << endl;
	cout << "The Control Points: " << endl;
	for (int i = 0; i < 4; i++) {
		control_points[i].print();
	}
}

//****************************************************
// Print overall
//****************************************************
void print(string mes) {
	cout << mes << endl;
}

void print(string mes, int v) {
	cout << mes << v << endl;
}

//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;
int windowID;

float kd[3] = {0.0, 0.0, 0.0};
float ka[3] = {0.0, 0.0, 0.0};
float ks[3] = {0.0, 0.0, 0.0};
float sp = 0;

bool toonShader;

vector<Light> pl_list; // list of point lights
vector<Light> dl_list; // list of directional lights


//List of all the Surface Patches
vector<SurfacePatch> list_of_SPatches;
bool isUniform = false;
bool isAdaptive = false;
float step_size = 0.0f;

//****************************************************
// Simple init function
//****************************************************
void initScene(){

  // Nothing to do here for this simple example.

}


//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
  viewport.w = w;
  viewport.h = h;

  glViewport (0,0,viewport.w,viewport.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, viewport.w, 0, viewport.h);

}

//****************************************************
// This is the exit on spacebar function
//****************************************************
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

//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************

void setPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b) {
  //So we can add the changes here for caclulating the color of the pixel maybe
  glColor3f(r, g, b);
  glVertex2f(x + 0.5, y + 0.5);   // The 0.5 is to target pixel
  // centers 
  // Note: Need to check for gap
  // bug on inst machines.
}

//****************************************************
// Calculate the dot product of 2 vectors.
//****************************************************

float dot_product(vector<float> vector1, vector<float> vector2) {
	float result = 0;
	for (unsigned int i = 0; i < vector1.size(); i++) {
		result += vector1[i] * vector2[i];
	}
	return result;
}

//****************************************************
// Normalize 1 vector.
//****************************************************
vector<float> normalize(vector<float> input) {
	float magnitude = 0;
	vector<float> result;

	for (unsigned int i = 0; i < input.size(); i++) {
		magnitude += pow(input[i],2);
	}

	magnitude = sqrt(magnitude);
	for (unsigned int i = 0; i < input.size(); i++) {
		result.push_back(input[i]/magnitude);
	}

	return result;
}


//****************************************************
// Draw a filled circle.  
//****************************************************


void circle(float centerX, float centerY, float radius) {
  // Draw inner circle
  glBegin(GL_POINTS);

  // We could eliminate wasted work by only looping over the pixels
  // inside the sphere's radius.  But the example is more clear this
  // way.  In general drawing an object by loopig over the whole
  // screen is wasteful.

  int i,j;  // Pixel indices

  int minI = max(0,(int)floor(centerX-radius));
  int maxI = min(viewport.w-1,(int)ceil(centerX+radius));

  int minJ = max(0,(int)floor(centerY-radius));
  int maxJ = min(viewport.h-1,(int)ceil(centerY+radius));

  // Define viewer vector 
  //The viewer is at 0, 0, -1.0
  vector<float> v;
  v.push_back(0.0); v.push_back(0.0); v.push_back(-1.0);

  //   Ambient Term = ka*I
  //float ambient[3] = {ka[0]*I[0], ka[1]*I[1], ka[2]*I[2]};
	float ambient[3] = {0.0, 0.0, 0.0};
	for (int m = 0; m < pl_list.size(); m++) {
		ambient[0] += ka[0]*pl_list[m].color[0];
		ambient[1] += ka[1]*pl_list[m].color[1];
		ambient[2] += ka[2]*pl_list[m].color[2];
	}
	for (int m = 0; m < dl_list.size(); m++) {
		ambient[0] += ka[0]*dl_list[m].color[0];
		ambient[1] += ka[1]*dl_list[m].color[1];
		ambient[2] += ka[2]*dl_list[m].color[2];
	}

  // MAIN LOOP
  for (i=0;i<viewport.w;i++) {
    for (j=0;j<viewport.h;j++) {

      // Location of the center of pixel relative to center of sphere
      float x = (i+0.5-centerX);
      float y = (j+0.5-centerY);

      float dist = sqrt(sqr(x) + sqr(y)); // how far away the pixel is from the center of the sphere

      if (dist<=radius) {//only if the point is within the radius then 

        // This is the front-facing Z coordinate
        float z = sqrt(radius*radius-dist*dist);

		// n = surface normal, right now the xyz point on the sphere
		vector<float> n;
		//z = sqrt(radius*radius - i*i - j*j);
		n.push_back(x); n.push_back(y); n.push_back(z);
		n = normalize(n);
        
    // Diffuse term = kd*I*max(l*n, 0)
		// l = direction of light, Light.pos vector
			// point light - l = location of light - current location on sphere (ijz)
			// diffuse light - l = xyz input from command line
		// multiple lights = calculate l vector for each light, compute diffuse term for each light, and then add it all together for final diffuse value
		
		float diffuse[3] = {0.0, 0.0, 0.0};
		for (int q = 0; q < pl_list.size(); q++) {
			//std::cout << "Positions of Light " << pl_list.size() << q << j << i;
			Light light = pl_list[q];			
			light.pos[0] = light.pos[0]*radius-x;
			light.pos[1] = light.pos[1]*radius-y;
			light.pos[2] = light.pos[2]*radius-z;

			light.pos = normalize(light.pos);
			

			float dotProdln = dot_product(light.pos, n);
			float maxdotProd = max(dotProdln, 0.0f);

			diffuse[0] += kd[0] * light.color[0] * maxdotProd;
			diffuse[1] += kd[1] * light.color[1] * maxdotProd;
			diffuse[2] += kd[2] * light.color[2] * maxdotProd;
		}

		for (int q = 0; q < dl_list.size(); q++) {
			Light light = dl_list[q];
			light.pos[0] = light.pos[0]*radius*-1;
			light.pos[1] = light.pos[1]*radius*-1;
			light.pos[2] = light.pos[2]*radius*-1;
			light.pos = normalize(light.pos);

			float dotProdln = dot_product(light.pos, n);
			float maxdotProd = max(dotProdln, 0.0f);

			diffuse[0] += kd[0] * light.color[0] * maxdotProd;
			diffuse[1] += kd[1] * light.color[1] * maxdotProd;
			diffuse[2] += kd[2] * light.color[2] * maxdotProd;
		}
		

		// Specular term = ks* I * max(r*v, 0)^p
		// r = reflected direction, r = -l + 2(l*n)n
		float specular[3] = {0.0, 0.0, 0.0};
		for (int q = 0; q < pl_list.size(); q++) {
			Light light = pl_list[q];

			light.pos[0] = light.pos[0] * -1;
			light.pos[1] = light.pos[1] * -1;
			light.pos[2] = light.pos[2] * -1;

			vector<float> r;
			r.push_back(-light.pos[0] + 2*(dot_product(light.pos, n))*n[0]);
			r.push_back(-light.pos[1] + 2*(dot_product(light.pos, n))*n[1]);
			r.push_back(-light.pos[2] + 2*(dot_product(light.pos, n))*n[2]);

			r = normalize(r);

			float dotProdrv = dot_product(r, v);
			float dotProdrvmax = pow(max(dotProdrv, 0.0f), sp);

			specular[0] += ks[0] * light.color[0] * dotProdrvmax;
			specular[1] += ks[1] * light.color[1] * dotProdrvmax;
			specular[2] += ks[2] * light.color[2] * dotProdrvmax;
		}
		for (int q = 0; q < dl_list.size(); q++) {
			Light light = dl_list[q];
			light.pos[0] = light.pos[0] * -1;
			light.pos[1] = light.pos[1] * -1;
			light.pos[2] = light.pos[2] * -1;

			vector<float> r;
			r.push_back(-light.pos[0] + 2*(dot_product(light.pos, n))*n[0]);
			r.push_back(-light.pos[1] + 2*(dot_product(light.pos, n))*n[1]);
			r.push_back(-light.pos[2] + 2*(dot_product(light.pos, n))*n[2]);
			r[0] = -r[0];
			r[1] = -r[1];
			r[2] = -r[2];
			r = normalize(r);

			float dotProdrv = dot_product(r, v);
			float dotProdrvmax = pow(max(dotProdrv, 0.0f), sp);

			specular[0] += ks[0] * light.color[0] * dotProdrvmax;
			specular[1] += ks[1] * light.color[1] * dotProdrvmax;
			specular[2] += ks[2] * light.color[2] * dotProdrvmax;
		}

		// Final Result :)
		setPixel(i,j, ambient[0] + diffuse[0] + specular[0], ambient[1] + diffuse[1] + specular[1], ambient[2] + diffuse[2] + specular[2]);

        // This is amusing, but it assumes negative color values are treated reasonably.
        //setPixel(i,j, x/radius, y/radius, z/radius );
      }
    }
  }
  glEnd();
}

//****************************************************
//Uniform Subdisivion
// Takes in step size and a surface patch
//****************************************************
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
			print("v", v);
		// evaluate surface
			float p, n;
			//p, n = bezpatchinterp(sp, u, v);
			//savesurfacepointandnormal(p,n)
		}
	}
}

//****************************************************
//Uniform Subdisivion
//given the control points of a bezier curve and a parametric value, 
//return the curve point and derivative
//****************************************************

vector<float> bezcurveinterp(Curve curve, float u) {
	// first, split each of the three segments # to form two new ones AB and BC
	//A B and C are vectors with (x,y,z)
	vector<float> A, B, C, D, E;
	A[0] = curve.pt1[0] * (1.0-u) + curve.pt2[0] * u;
	A[1] = curve.pt1[1] * (1.0-u) + curve.pt2[1] * u;
	A[2] = curve.pt1[2] * (1.0-u) + curve.pt2[2] * u;
	
	B[0] = curve.pt2[0] * (1.0-u) + curve.pt3[0] * u;
	B[1] = curve.pt2[1] * (1.0-u) + curve.pt3[1] * u;
	B[2] = curve.pt2[2] * (1.0-u) + curve.pt3[2] * u;

	C[0] = curve.pt3[0] * (1.0-u) + curve.pt4[0] * u;
	C[1] = curve.pt3[1] * (1.0-u) + curve.pt4[1] * u;
	C[2] = curve.pt3[2] * (1.0-u) + curve.pt4[2] * u;
	// now, split AB and BC to form a new segment DE D = A * (1.0-u) + B * u
	E[0] = B[0] * (1.0-u) + C[0] * u;
	E[1] = B[1] * (1.0-u) + C[1] * u;
	E[2] = B[2] * (1.0-u) + C[2] * u;
	// finally, pick the right point on DE, # this is the point on the curve
	vector<float> p, dPdu;
	p[0] = D[0] * (1.0-u) + E[0] * u;
	p[1] = D[1] * (1.0-u) + E[1] * u;
	p[2] = D[2] * (1.0-u) + E[2] * u;
	// compute derivative also 
	dPdu[0] = 3 * (E[0] - D[0]);
	dPdu[1] = 3 * (E[1] - D[1]);
	dPdu[2] = 3 * (E[2] - D[2]);
	return p, dPdu;
}

//****************************************************
// Uniform Subdivision
//***************************************************
// given a control patch and (u,v) values, find # the surface point and normal 
// vector<float> bezpatchinterp(patch, u, v) {
// 	//# build control points for a Bezier curve in v 
// 	// SurfacePatch
// 	Curve vcurve, ucurve;
// 	vcurve.pt1 = bezcurveinterp(patch[0][0:3], u); // sf have curves
// 	vcurve.pt2 = bezcurveinterp(patch[1][0:3], u); 
// 	vcurve.pt3 = bezcurveinterp(patch[2][0:3], u); 
// 	vcurve.pt4 = bezcurveinterp(patch[3][0:3], u);
// 	//build control points for a Bezier curve in u 
// 	ucurve.pt1 = bezcurveinterp(patch[0:3][0], v);
// 	ucurve.pt2 = bezcurveinterp(patch[0:3][1], v);
// 	ucurve.pt3 = bezcurveinterp(patch[0:3][2], v);
// 	ucurve.pt4 = bezcurveinterp(patch[0:3][3], v);
// 	// evaluate surface and derivative for u and v 
// 	p, dPdv = bezcurveinterp(vcurve, v)
// 	p, dPdu = bezcurveinterp(ucurve, u)
// 	# take cross product of partials to find normal n = cross(dPdu, dPdv)
// 	n = n / length(n)
// 	return p, n
// }
//****************************************************
// function that does the actual drawing of stuff
//***************************************************
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
	SurfacePatch sp = SurfacePatch();

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
					sp = SurfacePatch();
				}
				vector<float> *pt1 = new vector<float>(); vector<float> *pt2 = new vector<float>(); 
				vector<float> *pt3 = new vector<float>(); vector<float> *pt4 = new vector<float>(); 
				pt1->push_back((float) atof(token[i++])); pt1->push_back((float) atof(token[i++])); pt1->push_back((float) atof(token[i++]));
				pt2->push_back((float) atof(token[i++])); pt2->push_back((float) atof(token[i++])); pt2->push_back((float) atof(token[i++]));
				pt3->push_back((float) atof(token[i++])); pt3->push_back((float) atof(token[i++])); pt3->push_back((float) atof(token[i++]));
				pt4->push_back((float) atof(token[i++])); pt4->push_back((float) atof(token[i++])); pt4->push_back((float) atof(token[i++]));
				Curve *c = new Curve();
				c->pt1= *pt1; c->pt2 = *pt2; c->pt3 = *pt3; c->pt4 = *pt4; 
				sp.control_points[num_curves] = *c; //[0] = *pt1; sp.control_points[num_curves][1] = *pt2;
				//sp.control_points[num_curves][2] = *pt3; sp.control_points[num_curves][3] = *pt4;
				num_curves++;
				if (num_curves == 4) { 
					num_curves = 0; list_of_SPatches.push_back(sp);
					sp.print();
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
		if (isUniform) {
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








