#version 300 es 
precision highp float;

out vec4 fragmentColor;
in vec4 tex;
//LABTODO: world space inputs
in vec4 worldPosition;
in vec4 worldNormal;

uniform struct{
  //LABTODO: uniform for surface color (diffuse reflection coeff kd)
  sampler2D colorTexture;
} material;

uniform struct{
  mat4 viewProjMatrix;
  //LABTODO: uniform for computing view direction
  vec3 position;
} camera;

//LABTODO: uniforms for light source data
uniform struct {
  vec4 position;
  vec3 powerDensity;
} lights[2];


vec3 shade(
  vec3 normal, vec3 lightDir,
  vec3 powerDensity, vec3 materialColor) {

  float cosa = max(dot(normal, lightDir), 0.0f);

  return materialColor * cosa * powerDensity;
}


void main(void) {
  fragmentColor = vec4(0,0,0,1);
  for(int i = 0; i < 2; i++){
    vec3 lightDir = lights[i].position.xyz;
    vec3 powerDensity = lights[i].powerDensity;

    //fragmentColor.rgb += shade(worldNormal.xyz, lightDir, powerDensity, texture(material.colorTexture, tex.xy/tex.w).rgb);
    fragmentColor.rgb += shade(worldNormal.xyz,
                               lightDir,
                               powerDensity / length(lightDir.xyz - worldPosition.xyz * lights[i].position.w),
                               texture(material.colorTexture, tex.xy/tex.w).rgb
    );
  }

}
