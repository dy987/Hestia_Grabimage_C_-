#include "stdafx.h"
#include "affine.h"

void matrixcalc(double *matrix, Point_xy ori_Point[3], Point_xy obj_Point[3]) { //affine Çà·Ä °è»ê
	double a, b, c, d, e, f;
	double x1 = ori_Point[0].x;
	double y1 = ori_Point[0].y;

	double x2 = ori_Point[1].x;
	double y2 = ori_Point[1].y;

	double x3 = ori_Point[2].x;
	double y3 = ori_Point[2].y;

	double x_1 = obj_Point[0].x;
	double y_1 = obj_Point[0].y;

	double x_2 = obj_Point[1].x;
	double y_2 = obj_Point[1].y;

	double x_3 = obj_Point[2].x;
	double y_3 = obj_Point[2].y;

	a = ((x_1 - x_2)*(y1 - y3) - (x_1 - x_3)*(y1 - y2)) / ((x1 - x2)*(y1 - y3) - (x1 - x3)*(y1 - y2));
	b = ((x_1 - x_2) - a*(x1 - x2)) / (y1 - y2);
	e = x_1 - a*x1 - b*y1;

	c = ((y_1 - y_2)*(y1 - y3) - (y_1 - y_3)*(y1 - y2)) / ((x1 - x2)*(y1 - y3) - (x1 - x3)*(y1 - y2));
	d = ((y_1 - y_2) - c*(x1 - x2)) / (y1 - y2);
	f = y_1 - c*x1 - d*y1;

	matrix[3 * 0 + 0] = a;
	matrix[3 * 0 + 1] = b;
	matrix[3 * 0 + 2] = e;

	matrix[3 * 1 + 0] = c;
	matrix[3 * 1 + 1] = d;
	matrix[3 * 1 + 2] = f;

	matrix[3 * 2 + 0] = 0;
	matrix[3 * 2 + 1] = 0;
	matrix[3 * 2 + 2] = 1;

	/*printf("x1 : %d %d \n", (int)x1, (int)y1);
	printf("x2 : %d %d \n", (int)x2, (int)y2);
	printf("x3 : %d %d \n", (int)x3, (int)y3);
	

	printf("abc : %2.3lf %2.3lf %2.3lf \n", a, b, c);
	printf("def : %2.3lf %2.3lf %2.3lf \n", d, e, f);

	printf("x_1ÁÂÇ¥ : == %d  ", (int)(x1*a + b*y1 + e));
	printf("y_1ÁÂÇ¥ : == %d\n", (int)(x1*c + d*y1 + f));

	printf("x_2ÁÂÇ¥ : == %d  ", (int)(x2*a + b*y2 + e));
	printf("y_2ÁÂÇ¥ : == %d\n", (int)(x2*c + d*y2 + f));

	printf("x_3ÁÂÇ¥ : == %d  ", (int)(x3*a + b*y3 + e));
	printf("y_3ÁÂÇ¥ : == %d\n", (int)(x3*c + d*y3 + f));*/
}

Point_xy affinetransform_Point(Point_xy input, double *matrix) {
	Point_xy out;
	double a = matrix[3 * 0 + 0];
	double b = matrix[3 * 0 + 1];
	double e = matrix[3 * 0 + 2];

	double c = matrix[3 * 1 + 0];
	double d = matrix[3 * 1 + 1];
	double f = matrix[3 * 1 + 2];

	int x;
	int y;
	x = (int)(a*input.x + b*input.y + e);
	y = (int)(c*input.x + d*input.y + f);
	
	out.x = x;
	out.y = y;

	return out;
}