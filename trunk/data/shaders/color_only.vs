
/*

author: 2008 by Felix Weizig
mail:   thesleeper@gmx.net

*/


void main(void)
{
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
  gl_FrontColor = gl_Color;
}
