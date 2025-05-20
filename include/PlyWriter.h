#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

void writePLY(const std::vector<float>& vertices, const std::vector<float>& normals, std::string fileName);