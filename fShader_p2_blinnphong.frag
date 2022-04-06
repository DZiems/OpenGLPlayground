#version 430

//lights
in vec3 varyingNormal;
in vec3 varyingVertPos;
in vec3 varyingLightDir_pa;
in vec3 varyingLightDir_pb;
in vec3 varyingHalfVector_pa;
in vec3 varyingHalfVector_pb;

//texture
in vec2 tc;		//interpolated incoming texture coordinate

//shadow
in vec4 shadow_coord;

out vec4 fragColor;

struct PositionalLight
{	vec4 ambient;  
	vec4 diffuse;  
	vec4 specular;  
	vec3 position;
};

struct Material
{	vec4 ambient;  
	vec4 diffuse;  
	vec4 specular;  
	float shininess;
};

//uniforms
uniform vec4 globalAmbient;
uniform PositionalLight light_pa;
uniform PositionalLight light_pb;
uniform Material material;
uniform int matType;	//discerning how much light is influence

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 shadow_mvpmatrix;

layout (binding = 0) uniform sampler2D samp;
layout (binding = 1) uniform sampler2DShadow shTex_pa;
layout (binding = 2) uniform sampler2DShadow shTex_pb;
layout (binding = 3) uniform samplerCube sampcube;


void main(void) 
{	
	if (matType == 0) {
		fragColor = texture(samp, tc);
		return;
	}
	else if (matType == 3) {
		vec3 r = -reflect(normalize(-varyingVertPos), normalize(varyingNormal));
		fragColor = 0.5 * texture(samp, tc) + 0.5 * texture(sampcube, r);
		return;
	}
	else if (matType == 4) {
		vec3 r = -reflect(normalize(-varyingVertPos), normalize(varyingNormal));
		fragColor = texture(sampcube, r);
		return;
	}
	
// normalize the light, normal, and view vectors:
	vec3 L_pa = normalize(varyingLightDir_pa);
	vec3 L_pb = normalize(varyingLightDir_pb);
	vec3 N = normalize(varyingNormal);
	vec3 V = normalize(-varyingVertPos);
	// halfway vector varyingHalfVector was computed in the vertex shader,
	// and interpolated prior to reaching the fragment shader.
	// It is copied into variable H here for convenience later.
	vec3 H_pa = normalize(varyingHalfVector_pa);
	vec3 H_pb = normalize(varyingHalfVector_pb);
	
	float inShadow = textureProj(shTex_pa, shadow_coord);

	float lightAPower = 0.5;
	float lightBPower = 1 - lightAPower;
	vec3 ambient = ((globalAmbient * material.ambient) + 
	(lightAPower * light_pa.ambient * material.ambient) + 
	(lightBPower * light_pb.ambient * material.ambient)).xyz;
	

	if (inShadow != 0.0) {
		vec3 diffuse = 
			(lightAPower * (light_pa.diffuse.xyz * material.diffuse.xyz) * max(dot(L_pa, N), 0.0)) +
			(lightBPower * (light_pb.diffuse.xyz * material.diffuse.xyz) *  max(dot(L_pb, N), 0.0));
		vec3 specular = 
			( lightAPower * ( light_pa.specular.xyz * material.specular.xyz *  pow(max(dot(H_pa,N),0.0), material.shininess * 3.0) ) ) +
			( lightBPower * ( light_pb.specular.xyz * material.specular.xyz *  pow(max(dot(H_pb,N),0.0), material.shininess * 3.0) ) );
		if (matType == 1) {
			fragColor = (0.5 * vec4((ambient + diffuse + specular), 1.0)) + (0.5 * texture(samp, tc));
		}
		else {
			fragColor =  vec4((ambient + diffuse + specular), 1.0);
		}
	}
	else {
		if (matType == 1) {
			fragColor = (0.5 * vec4((ambient), 1.0)) + (0.5 * texture(samp, tc));
		}
		else {
			fragColor = vec4((ambient), 1.0);
		}
	}
	

		
}
