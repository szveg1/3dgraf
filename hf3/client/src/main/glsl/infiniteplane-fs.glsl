#version 300 es

precision highp float;

in vec4 texCoord;

out vec4 fragmentColor;

uniform struct{
    sampler2D colorTexture;
} material;

void main(void){
    fragmentColor = texture(material.colorTexture, texCoord.xy / 30.0 / texCoord.w);
}