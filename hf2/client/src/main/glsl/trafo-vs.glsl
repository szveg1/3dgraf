#version 300 es
in vec4 vertexPosition;
in vec4 vertexTexCoord;
out vec4 tex;
out vec4 modelPosition;

uniform struct {
	mat4 modelMatrix;
    mat4 texMatrix;
} gameObject;

uniform struct {
    mat4 viewProjMatrix;
} camera;

void main(void) {
    tex = vertexTexCoord * gameObject.texMatrix;
    modelPosition = vertexPosition;
    gl_Position = vertexPosition * gameObject.modelMatrix * camera.viewProjMatrix;
  //LABTODO: camera trafo
}
