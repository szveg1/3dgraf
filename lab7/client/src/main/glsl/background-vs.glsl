#version 300 es

in vec4 vertexPosition;
in vec2 vertexTexCoord;

uniform struct{
	mat4 viewProjMatrixInverse;
} camera;

out vec2 tex;

void main(void) {
  gl_Position = vertexPosition;
  tex = (vertexPosition * camera.viewProjMatrixInverse).xy;
  tex *= 0.03;
}
