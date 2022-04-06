#include "GroundPlane.h"

void GroundPlane::init()
{
	vertices[0] = -length;
	vertices[1] = 0.0f;
	vertices[2] = -length;
	texCoords[0] = 0.0f;
	texCoords[1] = 0.0f;

	vertices[3] = length;
	vertices[4] = 0.0f;
	vertices[5] = length;
	texCoords[2] = length;
	texCoords[3] = length;

	vertices[6] = -length;
	vertices[7] = 0.0f;
	vertices[8] = length;
	texCoords[4] = 0.0f;
	texCoords[5] = length;

	vertices[9] = length;
	vertices[10] = 0.0f;
	vertices[11] = length;
	texCoords[6] = length;
	texCoords[7] = length;

	vertices[12] = -length;
	vertices[13] = 0.0f;
	vertices[14] = -length;
	texCoords[8] = 0.0f;
	texCoords[9] = 0.0f;

	vertices[15] = length;
	vertices[16] = 0.0f;
	vertices[17] = -length;
	texCoords[10] = length;
	texCoords[11] = 0.0f;

}

GroundPlane::GroundPlane() : length(10.0f), pos(glm::vec3(0.0f, 0.0f, 0.0f))
{
	init();
}

GroundPlane::GroundPlane(float length, glm::vec3 pos) : length(length), pos(pos)
{
	init();
}

/* 
in setupVertices():
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundplane.vertices), groundplane.vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundplane.texCoords), groundplane.texCoords, GL_STATIC_DRAW);
*/

/*
in display():

	mMat = glm::translate(glm::mat4(1.0f), groundplane.pos);
	mvMat = vMat * mMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniform1i(matType, 0);

	//pass vectors through
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//pass texture buffer through
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planeTexture);

	glDrawArrays(GL_TRIANGLES, 0, 6);
*/