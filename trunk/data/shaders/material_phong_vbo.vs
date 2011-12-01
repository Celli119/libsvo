#version 120

/* A basic phong shader*/

varying vec3 viewDirection;
varying vec3 lightDirection;
varying vec3 normal;

void main( void )
{
  vec4 fvObjectPosition = gl_ModelViewMatrix * gl_Vertex;
  gl_Position = gl_ProjectionMatrix * fvObjectPosition;

  gl_FrontColor = gl_Color;

  vec3 fvEyePosition = -fvObjectPosition.xyz;
  viewDirection  = fvEyePosition - fvObjectPosition.xyz;
  normal         = gl_NormalMatrix * gl_Normal;

  vec3 fvLightPosition = gl_LightSource[0].position.xyz;
  lightDirection       = fvLightPosition - fvObjectPosition.xyz;
}
