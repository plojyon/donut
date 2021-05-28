#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 20
#define PI 3.14159265
#define TWOPI 6.28

typedef struct _Vector {
	int dim;
	double* data;
} Vector;

typedef struct _Matrix {
	int w;
	int h;
	double** data;
} Matrix;

void printVector(Vector v) {
	printf("(");
	for (int i = 0; i < v.dim-1; i++) {
		printf("%f, ", v.data[i]);
	}
	printf("%f)\n", v.data[v.dim-1]);
}

void printMatrix(Matrix m) {
	printf("[\n");
	for (int i = 0; i < m.w; i++) {
		for (int j = 0; j < m.h; j++) {
			printf("%f ", m.data[i][j]);
		}
		printf("\n");
	}
	printf("]\n");
}

Matrix makeMatrix(int h, int w) {
	Matrix* m = malloc(sizeof(Matrix));
	m->w = w;
	m->h = h;
	m->data = malloc(w * sizeof(double*));
	for (int i = 0; i < w; i++) {
		m->data[i] = malloc(h * sizeof(double));
	}
	return *m;
}

Matrix rotation3d(char axis, double angle) {
	Matrix m = makeMatrix(3, 3);
	switch (axis) {
		case 'x':
			m.data[0][0] = 1;
			m.data[0][1] = 0;
			m.data[0][2] = 0;
			m.data[1][0] = 0;
			m.data[1][1] = cos(angle);
			m.data[1][2] = -sin(angle);
			m.data[2][0] = 0;
			m.data[2][1] = sin(angle);
			m.data[2][2] = cos(angle);
			break;
		case 'y':
			m.data[0][0] = cos(angle);
			m.data[0][1] = 0;
			m.data[0][2] = sin(angle);
			m.data[1][0] = 0;
			m.data[1][1] = 1;
			m.data[1][2] = 0;
			m.data[2][0] = -sin(angle);
			m.data[2][1] = 0;
			m.data[2][2] = cos(angle);
			break;
		case 'z':
			m.data[0][0] = cos(angle);
			m.data[0][1] = -sin(angle);
			m.data[0][2] = 0;
			m.data[1][0] = sin(angle);
			m.data[1][1] = cos(angle);
			m.data[1][2] = 0;
			m.data[2][0] = 0;
			m.data[2][1] = 0;
			m.data[2][2] = 1;
			break;
	}
	return m;
}

void transform(Vector* p, Matrix m) {
	// p->dim == m.h
	Vector res = { .dim = p->dim, .data = malloc(p->dim * sizeof(double)) };
	for (int i = 0; i < p->dim; i++) {
		res.data[i] = 0;
		for (int j = 0; j < m.w; j++) {
			res.data[i] += m.data[i][j] * p->data[j];
		}
	}
	for (int i = 0; i < p->dim; i++) {
		p->data[i] = res.data[i];
	}
	free(res.data);
}

char getSymbol(int distance) {
	if (distance < 0) return 'X';
	switch (distance) {
		case 0: return '#';
		case 1: return '@';
		case 2: return '&';
		case 3: return '%';
		case 4: return '$';
		case 5: return '=';
		case 6: return '*';
		case 7: return '~';
		case 8: return ';';
		case 9: return ':';
		case 10: return '.';
		case 11: return '`';
		default: return ' ';
	}
}

void show(Vector* points, int count) {
	for (int y = -SCREEN_HEIGHT; y < SCREEN_HEIGHT; y++) {
		for (int x = -SCREEN_WIDTH; x < SCREEN_WIDTH; x++) {
			int closest = 1000000; // probably won't have a depth more than this
			for (int k = 0; k < count; k++) {
				if (round(points[k].data[0]) == x && round(points[k].data[1]) == y) {
					int dist = round(points[k].data[2]);
					if (closest > dist) closest = dist;
				}
			}
			printf("%c", getSymbol(closest));
		}
		printf("\n");
	}
}

int main(int argc, char** argv) {
	// DEMO: basic debug stuffs
	/*
	Vector a = { .dim = 3, .data = malloc(3*sizeof(int)) };
	Matrix m = makeMatrix(3, 3);
	printVector(a);
	printMatrix(m);
	Matrix project = makeMatrix(3, 2);
	Matrix r = rotation3d('y', PI/2);
	Vector a = { .dim = 3, .data = malloc(3*sizeof(int)) };
	a.data[0] = 0;
	a.data[1] = 1;
	a.data[2] = 0;
	transform(&a, r);
	printVector(a);
	return 0;
	*/

	// DEMO: spinny lines and shit
	/*
	const int POINT_COUNT = 20;
	Vector points[POINT_COUNT];
	for (int i = 0; i < POINT_COUNT; i++) {
		Vector p = { .dim = 3, .data = malloc(3*sizeof(int)) };
		points[i] = p;
	}
	double t = 0;
	while (true) {
		Matrix rotation = rotation3d('x', t);
		for (int i = 0; i < POINT_COUNT/2; i++) {
			points[i].data[0] = i;
			points[i].data[1] = POINT_COUNT/2-1;
			points[i].data[2] = i;
			transform(&points[i], rotation);
		}
		Matrix rotation2 = rotation3d('z', t);
		for (int i = POINT_COUNT/2; i < POINT_COUNT; i++) {
			points[i].data[0] = i - POINT_COUNT/2;
			points[i].data[1] = 0;
			points[i].data[2] = 0;
			transform(&points[i], rotation2);
		}

		show(points, POINT_COUNT);
		printf("time is %f. sin(t) = %f, cos(t) = %f\n", t, sin(t), cos(t));
		usleep(200*1000);
		t += 0.1;
	}
	*/

	Matrix rotationX = rotation3d('x', 0.1);
	Matrix rotationY = rotation3d('y', 0.1);
	Matrix rotationZ = rotation3d('z', 0.1);

	const int INNER_POINTS = 30;
	const int OUTER_POINTS = 20;
	const int inner_radius = 10;
	const int outer_radius = 3;
	Vector inner[INNER_POINTS];
	Vector outer[OUTER_POINTS * INNER_POINTS];
	for (int i = 0; i < INNER_POINTS; i++) {
		Vector p = { .dim = 3, .data = malloc(3*sizeof(int)) };
		inner[i] = p;
		inner[i].data[0] = cos(i * TWOPI/INNER_POINTS)*inner_radius;
		inner[i].data[1] = sin(i * TWOPI/INNER_POINTS)*inner_radius;
		inner[i].data[2] = 0;
		for (int j = 0; j < OUTER_POINTS; j++) {
			int offset = OUTER_POINTS * i + j;
			Vector r = { .dim = 3, .data = malloc(3*sizeof(int)) };
			outer[offset] = r;
			outer[offset].data[0] = cos(j * TWOPI/INNER_POINTS)*outer_radius;
			outer[offset].data[1] = 0;
			outer[offset].data[2] = sin(j * TWOPI/INNER_POINTS)*outer_radius;

			outer[offset].data[0] += inner[i].data[0];
			outer[offset].data[1] += inner[i].data[1];
			outer[offset].data[2] += inner[i].data[2];
		}
	}

	while (true) {
		//show(inner, INNER_POINTS);
		show(outer, OUTER_POINTS * INNER_POINTS);
		for (int i = 0; i < INNER_POINTS * OUTER_POINTS; i++) {
			transform(&outer[i], rotationX);
			transform(&outer[i], rotationY);
		}
		usleep(100000);
	}
	return 0;
	//*/
}
