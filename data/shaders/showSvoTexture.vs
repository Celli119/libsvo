
/*

author: 2008 by Felix Weizig
mail:   thesleeper@gmx.net

*/


void main(void)
{
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

  gl_TexCoord[0].st = gl_MultiTexCoord0.xy;
}
