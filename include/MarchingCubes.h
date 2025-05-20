#pragma once

#include <vector>
#include <functional>
#include <cmath>

#include "TriTable.h"

std::vector<float> marching_cubes(
	std::function<float(float, float, float)> f,
	float isoValue,
	float min,
	float max,
	float stepSize);
