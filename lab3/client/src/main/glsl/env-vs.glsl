#version 300 es

in vec4 vertexPosition; //#vec4# A four-element vector [x,y,z,w].; We leave z and w alone.; They will be useful later for 3D graphics and transformations. #vertexPosition# attribute fetched from vertex buffer according to input layout spec
in vec4 vertexTexCoord;


uniform struct{
    mat4 rayDirMatrix;
} camera;


out vec4 rayDir;

void main(void) {
    gl_Position = vec4(vertexPosition.xy, 0.99999, 1.0);
    rayDir = vertexPosition * camera.rayDirMatrix;
}