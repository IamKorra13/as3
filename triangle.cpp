#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm> 

using namespace std;

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