#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm> 
#include "vector.h"

using namespace std;

class Triangle {
	public:
		Triangle();
		Triangle(Vector, Vector, Vector);
		Vector v1, v2, v3, normal;
};

Triangle::Triangle() {
	v1 = Vector(); v2 = Vector(); v3 = Vector(); 
	normal = Vector();
}

Triangle::Triangle(Vector iv1, Vector iv2, Vector iv3) {
	v1 = iv1; v2 = iv2; v3 = iv3;
}