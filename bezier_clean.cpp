
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

Viewport viewport;
int windowID;

//List of all the Surface Patches
vector<SurfacePatch> list_patches;
vector<vector<Vector> > points_to_render;
vector<Triangle> list_triangles;
vector<Vector> curlevel;

bool isUniform = false;
bool isAdaptive = false;
float step_size = 0.0f;
int numSubdivisions = 0;


/***** OpenGL viewport functions ********/
void initScene(){ 
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent
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
        exit(0);
    }
  }
}


void drawTriangle(Triangle triangle) {
    Vector v1 = triangle.v1; Vector v2 = triangle.v2; Vector v3 = triangle.v3;
    
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glBegin(GL_TRIANGLES);
    
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v2.x, v2.y, v2.z);
	glVertex3f(v3.x, v3.y, v3.z);
	glEnd();
    
    points_to_render = vector<vector<Vector> >();
    curlevel = vector<Vector>();
}


/****** Bezier Functions *******/
vector<Vector> bezcurveinterp(Curve curve, float u) {
	// first, split each of the three segments # to form two new ones AB and BC
	//A B and C are vectors with (x,y,z)
	Vector A, B, C, D, E, temp1, temp2 = Vector();
	
    A = curve.pt1 * (1.0f-u) + curve.pt2 * u;
    B = curve.pt2 * (1.0f-u) + curve.pt3 * u;
    C = curve.pt3 * (1.0f-u) + curve.pt4 * u;
	
	
	// now, split AB and BC to form a new segment DE
    D = A * (1.0f - u) + B * u;
    E = B * (1.0f - u) + C * u;
	
	// finally, pick the right point on DE, # this is the point on the curve
	Vector p, dPdu;	
	temp1.scalar_multiply(D, (1.0f-u)); temp2.scalar_multiply(E, u);
	p.add(temp1, temp2);

	// // compute derivative also 
	// dPdu[0] = 3 * (E[0] - D[0]);
	// dPdu[1] = 3 * (E[1] - D[1]);
	// dPdu[2] = 3 * (E[2] - D[2]);
	temp1.subtract(E, D); dPdu.scalar_multiply(temp1, 3.0f);
	vector<Vector> result;
	result.push_back(p); result.push_back(dPdu);
    cout << "p5: "; p.print();
	return result;
}

/* Uniform Subdivision.*/
// // given a control patch and (u,v) values, find # the surface point and normal 
vector<Vector> bezpatchinterp(SurfacePatch patch, float u, float v) {
	//# build control points for a Bezier curve in v
	Curve vcurve, ucurve;
	vector<Vector> Vcurves1, Vcurves2, Vcurves3, Vcurves4;
	vector<Vector> Ucurves1, Ucurves2, Ucurves3, Ucurves4;
	Vcurves1 = bezcurveinterp(patch.vcurves[0], u);
	Vcurves2 = bezcurveinterp(patch.vcurves[1], u);
	Vcurves3 = bezcurveinterp(patch.vcurves[2], u);
	Vcurves4 = bezcurveinterp(patch.vcurves[3], u);

	vcurve.pt1 = Vcurves1[0];
	vcurve.pt2 = Vcurves2[0];
	vcurve.pt3 = Vcurves3[0];
	vcurve.pt4 = Vcurves4[0];

	//build control points for a Bezier curve in u 
	Ucurves1 = bezcurveinterp(patch.ucurves[0], v);
	Ucurves2 = bezcurveinterp(patch.ucurves[1], v);
	Ucurves3 = bezcurveinterp(patch.ucurves[2], v);
	Ucurves4 = bezcurveinterp(patch.ucurves[3], v);

	ucurve.pt1 = Ucurves1[0];
	ucurve.pt2 = Ucurves2[0];
	ucurve.pt3 = Ucurves3[0];
	ucurve.pt4 = Ucurves4[0];
	// evaluate surface and derivative for u and v 

	// p, dPdv = bezcurveinterp(vcurve, v);
	// p, dPdu = bezcurveinterp(ucurve, u);
	vector<Vector> result1 = bezcurveinterp(vcurve, v);
	vector<Vector> result2 = bezcurveinterp(ucurve, u);

	Vector p = result1[0]; Vector dPdv = result1[1]; Vector dPdu = result2[1];
	// p = bezcurveinterp(vcurve, v);
	// p = bezcurveinterp(ucurve, u);
	// // # take cross product of partials to find normal 
	Vector n = dPdu.cross_product(dPdv);
	n.normalize(); //print("Normal = "); //print(n);
	vector<Vector> newResult; newResult.push_back(p); newResult.push_back(n);
	return newResult;
}

