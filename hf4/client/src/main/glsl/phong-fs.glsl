#version 300 es 
precision highp float;

in vec4 worldPosition;
in vec4 worldNormal;
in vec4 worldTangent;
in vec4 worldBitangent;
in vec4 texCoord;

out vec4 fragmentColor;

uniform struct {
    mat4 viewProjMatrix;
    vec3 position;
} camera;

uniform struct {
    vec4 position;
    vec3 powerDensity;
} lights[8];

uniform struct {
    vec3 position;
    vec3 direction;
    vec3 powerDensity;
} spotlights[8];

uniform struct {
    sampler2D diffuseMap;
    sampler2D normalMap;
    vec3 ks;
    float shininess;
} material;


vec3 shade(
    vec3 powerDensity,
    vec3 normal,
    vec3 lightDir,
    vec3 viewDir,
    vec3 materialColor) {

    float cosa = clamp(dot(lightDir, normal), 0.0, 1.0);
    float cosa2 = clamp(dot(normal, viewDir), 0.0, 1.0);
    vec3 halfway = normalize(lightDir + viewDir);
    vec3 specular = pow(clamp(dot(normal, halfway), 0.0, 1.0), material.shininess) * material.ks;
    float maxblinn = cosa / max(cosa, cosa2);

    return
    powerDensity * cosa * materialColor
    +
    powerDensity * specular * maxblinn;
    ;
}

void main(void) {
    vec3 x = worldPosition.xyz / worldPosition.w;
    vec3 viewDir = normalize(camera.position - x);

    mat3 TBN = mat3(worldTangent.xyz, worldBitangent.xyz, worldNormal.xyz);
    vec3 tNormal = texture(material.normalMap, texCoord.xy).rgb;
    vec3 normal = normalize(TBN * tNormal);

    vec3 materialColor = texture(material.diffuseMap, texCoord.xy).rgb;

    vec3 radianceToEye = vec3(0, 0, 0);
        for (int i = 0; i < 1; i++) {
            vec3 lightDiff = lights[i].position.xyz -
            x * lights[i].position.w;
            float lightDistSquared = dot(lightDiff, lightDiff);
            vec3 lightDir = lightDiff / sqrt(lightDistSquared);
            vec3 powerDensity = lights[i].powerDensity / lightDistSquared;

            radianceToEye += shade(
                powerDensity,
                normal,
                lightDir,
                viewDir,
                materialColor
            );
        }

    for (int i = 0; i < 1; i++) {
        vec3 lightDir = normalize(spotlights[0].position - x);
        float cosTheta = dot(lightDir, normalize(spotlights[0].direction));
        vec3 powerDensity = spotlights[i].powerDensity * pow(cosTheta, 8.0);

        radianceToEye += shade(
            powerDensity,
            normal,
            lightDir,
            viewDir,
            materialColor
        );
    }
    fragmentColor = vec4(radianceToEye, 1);

}
