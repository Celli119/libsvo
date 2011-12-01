
varying vec4 model_color;
varying vec4 pit_color;

//uniform float MSHLB_QUALITY_MIN;
//uniform float MSHLB_QUALITY_MAX;
//attribute float MSHLB_QUALITY;
float transition_center = 0.5;  // between -1 and 1
float transition_speed  = 0.2;   // between 0 constant and 1 binary.


varying float ao;
varying vec3 ViewDirection;
varying vec3 LightDirection;
varying vec3 Normal;
varying vec4 Cs;

void main(void)
{
//  gl_FrontColor = gl_Color;
  model_color   = gl_Color;
  pit_color     = gl_Color*0.5;

  // we compute a zero-centered clamping.
//  float max_abs_Q = max(abs(MSHLB_QUALITY_MIN), abs(MSHLB_QUALITY_MAX));
//  ao = (MSHLB_QUALITY+max_abs_Q)/(2.0*max_abs_Q);

//  ao = clamp(ao,0.0,1.0);
  ao = 1.0;


//  ao = ao*2.0 - 1.0; // Now it is in the -1 .. 1 range;
//  ao = ao+ transition_center;
  ao = atan(ao* pow(transition_speed,3.0))/(3.14159265/2.0); // now in the -1..1 range
  ao = ao/2.0 + 0.5;
  gl_Position = ftransform();
  ViewDirection  = -vec3(gl_ModelViewMatrix * gl_Vertex);
  LightDirection = gl_LightSource[0].position.xyz;
  Normal         = gl_NormalMatrix * gl_Normal;
  Cs = mix(pit_color,model_color,ao);
 }






