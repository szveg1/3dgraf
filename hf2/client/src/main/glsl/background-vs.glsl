#version 300 es
in vec4 vertexPosition;
in vec4 vertexTexCoord;
out vec4 tex;
out vec4 modelPosition;

uniform struct {
    mat4 modelMatrix;
} gameObject;

uniform struct {
    mat4 viewProjMatrixInverse;
} camera;

void main(void) {
    tex = vertexPosition * camera.viewProjMatrixInverse * 0.2;
    gl_Position = vertexPosition;
}
