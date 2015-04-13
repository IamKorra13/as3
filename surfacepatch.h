#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm> 

#include "vector.h"


using namespace std;

class SurfacePatch {
	public:
		SurfacePatch(); 
		void print();
		void makeCurves();
    
		Vector cp [4][4];
		Curve vcurves [4];
		Curve ucurves [4];
};