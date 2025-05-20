#include "../include/PlyWriter.h"

// function for writing ply file
void writePLY(const std::vector<float>& vertices, const std::vector<float>& normals, std::string fileName) {
	// create the file
	std::ofstream file(fileName + ".ply");

	// error check 
	if (!file) {
		std::cerr << "Error creating file!" << std::endl;
		return;
	}

	// write the header info
	file <<"ply" << std::endl;
	file <<"format ascii 1.0" << std::endl;
	file << "element vertex " << (vertices.size() / 3) << std::endl;
	file << "property float x" << std::endl;
	file << "property float y" << std::endl;
	file << "property float z" << std::endl;
	file << "property float nx" << std::endl;
	file << "property float ny" << std::endl;
	file << "property float nz" << std::endl;
	file << "element face " << (vertices.size() / 9) << std::endl;
	file << "property list uchar uint vertex_indices" << std::endl;
	file << "end_header" << std::endl;

	// loop through vertices and normals, add them to the file
	for (size_t i = 0; i < vertices.size(); i += 3) {
		file << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << " "
			<< normals[i] << " " << normals[i + 1] << " " << normals[i + 2] << std::endl;
	}

	// add faces to the file (triangle indices)
	for (size_t i = 0; i < (vertices.size() / 3); i += 3) {
		file << "3 " << i << " " << (i + 1) << " " << (i + 2) << std::endl;
	}

	// close file
	file.close();

	std::cout<< fileName << ".ply written successfully!" << std::endl;
}