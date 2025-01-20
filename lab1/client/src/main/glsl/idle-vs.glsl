#version 300 es

in vec4 vertexPosition; //#vec4# A four-element vector [x,y,z,w].; We leave z and w alone.; They will be useful later for 3D graphics and transformations. #vertexPosition# attribute fetched from vertex buffer according to input layout spec

mat4 scale = mat4   (0.5,0,0,0,
                    0,0.5,0,0,
                    0,0,0.5,0,
                    0,0,0,1);

void main(void) {
  gl_Position = vertexPosition * scale; //#gl_Position# built-in output, required
}