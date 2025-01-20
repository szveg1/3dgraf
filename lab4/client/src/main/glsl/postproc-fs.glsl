#version 300 es
precision highp float;

out vec4 fragmentColor;
in vec4 texCoord;

uniform sampler2D rawTexture;

void main(void) {
  vec2 centeredCoord = texCoord.xy - 0.5;
  float dist = length(centeredCoord);
  float angle = dist > 0.5 ? 0.0 : dist * 2.0 * 3.14159;
  vec2 rotatedCoord = mat2(cos(angle), -sin(angle), sin(angle), cos(angle)) * centeredCoord;
  vec2 finalTexCoord = rotatedCoord + 0.5;
  fragmentColor = texture(rawTexture, finalTexCoord);
}