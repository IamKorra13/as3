#include "vector.h"
Vector::Vector() {
	x = 0;
	y = 0;
	z = 0;
	normal.push_back(0.0f); normal.push_back(0.0f); 
	normal.push_back(0.0f);
}

Vector::Vector(int ix, int iy, int iz) {
	x = (float) ix;
	y = (float) iy;
	z = (float) iz;
	normal.push_back(0.0f); normal.push_back(0.0f); 
	normal.push_back(0.0f);
	w = 0.0f;
}

Vector::Vector(float ix, float iy, float iz) {
	x = ix;
	y = iy;
	z = iz;
	normal.push_back(0.0f); normal.push_back(0.0f); 
	normal.push_back(0.0f);
	w = 0.0f;
}


Vector::Vector(float ix, float iy, float iz, float iw) {
	x = ix;
	y = iy;
	z = iz;
	w = iw;
}
void Vector::add(Vector v1, Vector v2) {
	x = v1.x + v2.x;
	y = v1.y + v2.y;
	z = v1.z + v2.z;
}

void Vector::subtract(Vector v1, Vector v2) {
	x = v1.x - v2.x;
	y = v1.y - v2.y;
	z = v1.z - v2.z;
}

void Vector::scalar_multiply(Vector v, float scalar) {
	x = v.x * scalar;
	y = v.y * scalar;
	z = v.z * scalar;
}

void Vector::scalar_divide(Vector v, float scalar) {
	x = v.x / scalar;
	y = v.y / scalar;
	z = v.z / scalar;
}

Vector Vector::operator+(const Vector& v) {
	Vector result;
	result.x = this->x + v.x;
	result.y = this->y + v.y;
	result.z = this->z + v.z;
	return result;
}

Vector Vector::operator-(const Vector& v) {
	Vector result;
	result.x = this->x - v.x;
	result.y = this->y - v.y;
	result.z = this->z - v.z;
	return result;
}

Vector Vector::operator*(const Vector& v) {
	Vector result;
	result.x = this->x * v.x;
	result.y = this->y * v.y;
	result.z = this->z * v.z;
	return result;
}

Vector Vector::operator*(float scalar) {
	Vector result;
	result.x = this->x * scalar;
	result.y = this->y * scalar;
	result.z = this->z * scalar;
	return result;
}

Vector Vector::operator/(const Vector& v) {
	Vector result;
	result.x = this->x / v.x;
	result.y = this->y / v.y;
	result.z = this->z / v.z;
	return result;
}

Vector Vector::operator/(float scalar) {
	Vector result;
	result.x = this->x / scalar;
	result.y = this->y / scalar;
	result.z = this->z / scalar;
	return result;
}

void Vector::normalize() {
	float magnitude = pow(x, 2) + pow(y, 2) + pow(z, 2);
	magnitude = sqrt(magnitude);
	
	if( magnitude > 0) {
		x = x / magnitude;
		y = y / magnitude;
		z = z / magnitude;
	}
}

float Vector::magnitude() {
    return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

void Vector::makeNormal(Vector n) {
	normal[0] = n.x; normal[1] = n.y; normal[2] = n.z;
}

float Vector::dot_product(Vector v) {
	return x * v.x + y * v.y + z * v.z;
}

Vector Vector::cross_product(Vector v) {
	return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

void Vector::print() {
	cout << "<" << x << ", " << y << ", " << z << ">  ";
}

float dot_product(Vector v1, Vector v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}