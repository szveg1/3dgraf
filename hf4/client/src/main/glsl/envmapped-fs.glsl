#version 300 es
precision highp float;

out vec4 fragmentColor;
in vec4 texCoord;
in vec4 worldNormal;
in vec4 worldTangent;
in vec4 worldPosition;

uniform struct{
  mat4 viewProjMatrix;
  vec3 position;
} camera;

uniform struct {
  vec4 position;
  vec3 powerDensity;
} lights[8];

uniform struct{
  sampler2D environment;
  vec3 kr;
} material;

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

void main(void) {
  vec3 normal = normalize(worldNormal.xyz);
  vec3 bitangent = cross(normal, worldTangent.xyz);
  vec3 tangent = cross(bitangent, normal);

  vec3 x = worldPosition.xyz / worldPosition.w;
  vec3 viewDir = normalize(camera.position - x);

  vec2 noise = vec2(noise(vec3(1.4, 2.6, 5.3)), noise(vec3(4.3, 2.1, 0.5)));

  vec3 radianceToEye = vec3(0, 0, 0);
  radianceToEye +=
  material.kr *
  texture(material.environment, texCoord.xy + 0.1 * noise).rgb;

  fragmentColor = vec4(radianceToEye, 1);
}
