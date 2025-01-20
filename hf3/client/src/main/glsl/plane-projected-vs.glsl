#version 300 es
precision highp float;

in vec4 vertexPosition;

uniform struct {
    mat4 modelMatrix;
    mat4 modelMatrixInverse;
} gameObject;

uniform struct {
    mat4 shadowMatrix;
} scene;

uniform struct{
    mat4 viewProjMatrix;
    vec3 position;
} camera;

void main(void) {
    gl_Position = vertexPosition * gameObject.modelMatrix * scene.shadowMatrix * camera.viewProjMatrix;
}