
/*

author: 2008 by Felix Weizig
mail:   thesleeper@gmx.net

*/

varying vec3 vPosition;
varying vec3 vNormal;


void main(void)
{
  vec4 vertPosition = gl_ModelViewMatrix * gl_Vertex;
  vPosition         = (vertPosition / vertPosition.w).xyz;

  gl_TexCoord[0].st = gl_MultiTexCoord0.xy;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;


  gl_FrontColor = gl_Color;


  gl_ClipVertex  = gl_ModelViewMatrix * gl_Vertex;
  gl_TexCoord[1] = gl_TextureMatrix[1]*gl_ClipVertex;

  // Eye-space lighting
  vNormal = normalize(gl_NormalMatrix * gl_Normal).xyz;

  // env map
  {
    vec3 u = normalize( vertPosition.xyz );
    vec3 r = reflect( u, vNormal  );
    float m = 2.0 * sqrt( r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0) );
    gl_TexCoord[2].s = r.x/m + 0.5;
    gl_TexCoord[2].t = 1.0-(r.y/m + 0.5);
  }

}
