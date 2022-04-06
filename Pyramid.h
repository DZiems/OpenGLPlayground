#pragma once
#include <cmath>
#include <glm\glm.hpp>

class Pyramid {
private:
	void init();
	const int numRepeatedTextures;
public:
	const float size;
	glm::vec3 pos;
	float vertices[54];
	float texCoords[36];
	Pyramid();
	Pyramid(float size, glm::vec3 pos);
	Pyramid(int numRT, float size, glm::vec3 pos);
};