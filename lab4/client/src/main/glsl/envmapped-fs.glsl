#version 300 es 
precision highp float;


out vec4 fragmentColor;
//LABTODO: world space inputs
in vec4 worldPosition;
in vec4 worldNormal;

uniform struct{
  //LABTODO: uniform for environment
  samplerCube envmapTexture;
} material;

uniform struct{
  mat4 viewProjMatrix;
  //LABTODO: uniform for computing view direction
  vec3 position;
} camera;

//LABTODO: uniforms for light source data

float noise(vec3 r) {
  uvec3 s = uvec3(
    0x1D4E1D4E,
    0x58F958F9,
    0x129F129F);
  float f = 0.0;
  for(int i=0; i<16; i++) {
    vec3 sf =
    vec3(s & uvec3(0xFFFF))
    / 65536.0 - vec3(0.5, 0.5, 0.5);

    f += sin(dot(sf, r));
    s = s >> 1;
  }
  return f / 32.0 + 0.5;
}

vec3 noiseGrad(vec3 r) {
  uvec3 s = uvec3(
    0x1D4E1D4E,
    0x58F958F9,
    0x129F129F);
  vec3 f = vec3(0, 0, 0);
  for(int i=0; i<16; i++) {
    vec3 sf =
    vec3(s & uvec3(0xFFFF))
    / 65536.0 - vec3(0.5, 0.5, 0.5);

    f += cos(dot(sf, r)) * sf;
    s = s >> 1;
  }
  return f;
}



void main(void) {
  vec3 normal = normalize(worldNormal.xyz) + 0.5 * noiseGrad(vec3(1.5f, 2.5f, 4.5f));
  vec3 x = worldPosition.xyz / worldPosition.w;
  vec3 viewDir = normalize(camera.position - x);
  fragmentColor = texture(material.envmapTexture, reflect(-viewDir, normal));
}

