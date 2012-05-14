#version 330
#extension GL_EXT_gpu_shader4 : enable
// svo data
//uniform samplerBuffer tbo_child_pointers;
//uniform samplerBuffer tbo_bitmasks;
//uniform samplerBuffer tbo_attribute_pointers;

// attributes
uniform samplerBuffer tbo_attributes;


in vec3 normal;
in vec4 color;

out vec4 out_Color;


void main(void)
{
//  vec4 attributeColor = texelFetchBuffer(tbo_attributes, int(gl_FragCoord.y*100+gl_FragCoord.x)*2+1);
  vec4 attributeColor = texelFetchBuffer(tbo_attributes, 0);

//	out_Color = vec4(normal, 1.0);
//	out_Color = color;
	out_Color = attributeColor;
}
