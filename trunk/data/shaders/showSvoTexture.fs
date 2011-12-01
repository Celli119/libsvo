#version 120

/*

author: 2008 by Felix Weizig
mail:   thesleeper@gmx.net

*/


uniform sampler2D svoTexture;
uniform float     numNodes;
uniform float     numAttribs;

void main(void)
{
  vec3 textureColor = texture2D(svoTexture, gl_TexCoord[0].st ).rgb;

  gl_FragColor = vec4(textureColor.r/numNodes,   // child position
                      textureColor.g/65536.0,    // masks pow(2,16)
                      textureColor.b/numAttribs, // attribs position
                      1.0);
}


