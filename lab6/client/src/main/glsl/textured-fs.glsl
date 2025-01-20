#version 300 es

precision highp float;

in vec2 tex;
in vec3 normal;
out vec4 fragmentColor;

uniform struct {
	sampler2D colorTexture;
	vec2 texScale;
} material;

uniform struct{
	mat4 modelMatrix;
	vec2 texOffset;
} gameObject;

void main(void) {
  fragmentColor = texture(material.colorTexture, (tex + gameObject.texOffset) * material.texScale);
}