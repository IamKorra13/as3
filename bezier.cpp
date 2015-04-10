
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
// Bezier Surface Patches
//****************************************************
class SurfacePatch {
	public:
		SurfacePatch();
		float control_points [4][4];
};

SurfacePatch::SurfacePatch() {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			control_points[i][j] = 0.0f;
		}
	}
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

	 // read each line of the file
	while (!fin.eof()) {
		// read an entire line into memory
		char buf[MAX_CHARS_PER_LINE];
		fin.getline(buf, MAX_CHARS_PER_LINE);
    
		// parse the line into blank-delimited tokens
		int n = 0;
    
		// array to store memory addresses of the tokens in buf
		const char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
    
		// parse the line
		token[0] = strtok(buf, DELIMITER); // first token
		if (token[0]) { // zero if line is blank
		  for (n = 1; n < MAX_TOKENS_PER_LINE; n++) {
			token[n] = strtok(0, DELIMITER); // subsequent tokens
			if (!token[n]) break; // no more tokens
		  }
		}

		// process tokens
		int num_of_surface_patches = 0;
		for (int i = 0; i < n; i++) { // n = #of tokens
			// This is the first line of the file that says the number of surface patches
			if (n == 1) { num_of_surface_patches = (int) atof(token[i]);}
			cout << "Number of Surface Patches = " << num_of_surface_patches << endl;
			//if(strcmp(token[i], "cam") == 0)
		}
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

	// for(int i = 0; i < argc; i++) {

	// 	// First argument is the file
	// 	if(strcmp(argv[i], "-pt") == 0) {
	// 		SurfacePatch sp = SurfacePatch();
	// 		sp.control_points[0][0] = atof(argv[i+1]); sp.control_points[0][1] = atof(argv[i+2]); sp.control_points[0][2] = atof(argv[i+3]);  
	// 		//temp.push_back(atof(argv[i+1])); temp.push_back(atof(argv[i+2])); temp.push_back(atof(argv[i+3]));
	// 		//triangle.v1 = temp;
	// 		cout << "I saved a Surface Patch" << endl;
	// 		cout << "Surface Patch : (" << sp.control_points[0][0] << ", " << sp.control_points[0][1] << ", " << sp.control_points[0][2] << ")" << endl;
	// 		i = i + 3;
	// 	}
	// }

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








