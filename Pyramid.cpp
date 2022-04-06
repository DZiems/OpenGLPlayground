#include "Pyramid.h"

void Pyramid::init()
{
	float s = size / 2;
	float t = s * numRepeatedTextures;

	//front face
	vertices[0] = -s;
	vertices[1] = -s;
	vertices[2] = s;
	texCoords[0] = 0.0f;
	texCoords[1] = 0.0f;

	vertices[3] = s;
	vertices[4] = -s;
	vertices[5] = s;
	texCoords[2] = t;
	texCoords[3] = 0.0f;

	vertices[6] = -0.0f;
	vertices[7] = s;
	vertices[8] = 0.0f;
	texCoords[4] = t/2;
	texCoords[5] = t;

	//right face
	vertices[9] = s;
	vertices[10] = -s;
	vertices[11] = s;
	texCoords[6] = 0.0f;
	texCoords[7] = 0.0f;

	vertices[12] = s;
	vertices[13] = -s;
	vertices[14] = -s;
	texCoords[8] = t;
	texCoords[9] = 0.0f;

	vertices[15] = 0.0f;
	vertices[16] = s;
	vertices[17] = 0.0f;
	texCoords[10] = t/2;
	texCoords[11] = t;

	//back face
	vertices[18] = s;
	vertices[19] = -s;
	vertices[20] = -s;
	texCoords[12] = 0.0f;
	texCoords[13] = 0.0f;

	vertices[21] = -s;
	vertices[22] = -s;
	vertices[23] = -s;
	texCoords[14] = t;
	texCoords[15] = 0.0f;

	vertices[24] = 0.0f;
	vertices[25] = s;
	vertices[26] = 0.0f;
	texCoords[16] = t/2;
	texCoords[17] = t;

	//left face
	vertices[27] = -s;
	vertices[28] = -s;
	vertices[29] = -s;
	texCoords[18] = 0.0f;
	texCoords[19] = 0.0f;

	vertices[30] = -s;
	vertices[31] = -s;
	vertices[32] = s;
	texCoords[20] = t;
	texCoords[21] = 0.0f;

	vertices[33] = 0.0f;
	vertices[34] = s;
	vertices[35] = 0.0f;
	texCoords[22] = t / 2;
	texCoords[23] = t;

	//base a
	vertices[36] = -s;
	vertices[37] = -s;
	vertices[38] = -s;
	texCoords[24] = 0.0f;
	texCoords[25] = 0.0f;

	vertices[39] = s;
	vertices[40] = -s;
	vertices[41] = s;
	texCoords[26] = t;
	texCoords[27] = t;

	vertices[42] = -s;
	vertices[43] = -s;
	vertices[44] = s;
	texCoords[28] = 0.0f;
	texCoords[29] = t;

	//base b
	vertices[45] = s;
	vertices[46] = -s;
	vertices[47] = s;
	texCoords[30] = t;
	texCoords[31] = t;

	vertices[48] = -s;
	vertices[49] = -s;
	vertices[50] = -s;
	texCoords[32] = 0.0f;
	texCoords[33] = 0.0f;

	vertices[51] = s;
	vertices[52] = -s;
	vertices[53] = -s;
	texCoords[34] = t;
	texCoords[35] = 0.0f;
}

Pyramid::Pyramid() : numRepeatedTextures(4), size(2.0f)
{
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	init();
}

Pyramid::Pyramid(float size, glm::vec3 pos) : numRepeatedTextures(4), size(size), pos(pos)
{
	init();
}

Pyramid::Pyramid(int numRT, float size, glm::vec3 pos) : numRepeatedTextures(numRT), size(size), pos(pos)
{
	init();
}
