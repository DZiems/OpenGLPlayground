#version 430
#define PI 3.1415926536
#define H_PI 1.5707963268
#define Q_PI 0.7853981634

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 vertNormal;
out vec2 tc;	//texture coordinate output to rasterizer for interpolation

//lights
out vec3 varyingNormal, varyingVertPos, 
varyingLightDir_pa, varyingLightDir_pb, 
varyingHalfVector_pa, varyingHalfVector_pb;

//shadow
out vec4 shadow_coord;

struct PositionalLight {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

//uniforms
uniform vec4 globalAmbient;
uniform PositionalLight light_pa;
uniform PositionalLight light_pb;
uniform Material material;
uniform int matType;		//discerning how much light is influence

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 shadow_mvpmatrix;

mat4 buildTranslate(float x, float y, float z);
mat4 buildRotateX(float rad);
mat4 buildRotateY(float rad);
mat4 buildRotateZ(float rad);
mat4 buildScale(float x, float y, float z);

layout (binding = 0) uniform sampler2D samp;
layout (binding = 1) uniform sampler2DShadow shTex_pa;
layout (binding = 2) uniform sampler2DShadow shTex_pb;
layout (binding = 3) uniform samplerCube sampcube;

void main(void)
{	
	varyingVertPos = (mv_matrix * vec4(vertPos,1.0)).xyz;
	varyingLightDir_pa = light_pa.position - varyingVertPos;
	varyingLightDir_pb = light_pb.position - varyingVertPos;
	varyingNormal = (norm_matrix * vec4(vertNormal,1.0)).xyz;

	//varyingLightAPower = lightAPower;
	
	varyingHalfVector_pa = (varyingLightDir_pa - varyingVertPos).xyz;

	varyingHalfVector_pb = (varyingLightDir_pb - varyingVertPos).xyz;

	shadow_coord = shadow_mvpmatrix * vec4(vertPos, 1.0);

	gl_Position = proj_matrix * mv_matrix * vec4(vertPos, 1.0); //declares a vec4 out of a vec3 and a 4th param
	tc = texCoord;
}



mat4 buildTranslate(float x, float y, float z)
{ mat4 trans = mat4(1.0, 0.0, 0.0, 0.0,
					0.0, 1.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					x, y, z, 1.0);
  return trans;
}
mat4 buildRotateX(float rad)
{ mat4 xrot = mat4(1.0, 0.0, 0.0, 0.0,
					0.0, cos(rad), -sin(rad), 0.0,
					0.0, sin(rad), cos(rad), 0.0,
					0.0, 0.0, 0.0, 1.0);
  return xrot;
}
mat4 buildRotateY(float rad)
{ mat4 yrot = mat4(cos(rad), 0.0, sin(rad),	0.0,
					0.0, 1.0, 0.0, 0.0,
					-sin(rad), 0.0, cos(rad), 0.0,
					0.0, 0.0, 0.0, 1.0);
  return yrot;
}
mat4 buildRotateZ(float rad)
{ mat4 zrot = mat4(cos(rad), -sin(rad),	0.0, 0.0,
					sin(rad), cos(rad), 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0, 0.0, 0.0, 1.0);
  return zrot;
}
mat4 buildScale(float x, float y, float z)
{ mat4 scale = mat4(x, 0.0, 0.0, 0.0,
					0.0, y, 0.0, 0.0,
					0.0, 0.0, z, 0.0,
					0.0, 0.0, 0.0, 1);
	return scale;
}
