#version 300 es

in vec4 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
uniform struct{
	mat4 modelMatrix;
	vec2 texOffset;
} gameObject;

uniform struct{
	mat4 viewProjMatrix;
} camera;

out vec2 tex;
out vec3 normal;

void main(void) {
  gl_Position = vertexPosition * gameObject.modelMatrix * camera.viewProjMatrix;
  tex = vertexTexCoord;
  normal = vertexNormal;
}
