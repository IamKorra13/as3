#include "curve.h"

Curve::Curve() {
	pt1 = Vector();
	pt2 = Vector();
	pt3 = Vector();
	pt4 = Vector();
}

void Curve::print() {
	pt1.print(); pt2.print(); pt3.print(); pt4.print();
	cout << endl;
	
}
