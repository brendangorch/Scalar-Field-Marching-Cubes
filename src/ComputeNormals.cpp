#include "../include/ComputeNormals.h"

#include <glm/glm.hpp>


// compute normals function
std::vector<float> compute_normals(const std::vector<float>& vertices) {
    // return list for normals
	std::vector<float> normals;

    // 9 consecutive floats in the vertices list represent the x, y, z coordinates of vertices of a triangle
	for (size_t i = 0; i < vertices.size(); i += 9) {
        // extract the 3 vertices for the current triangle
        glm::vec3 vertex1(vertices[i], vertices[i + 1], vertices[i + 2]);
        glm::vec3 vertex2(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        glm::vec3 vertex3(vertices[i + 6], vertices[i + 7], vertices[i + 8]);

        // compute two edge vectors of the triangle for the normal calculation
        glm::vec3 edge1 = vertex2 - vertex1;
        glm::vec3 edge2 = vertex3 - vertex2;

        // compute the normal using the cross product of the edge vectors and normalize it
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        // append the normal to the list for all 3 vertices of the triangle (9 total since vertices are represented as x, y, z in the list)
        for (int j = 0; j < 3; j++) {
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        }
	}
    return normals;
}
