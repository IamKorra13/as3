#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm> 
#include "vector.h"

using namespace std;

class Curve {
	public:
		Curve();
		Vector pt1, pt2, pt3, pt4;
		void print();
};