#include "../include/MarchingCubes.h"

#define FRONT_TOP_LEFT     128
#define FRONT_TOP_RIGHT     64
#define BACK_TOP_RIGHT      32
#define BACK_TOP_LEFT       16
#define FRONT_BOTTOM_LEFT    8
#define FRONT_BOTTOM_RIGHT   4
#define BACK_BOTTOM_RIGHT    2
#define BACK_BOTTOM_LEFT     1

// struct to hold x, y, and z coordinates of a vertex
struct Vertex {
	float x, y, z;
};

// the marching cubes algorithm
std::vector<float> marching_cubes(
	std::function<float(float, float, float)> f,
	float isoValue,
	float min,
	float max,
	float stepSize)
{

	// define the list of vertices to return
	std::vector<float> verticesList;

	// loop over the grid
	for (float x = min; x < max; x += stepSize) {
		for (float y = min; y < max; y += stepSize) {
			for (float z = min; z < max; z += stepSize) {

				// create array to hold positions of all 8 vertices of the cube
				Vertex positions[8] = {
					{x, y, z},
					{x + stepSize, y, z},
					{x + stepSize, y + stepSize, z},
					{x, y + stepSize, z},
					{x, y, z + stepSize},
					{x + stepSize, y, z + stepSize},
					{x + stepSize, y + stepSize, z + stepSize},
					{x, y + stepSize, z + stepSize}
				};

				// create array to hold floats representing the vertices (scalar field)
				float scalars[8] = { 0 };

				// compute the scalar field values of the cube's 8 vertices
				for (size_t i = 0; i < 8; i++) {
					scalars[i] = f(positions[i].x, positions[i].y, positions[i].z);
				}

				// determine the case of the cube from the scalar values
				int theCase = 0;
				
				if (scalars[0] < isoValue) {
					theCase |= BACK_BOTTOM_LEFT;
				}
				if (scalars[1] < isoValue) {
					theCase |= BACK_BOTTOM_RIGHT;
				}
				if (scalars[2] < isoValue) {
					theCase |= FRONT_BOTTOM_RIGHT;
				}
				if (scalars[3] < isoValue) {
					theCase |= FRONT_BOTTOM_LEFT;
				}
				if (scalars[4] < isoValue) {
					theCase |= BACK_TOP_LEFT;
				}
				if (scalars[5] < isoValue) {
					theCase |= BACK_TOP_RIGHT;
				}
				if (scalars[6] < isoValue) {
					theCase |= FRONT_TOP_RIGHT;
				}
				if (scalars[7] < isoValue) {
					theCase |= FRONT_TOP_LEFT;
				}

				// search the lookup table for the case to get the edges (basically indices for vertTable which make up triangles)
				const int* caseEdges = marching_cubes_lut[theCase];

				// loop through the edges (indices of vertices for triangles)
				for (size_t i = 0; i < 16; i++) {
					// ignore -1 (padding)
					if (caseEdges[i] != -1) {
						// add the triangle's vertices to the return list
						verticesList.push_back(x + (stepSize * vertTable[caseEdges[i]][0]));
						verticesList.push_back(y + (stepSize * vertTable[caseEdges[i]][1]));
						verticesList.push_back(z + (stepSize * vertTable[caseEdges[i]][2]));
					}
				}
			}
		}
	}
	return verticesList;
}
