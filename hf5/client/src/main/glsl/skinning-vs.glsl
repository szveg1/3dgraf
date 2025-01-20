#version 300 es
in vec4 vertexPosition;
in vec4 vertexTexCoord;
in vec4 vertexNormal;
in vec4 blendIndices;
in vec4 blendWeights;
// LABTODO: attributes for rigging/blending
out vec4 texCoord; // passed to FS
out vec4 worldPosition;
out vec4 worldNormal;

struct DualQuat {
    vec4 q;
    vec4 t;
};

uniform struct {
    sampler2D boneQTexture;
    sampler2D boneTTexture;
/*iMSc: instance*/
    float nBones;
} multiMesh;

uniform struct {
    mat4 viewProjMatrix;
    vec3 position;
} camera;

void main(void) {
    DualQuat dq0;
    DualQuat dq1;
    DualQuat dq2;
    DualQuat dq3;
    dq0.q = texture(multiMesh.boneQTexture, vec2((float(blendIndices.x) + 0.5) / multiMesh.nBones, float(gl_InstanceID)));   // LABTODO: bone transformation
    dq0.t = texture(multiMesh.boneTTexture, vec2((float(blendIndices.x) + 0.5) / multiMesh.nBones, float(gl_InstanceID)));   // LABTODO: bone transformation

    // LABTODO: read all DQ data for smooth skinning
    dq1.q = texture(multiMesh.boneQTexture, vec2((float(blendIndices.y) + 0.5) / multiMesh.nBones, float(gl_InstanceID)));
    dq1.t = texture(multiMesh.boneTTexture, vec2((float(blendIndices.y) + 0.5) / multiMesh.nBones, float(gl_InstanceID)));
    dq2.q = texture(multiMesh.boneQTexture, vec2((float(blendIndices.z) + 0.5) / multiMesh.nBones, float(gl_InstanceID)));
    dq2.t = texture(multiMesh.boneTTexture, vec2((float(blendIndices.z) + 0.5) / multiMesh.nBones, float(gl_InstanceID)));
    dq3.q = texture(multiMesh.boneQTexture, vec2((float(blendIndices.w) + 0.5) / multiMesh.nBones, float(gl_InstanceID)));
    dq3.t = texture(multiMesh.boneTTexture, vec2((float(blendIndices.w) + 0.5) / multiMesh.nBones, float(gl_InstanceID)));
    // LABTODO: fix podality
    vec3 podality = vec3(
    (dot(dq0.q, dq1.q) >= 0.0) ? 1.0 : -1.0,
    (dot(dq0.q, dq2.q) >= 0.0) ? 1.0 : -1.0,
    (dot(dq0.q, dq3.q) >= 0.0) ? 1.0 : -1.0);

    vec4 w = blendWeights;
    w.w = 1.0 - dot(w.xyz, vec3(1, 1, 1));
    w.yzw *= podality;

    // LABTODO: blending for smooth skinning
    DualQuat dq;
    dq.q = dq0.q * w.x + dq1.q * w.y + dq2.q * w.z + dq3.q * w.w;
    dq.t = dq0.t * w.x + dq1.t * w.y + dq2.t * w.z + dq3.t * w.w;
    // LABTODO: normalize dq
    dq.q = dq.q / length(dq.q);
    dq.t = dq.t / length(dq.q);


    // rotate with dq
    vec3 blendedPos = vertexPosition.xyz + 2.0 * cross(dq.q.xyz,
                                                       cross(dq.q.xyz, vertexPosition.xyz) +
                                                       dq.q.w * vertexPosition.xyz);
    // translate by dq
    blendedPos += 2.0 * (dq.q.w * dq.t.xyz - dq.t.w * dq.q.xyz + cross(dq.q.xyz, dq.t.xyz));
    blendedPos += vec3(float(gl_InstanceID) * 100.0, 0, 0);
    // iMSc: offset by instance

    worldPosition = vec4(blendedPos, 1);
    gl_Position = worldPosition * camera.viewProjMatrix;
    texCoord = vertexTexCoord;
    // rotate by dq
    worldNormal.xyz = vertexNormal.xyz + 2.0 * cross(dq.q.xyz, cross(dq.q.xyz, vertexNormal.xyz) + dq.q.w * vertexNormal.xyz);
}