/* Saves the point in structure to be converted to triangles then rendered.*/
void savesurfacepointandnormal(Vector p) {
	if (curlevel.size() == numSubdivisions + 1) {
		points_to_render.push_back(curlevel);
		curlevel = vector<Vector>();
	}
    cout << "p3: "; p.print();
	curlevel.push_back(p);
}

/*Uniform Subdivision.*/
void subdividePatch(SurfacePatch sp, float step) {
	//compute how many subdivisions there # are for this step size
	float epsilon = 0.05f; //BecauseI I don't know the real value
	int numdiv = ((1 + epsilon) / step);
	numSubdivisions = numdiv;
    
	//for each parametric value of u 
	for (int iu = 0; iu <= numdiv; iu++) {
		float u = iu * step;
        
		// for each parametric value of v 
		for (int iv = 0; iv <= numdiv; iv++) {
			float v = iv * step;
            
            // evaluate surface
			vector<Vector> result;
			result = bezpatchinterp(sp, u, v);
			Vector p = result[0];
			Vector n = result[1];
			p.makeNormal(n);
			// print("Print the normal = "); 
			// print(p.normal);
			// I am not sure how to do this
            cout << "p2: "; p.print();
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
 	for (int i = 0; i < list_triangles.size(); i++) {
 		drawTriangle(list_triangles[i]);
 	}
    
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
                    num_curves = 0; list_patches.push_back(*sp);
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
    // Parse the bezier file or OBJ file
	if(argc >= 2) {
		parse_input(argv[1]);
        step_size = (float) atof(argv[2]);
        
        if (argc == 4) {
            if (strcmp(argv[3], "-a") == 0) {
                isAdaptive = true;
            }
        } else {
            isUniform = true;
        }
	}

	cout << "Step Size = " << step_size << endl;
    
    // DEBUG: Print patch
    cout << "Patches:" << endl;
    cout << "========" << endl;
    for (int a = 0; a < list_patches.size(); a++) {
        list_patches[a].print();
    }
    
	// Given a patch preform uniform subdisision
	for (int i = 0; i < list_patches.size(); i++) {
		if (isUniform) {
			list_patches[i].makeCurves();
			subdividePatch(list_patches[i], step_size);
		}
	}
    
    points_to_render.push_back(curlevel);
    
    cout << "Num Subdivisions = " << numSubdivisions << endl;
    cout << "Points to Render rows = " << points_to_render.size() << " columns =" << points_to_render[0].size() << endl;
    cout << endl;

    
    // DEBUG: Iterate through points_to_render
    cout << "Points to Render:" << endl;
    cout << "=================" << endl;
    for(int j = 0; j < points_to_render.size(); j++) {
        for(int k = 0; k < points_to_render[0].size(); k++) {
            points_to_render[j][k].print();
        }
        cout << endl;
    }
    
    // Create list of triangles
    for (int i = 0; i < numSubdivisions; i++) {
        for (int j = 0; j < numSubdivisions; j++) {
            list_triangles.push_back(Triangle(points_to_render[i][j], points_to_render[i][j+1],
                     points_to_render[i+1][j]));
            list_triangles.push_back(Triangle(points_to_render[i+1][j], points_to_render[i][j+1],
                     points_to_render[i+1][j+1]));
        }
    }
    
    // DEBUG: Print list_triangles
    cout << "List_triangles size: " << list_triangles.size() << endl;
    for(int i = 0; i < list_triangles.size(); i++) {
        cout << i << ": "; list_triangles[i].print();
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
//glPolygonMode(GL_FRONT, GL_LINE);
  glutDisplayFunc(myDisplay);				// function to run when its time to draw something
  glutReshapeFunc(myReshape);       // function to run when the window gets resized
  glutKeyboardFunc(keyBoardFunc);		// function to run to exit window with spacebar		

  glutMainLoop();							// infinite loop that will keep drawing and resizing
  // and whatever else
 	return 0;
}








