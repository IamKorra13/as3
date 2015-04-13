#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

class Vector {
  public:
	float x, y, z, w;
	vector<float> normal;

	Vector();
	Vector(int, int, int);
	Vector(float, float, float);
	Vector(float, float, float, float);

    void add (Vector, Vector);
	void subtract (Vector, Vector);
	void scalar_multiply (Vector, float);
	void scalar_divide (Vector, float);
	void makeNormal(Vector);

    Vector operator+(const Vector& v);
	Vector operator-(const Vector& v);
	Vector operator*(const Vector& v);
	Vector operator*(float scalar);
	Vector operator/(const Vector& v);
	Vector operator/(float scalar);
	
	/*
	Vector add (Vector, Vector);
	Vector subtract (Vector, Vector);
	Vector scalar_multiply (Vector, float);
	Vector scalar_divide (Vector, float);
	*/
	void normalize();
	float dot_product(Vector);
	void print();
	Vector cross_product(Vector);
};

float dot_product(Vector, Vector);

#endif