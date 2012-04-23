#version 330

in vec3 normal;
in vec4 color;


out vec4 out_Color;


void main(void)
{
	out_Color = color;
//	out_Color = vec4(normalize(normal), 1.0);
}
