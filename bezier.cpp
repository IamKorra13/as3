
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
#include <cfloat>

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

Viewport	viewport;
int windowID;

//List of all the Surface Patches
vector<Triangle> list_triangles;
vector<SurfacePatch> list_of_SPatches;
vector<vector<Vector> > points_to_Render;
vector<Vector> curlevel;

float step_size = 0.0f;
//The number of divisions per side pf surface patch
int numSubdivisions = 0;

// user settings
bool wireframe = false;
bool isAdaptive = false;
bool flatShading = false;

// rotation
float rotate_x = 0.0f;
float rotate_y = 0.0f;


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
    // gluOrtho2D(10, viewport.w, 10, viewport.h);
    glOrtho(-4, 4, -4, 4, 4, -4);
    
}

void drawTriangle(Triangle tri) {
    Vector v1 = tri.v1;
    Vector v2 = tri.v2;
    Vector v3 = tri.v3;
    
    glBegin(GL_TRIANGLES);
    
    glColor3f(1.0f, 0.0f, 0.0f);
    glNormal3f(v1.normal[0], v1.normal[1], v1.normal[2]);
    glVertex3f(v1.x, v1.y, v1.z);
    glNormal3f(v2.normal[0], v2.normal[1], v2.normal[2]);
    glVertex3f(v2.x, v2.y, v2.z);
    glNormal3f(v3.normal[0], v3.normal[1], v3.normal[2]);
    glVertex3f(v3.x, v3.y, v3.z);
    
    glEnd();
    
}

/* Main display function. */
void myDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);				// clear the color buffer
    glMatrixMode(GL_MODELVIEW);			        // indicate we are specifying camera transformations
    glLoadIdentity();
    
    glPushMatrix();
    
    glRotatef(rotate_y, 1.0, 0.0, 0.0);
    glRotatef(rotate_x, 0.0, 1.0, 0.0);
    /* Draws the triangles.*/
    for (int i = 0; i < list_triangles.size(); i++) {
        drawTriangle(list_triangles[i]);
    }
    glPopMatrix();
    
    glFlush();
    glutSwapBuffers();					// swap buffers (we earlier set double buffer)
}


void keyBoardFunc(unsigned char key, int x, int y) {
        switch(key)
        {
            case 32: //spacebar
                exit(0);
            case 'w':
                if(wireframe == false) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    wireframe = true;
                    
                }
                else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    wireframe = false;
                }
                myDisplay();
                break;
            case 's':
                if(flatShading == false) {
                    glShadeModel(GL_SMOOTH);
                    flatShading = true;
                }
                else {
                    glShadeModel(GL_FLAT);
                    flatShading = false;
                }
                myDisplay();
                break;
                
        }
}

//For the move tool
void processSpecialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            rotate_y += 10.0f;
            myDisplay();
            break;
        case GLUT_KEY_DOWN:
            rotate_y -= 10.0f;
            myDisplay();
            break;
        case GLUT_KEY_RIGHT:
            rotate_x -= 10.0f;
            myDisplay();
            break;
        case GLUT_KEY_LEFT:
            rotate_x += 10.0f;
            myDisplay();
            break;
    }
}




/* Uniform Subdivision. */
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
    Vector p = D * (1.0f - u) + E * u;
    Vector dPdu = (E - D) * 3.0f; // derivative
    
    vector<Vector> result;
    result.push_back(p); result.push_back(dPdu);
    return result;
}

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

/* populates triangle list */
void makeTriangles() {
    for (int i = 0; i < numSubdivisions; i++) {
        for (int j = 0; j < numSubdivisions; j++) {
            Triangle triangle1 = Triangle(points_to_Render[i][j], points_to_Render[i][j+1],
                                          points_to_Render[i+1][j]);
            Triangle triangle2 = Triangle(points_to_Render[i+1][j], points_to_Render[i][j+1],
                                          points_to_Render[i+1][j+1]);
            list_triangles.push_back(triangle1); list_triangles.push_back(triangle2);
        }
    }
    
    //reset all the lists
    points_to_Render = vector<vector<Vector> >(); curlevel = vector<Vector>();
}

/* Saves the point in structure to be converted to triangles then rendered.*/
void savesurfacepointandnormal(Vector p) {
    if (curlevel.size() == numSubdivisions + 1) {
        points_to_Render.push_back(curlevel);
        curlevel = vector<Vector>();
    }
    curlevel.push_back(p);
}

/*Uniform Subdivision.*/
void subdividePatchUniform(SurfacePatch sp, float step) {
    //compute how many subdivisions there # are for this step size
    int numdiv = ((1 + 0.005f) / step);
    numSubdivisions = numdiv;
    
    //for each parametric value of u
    for (int iu = 0; iu < numdiv+1; iu++) {
        float u = iu * step;
        
        // for each parametric value of v
        for (int iv = 0; iv < numdiv+1; iv++) {
            float v = iv * step;
            
            // evaluate surface
            vector<Vector> result;
            result = bezpatchinterp(sp, u, v);
            Vector p = result[0];
            Vector n = result[1];
            p.makeNormal(n);
            savesurfacepointandnormal(p);
        }
    }
}

/*Uniform Subdivision.*/
void subdividePatchAdaptive(SurfacePatch sp, float error) {
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
    // Parse the bezier file or OBJ file
    if(argc >= 2) {
        parse_input(argv[1]);
        step_size = (float) atof(argv[2]);
        
        if (argc == 4) {
            if (strcmp(argv[3], "-a") == 0) {
                isAdaptive = true;
            }
        }
    }

    cout << "Step Size = " << step_size << endl;
    
    if(!isAdaptive) {
    // Given a patch preform uniform subdisision
        for (int i = 0; i < list_of_SPatches.size(); i++) {
            list_of_SPatches[i].makeCurves();
            
            subdividePatchUniform(list_of_SPatches[i], step_size);
            
            points_to_Render.push_back(curlevel);
            makeTriangles();
        }
    }
    // Adaptive subdivision
    else {
        for (int i = 0; i < list_of_SPatches.size(); i++) {
            list_of_SPatches[i].makeCurves();
            
            subdividePatchAdaptive(list_of_SPatches[i], step_size);
            
            points_to_Render.push_back(curlevel);
            makeTriangles();
        }
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
    
    //shading
    glShadeModel(GL_FLAT);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    
    float diffuse0[]={1.0, 0.0, 0.0, 1.0};
    float ambient0[]={1.0, 0.0, 0.0, 1.0};
    float specular0[]={1.0, 0.0, 0.0, 1.0};
    float light0_pos[]={-1.0, -2.0, -3,0, -1.0};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
    
    glEnable(GL_LIGHT0);
    
    initScene();							// quick function to set up scene
    glutDisplayFunc(myDisplay);			// function to run when its time to draw something
    glutReshapeFunc(myReshape);       // function to run when the window gets resized
    glutKeyboardFunc(keyBoardFunc);
    glutSpecialFunc(processSpecialKeys);
    
    
    glutMainLoop();							// infinite loop that will keep drawing and resizing
    // and whatever else
    return 0;
}







