#version 300 es

in vec4 vertexPosition;
in vec2 vertexTexCoord;

out vec2 tex;

mat4 scale = mat4   (0.5,0,0,0,
                    0,0.5,0,0,
                    0,0,0.5,0,
                    0,0,0,1);

uniform struct{
    mat4 modelMatrix;
    vec4 color;
} gameObject;

void main(void) {
  gl_Position = vertexPosition * gameObject.modelMatrix * scale; //#gl_Position# built-in output, required
  tex = vertexTexCoord + gameObject.color.xy;
}