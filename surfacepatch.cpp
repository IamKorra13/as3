#include "surfacepatch.h"

SurfacePatch::SurfacePatch() {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			cp[i][j] = Vector();
 		}
 	}
}

void SurfacePatch::print() {
	cout << "The Control Points: " << endl;
	for (int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			cp[i][j].print();
 		}
 		cout << endl;
	}
}

void SurfacePatch::makeCurves() {
	Curve u1, u2, u3, u4;
	Curve v1, v2, v3, v4;
	// V Curves - like the y axis
	v1.pt1 = cp[0][0]; v1.pt2 = cp[0][1]; v1.pt3 = cp[0][2]; v1.pt4 = cp[0][3]; 
	v2.pt1 = cp[1][0]; v2.pt2 = cp[1][1]; v2.pt3 = cp[1][2]; v2.pt4 = cp[1][3]; 
	v3.pt1 = cp[2][0]; v3.pt2 = cp[2][1]; v3.pt3 = cp[2][2]; v3.pt4 = cp[2][3]; 
	v4.pt1 = cp[3][0]; v4.pt2 = cp[3][1]; v4.pt3 = cp[3][2]; v4.pt4 = cp[3][3]; 

	// U Curves - like the x axis
	u1.pt1 = cp[0][0]; u1.pt2 = cp[1][0]; u1.pt3 = cp[2][0]; u1.pt4 = cp[3][0];
	u2.pt1 = cp[0][1]; u2.pt2 = cp[1][1]; u2.pt3 = cp[2][1]; u2.pt4 = cp[3][1]; 
	u3.pt1 = cp[0][2]; u3.pt2 = cp[1][2]; u3.pt3 = cp[2][2]; u3.pt4 = cp[3][2]; 
	u4.pt1 = cp[0][3]; u4.pt2 = cp[1][3]; u4.pt3 = cp[2][3]; u4.pt4 = cp[3][3];
	
	vcurves[0] = v1; vcurves[1] = v2; vcurves[2] = v3; vcurves[3] = v4;
	ucurves[0] = u1; ucurves[1] = u2; ucurves[2] = u3; ucurves[3] = u4;
}
