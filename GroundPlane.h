#pragma once
#include <cmath>
#include <glm\glm.hpp>

class GroundPlane {
private:
	void init();

public:
	const float length;
	glm::vec3 pos;
	float vertices[18];
	float texCoords[12];
	GroundPlane();
	GroundPlane(float length, glm::vec3 pos);
};