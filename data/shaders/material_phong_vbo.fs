#version 120

/* A basic phong shader*/

varying vec3 viewDirection;
varying vec3 lightDirection;
varying vec3 normal;

void main( void )
{
   vec3  fvLightDirection = normalize( lightDirection );
   vec3  fvNormal         = normalize( normal );

   float fNDotL           = dot( fvNormal, fvLightDirection );

   vec3  fvReflection     = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection );
   vec3  fvViewDirection  = normalize( viewDirection );
   float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );

   vec4  fvTotalAmbient   = gl_Color * gl_Color * gl_LightSource[0].ambient;
   vec4  fvTotalDiffuse   = gl_Color * gl_LightSource[0].diffuse * fNDotL;

   vec4 fvTotalSpecular = gl_Color
                           * gl_LightSource[0].specular
                           * pow(fRDotV, 40.0);

   gl_FragColor = ( fvTotalAmbient*1.4 + fvTotalDiffuse + fvTotalSpecular*0.15 );

}
