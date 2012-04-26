#version 330

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec4 in_color;


out vec3 normal;
out vec4 color;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform float time;


void main(void)
{
	//^= old ModelViewProjectionMatrix
	vec4 vertexPos = vec4(in_position, 1.0);

	gl_Position = ProjectionMatrix * ModelViewMatrix * vertexPos;
	normal      = (NormalMatrix * vec4(normalize(in_normal), 0.0)).xyz;
	color       = in_color;
}
