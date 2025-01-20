#version 300 es
precision highp float;

in vec2 texCoord;
out vec4 fragmentColor;

uniform struct {
    sampler2D prevFrame;
    sampler2D currentFrame;
    float weight;
} mesh;

void main(void) {
    vec4 prevColor = texture(mesh.prevFrame, texCoord);
    vec4 currentColor = texture(mesh.currentFrame, texCoord);
    fragmentColor = mix(prevColor, currentColor, mesh.weight);
}
