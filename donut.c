#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#define SCREEN_WIDTH 13
#define SCREEN_HEIGHT 14
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

// maps x from [min_x, max_x] to the interval [min_y, max_y]
double map(double x, double min_x, double max_x, double min_y, double max_y) {
	x -= min_x;
	x /= max_x-min_x;
	x *= max_y-min_y;
	x += min_y;
	return x;
}

void printVector(Vector v) {
	printf("(");
	for (int i = 0; i < v.dim-1; i++) {
		printf("%5.2f, ", v.data[i]);
	}
	printf("%5.2f)", v.data[v.dim-1]);
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
	Matrix m;
	m.w = w;
	m.h = h;
	m.data = malloc(w * sizeof(double*));
	for (int i = 0; i < w; i++) {
		m.data[i] = malloc(h * sizeof(double));
	}
	return m;
}

void destroyMatrix(Matrix* m) {
	for (int i = 0; i < m->w; i++) {
		free(m->data[i]);
	}
	free(m->data);
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

double dot(Vector a, Vector b) {
	double sum = 0;
	for (int i = 0; i < a.dim; i++) {
		sum += a.data[i] * b.data[i];
	}
	return sum;
}

bool getnan(Vector v) {
	for (int i = 0; i < v.dim; i++) {
		if (isnan(v.data[i])) {
			return true;
		}
	}
	return false;
}

double length(Vector v) {
	double len = 0;
	for (int i = 0; i < v.dim; i++) {
		len += v.data[i]*v.data[i];
	}
	return sqrt(len);
}

// returns: [-1, 1]
double angle(Vector a, Vector b) {
	return dot(a, b) / (length(a)*length(b));
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

const char SYMBOLS[13] = {' ', '.', ',', '-', '~', ':', ';', '=', '!', '*', '#', '$', '@'};
const int SYMBOL_COUNT = 12; // do not count ' '
char getSymbol(double luminance) {
	luminance = map(luminance, -1, 1, 0, SYMBOL_COUNT);
	int lux = (int)round(luminance);
	if (lux > SYMBOL_COUNT) return '?'; // shouldn't happen
	if (lux < 0) return '?'; // shouldn't happen
	return SYMBOLS[lux];
}

void show(Vector* points, int count, Vector* normals, Vector lightSource) {
	for (int y = -SCREEN_HEIGHT; y < SCREEN_HEIGHT; y++) {
		for (int x = -SCREEN_WIDTH; x < SCREEN_WIDTH; x++) {
			// find the closest point whose coordinates round to (x, y)
			int closest_index = -1;
			for (int k = 0; k < count; k++) {
				if (round(points[k].data[0]) == x && round(points[k].data[1]) == y) {
					if (closest_index == -1) closest_index = k;
					double depth = points[k].data[2];
					double min_depth = points[closest_index].data[2];
					if (depth < min_depth) closest_index = k;
				}
			}
			if (closest_index != -1) {
				double luminance = -angle(normals[closest_index], lightSource);
				printf("%c", getSymbol(luminance));
			}
			else printf(" ");
		}
		printf("\n");
	}
}

int main(int argc, char** argv) {
	srand(time(NULL));

	Vector lightSource = { .dim = 3, .data = malloc(3*sizeof(int)) };
	lightSource.data[0] = 0;
	lightSource.data[1] = 1;
	lightSource.data[2] = 0;

	Matrix rotationX = rotation3d('x', 0.3);
	Matrix rotationY = rotation3d('y', 0.1);
	Matrix rotationZ = rotation3d('z', 0.2);

	const int INNER_POINTS = 60;
	const int OUTER_POINTS = 60;
	const int POINTS = INNER_POINTS * OUTER_POINTS;
	const int inner_radius = 8;
	const int outer_radius = 4;

	Vector inner[INNER_POINTS];
	Vector outer[POINTS * 2];
	Vector* normals = &outer[POINTS];
	// append normals array at the end of the outer array,
	// so I only have to call transform() once

	for (int i = 0; i < INNER_POINTS; i++) {
		Matrix rotate = rotation3d('z', i * TWOPI/INNER_POINTS);
		Vector p = { .dim = 3, .data = malloc(3*sizeof(int)) };
		inner[i] = p;
		inner[i].data[0] = 0;
		inner[i].data[1] = inner_radius;
		inner[i].data[2] = 0;
		for (int j = 0; j < OUTER_POINTS; j++) {
			int offset = OUTER_POINTS * i + j;
			Vector r = { .dim = 3, .data = malloc(3*sizeof(int)) };
			outer[offset] = r;
			outer[offset].data[0] = 0;
			outer[offset].data[1] = sin(j * TWOPI/OUTER_POINTS)*outer_radius;
			outer[offset].data[2] = cos(j * TWOPI/OUTER_POINTS)*outer_radius;

			outer[offset].data[0] += inner[i].data[0];
			outer[offset].data[1] += inner[i].data[1];
			outer[offset].data[2] += inner[i].data[2];

			// calculate the surface normal at this point
			// at each point, the surface normal is the vector
			// from the inner point to it's outer point
			Vector s = { .dim = 3, .data = malloc(3*sizeof(int)) };
			normals[offset] = s;
			normals[offset].data[0] = outer[offset].data[0] - inner[i].data[0];
			normals[offset].data[1] = outer[offset].data[1] - inner[i].data[1];
			normals[offset].data[2] = outer[offset].data[2] - inner[i].data[2];

			transform(&outer[offset], rotate);
			transform(&normals[offset], rotate);
		}
		transform(&inner[i], rotate);
		destroyMatrix(&rotate);
	}

	while (true) {
		show(outer, POINTS, normals, lightSource);

		for (int i = 0; i < 2*POINTS; i++) {
			transform(&outer[i], rotationX);
			transform(&outer[i], rotationZ);
		}
		fflush(stdout);
		usleep(100000);
	}
	return 0;
}
