#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
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
	distance /= 10;
	distance += 7;
	if (distance < 0) return '@';
	switch (distance) {
		case 0: return '@';
		case 1: return '$';
		case 2: return '#';
		case 3: return '*';
		case 4: return '!';
		case 5: return '=';
		case 6: return ';';
		case 7: return ':';
		case 8: return '~';
		case 9: return '-';
		case 10: return ',';
		case 11: return '.';
		default: return ' ';
	}
}

void show(Vector* points, int count, Vector* normals, Vector lightSource) {
	for (int y = -SCREEN_HEIGHT; y < SCREEN_HEIGHT; y++) {
		for (int x = -SCREEN_WIDTH; x < SCREEN_WIDTH; x++) {
			// find the closest point whose coordinates round to (x, y)
			int closest_index = -1;
			for (int k = 0; k < count; k++) {
				if (round(points[k].data[0]) == x && round(points[k].data[1]) == y) {
					if (closest_index == -1) closest_index = k;
					int depth = round(points[k].data[2]);
					int min_depth = round(points[closest_index].data[2]);
					if (depth < min_depth) closest_index = k;
				}
			}
			if (closest_index != -1) {
				Vector towards_light = { .dim = 3, .data = malloc(3*sizeof(int))};
				towards_light.data[0] = lightSource.data[0] - normals[closest_index].data[0];
				towards_light.data[1] = lightSource.data[1] - normals[closest_index].data[1];
				towards_light.data[2] = lightSource.data[2] - normals[closest_index].data[2];
				int luminance = round(dot(normals[closest_index], towards_light));
				printf("%c", getSymbol(luminance));
				//printf("%c", getSymbol(round(points[closest_index].data[2])));
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
	lightSource.data[1] = 10;
	lightSource.data[2] = -10;

	Matrix rotationX = rotation3d('x', 0.3);
	Matrix rotationY = rotation3d('y', 0.1);
	Matrix rotationZ = rotation3d('z', 0.2);

	const int INNER_POINTS = 50;
	const int OUTER_POINTS = 50;
	const int POINTS = INNER_POINTS * OUTER_POINTS;
	const int inner_radius = 6;
	const int outer_radius = 6;

	Vector inner[INNER_POINTS];
	Vector outer[POINTS * 2];
	Vector* normals = &outer[POINTS];
	// append normals array at the end of the outer array,
	// so I only have to call transform() once

	for (int i = 0; i < INNER_POINTS; i++) {
		Matrix rotate = rotation3d('z', i * TWOPI/INNER_POINTS);
		Vector p = { .dim = 3, .data = malloc(3*sizeof(int)) };
		inner[i] = p;
		inner[i].data[0] = inner_radius;
		inner[i].data[1] = inner_radius;
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
		for (int i = 0; i < POINTS * 2; i++) {
			transform(&outer[i], rotationX);
			transform(&outer[i], rotationZ);
		}
		usleep(100000);
	}
	return 0;
}
