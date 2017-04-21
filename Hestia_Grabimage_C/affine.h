#pragma once
#ifndef _AFFINE_H_
#define _AFFINE_H_

typedef struct MyStruct {
	int x, y;
	
	void setPoint(int in_x, int in_y){
		x = in_x;
		y = in_y;
	}
	void printPoint() {
		cout << "X : " << x << "  Y : " << y << endl;
	}

}Point_xy;

void matrixcalc(double *matrix, Point_xy ori_Point[3], Point_xy obj_Point[3]);
Point_xy affinetransform_Point(Point_xy input, double *matrix);

#endif