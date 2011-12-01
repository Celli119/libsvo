#version 120

/*

author: 2008 by Felix Weizig
mail:   thesleeper@gmx.net

*/

uniform sampler2D         environmentMap;
//uniform sampler2DShadow shadow_map;
uniform float             reflection = 0.5;
uniform float             shininess  = 120.0;
uniform float             specular   = 1.0;

varying vec3 vPosition;
varying vec3 vNormal;

void main(void)
{

  vec4  color       =  gl_Color;
  vec4 reflectColor = texture2D(environmentMap, gl_TexCoord[2].st );

  vec3 vNormalNorm = normalize(vNormal);
  vec3 NEyeVec     = normalize(-vPosition);



  vec4 cAmbient   = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 cDiffuse   = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 cSpecColor = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 cReflection = vec4(0.0, 0.0, 0.0, 1.0);

  float shadow = 1.0;//shadow2DProj(shadow_map, gl_TexCoord[1]).z;

  for (int i=0; i!=1; ++i)
  {
    vec3 NLightVec = vec3(gl_LightSource[i].position.xyz) - vPosition;
    NLightVec      = normalize(NLightVec);

    vec3 NHalfVec  = normalize(NLightVec + NEyeVec);
    float ndotl    = max( dot(NLightVec,vNormalNorm), 0.0 );

//    float shadowSoft = (ndotl)*shadow;


//    cAmbient    += gl_LightSource[i].ambient * color;

    cDiffuse    += gl_LightSource[i].diffuse * color * ndotl /* * shadowSoft*/;

    cSpecColor += gl_LightSource[i].specular
                  * pow(max( dot(vNormalNorm, NHalfVec), 0.0 ), shininess)* specular /* * shadowSoft*/ ;

  }


  vec4 materialColor = cAmbient + cDiffuse;

  vec4 finalColor = mix(materialColor, reflectColor*1.5, reflection);

  finalColor.a = 1.0;

  gl_FragColor = finalColor + cSpecColor;
}


