
varying vec4 model_color;
varying vec4 pit_color;
float SpecularPower = 0.1;

varying float ao;
varying vec4 Cs;
varying vec3 ViewDirection;
varying vec3 LightDirection;
varying vec3 Normal;


vec4 Desaturate(vec3 color, float Desaturation)
{
   vec3 grayXfer = vec3(0.3, 0.59, 0.11);
   vec3 gray = vec3(dot(grayXfer, color));
   return vec4(mix(color, gray, Desaturation), 1.0);
}


void main(void)
{

   //phong lighting
   vec4 Ambient           = pit_color*0.1;
   vec4 Specular          = vec4(1.0,1.0,1.0,1.0)*ao*ao;
   vec3  fvLightDirection = normalize( LightDirection );
   vec3  fvNormal         = normalize( Normal );
   float fNDotL           = dot( fvNormal, fvLightDirection );

//   fNDotL = 1.0-fNDotL;

   vec3  fvReflection     = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection );
   vec3  fvViewDirection  = normalize( ViewDirection );
   float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );


   vec4  fvTotalDiffuse   = fNDotL * Cs;
   vec4  fvTotalSpecular  = Specular * ( pow( fRDotV, SpecularPower ) );
   vec4 color = (Ambient + fvTotalDiffuse) + fvTotalSpecular;

   gl_FragColor = color;
//   gl_FragColor = Desaturate(color.rgb, 0.5);
}





