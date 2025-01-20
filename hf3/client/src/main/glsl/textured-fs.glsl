#version 300 es

precision highp float;

out vec4 fragmentColor;
in vec4 texCoord;

in vec4 worldPosition;
in vec4 worldNormal;

uniform struct {
  sampler2D colorTexture;
} material;

uniform struct {
  vec4 position;
  vec3 powerDensity;
} lights[8];

vec3 shade(
  vec3 normal, vec3 lightDir,
  vec3 powerDensity, vec3 materialColor) {

  float cosa = max(dot(normal, lightDir), 0.0f);

  return materialColor * cosa * powerDensity;
}



void main(void) {
  fragmentColor = texture(material.colorTexture, texCoord.xy);
//  fragmentColor = vec4(texCoord.xyz,1);
//  for(int i = 0; i < 1; i++){
//    vec3 lightDir = lights[i].position.xyz;
//    vec3 powerDensity = lights[i].powerDensity;
//
//    fragmentColor.rgb += shade(worldNormal.xyz,
//                               lightDir,
//                               powerDensity / length(lightDir.xyz - worldPosition.xyz * lights[i].position.w),
//                               texture(material.colorTexture, texCoord.xy/texCoord.w).rgb
//    );
//  }

}