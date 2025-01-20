#version 300 es 
precision highp float;
precision highp usampler2D;
precision highp int;

in vec2 texCoord; // passed from VS

uniform struct {
    usampler2D skeletonTexture;
    sampler2D riggingTexture;
    sampler2D nodeQTexture;
    sampler2D nodeTTexture;
    float nNodes;
    float nBones;
} mesh;

struct DualQuat {
    vec4 q;
    vec4 t;
};

vec4 qmul(vec4 q1, vec4 q2) {
    vec4 r;

    r.w = q1.w * q2.w - dot(q1.xyz, q2.xyz);
    r.xyz = q1.w * q2.xyz + q2.w * q1.xyz + cross(q1.xyz, q2.xyz);

    return r;
}

DualQuat dqmul(DualQuat q, DualQuat r) {
    DualQuat dq;
    dq.q = qmul(q.q, r.q);
    dq.t = qmul(q.q, r.t) + qmul(q.t, r.q);
    return dq;
}

layout (location = 0) out vec4 fragQ;
layout (location = 1) out vec4 fragT;

void main() {
    float iBone = texCoord.x * mesh.nBones;

    DualQuat dq;

    dq.q = texture(mesh.riggingTexture,
                   vec2((iBone * 2.0 - 0.25) / mesh.nBones / 2.0, 0.5));

    dq.t = texture(mesh.riggingTexture,
                   vec2((iBone * 2.0 + 0.25) / mesh.nBones / 2.0, 0.5));

    uvec4 ns = texture(mesh.skeletonTexture,
                       vec2(iBone / mesh.nBones, 0.5));

    for(int i = 0; i < 4; i++){
        uint iNode = (ns.x >> i * 8) & 0xffu;
        if(iNode != 0xffu) {
            DualQuat s;
            s.q = texture(mesh.nodeQTexture, vec2(texCoord.y, (mesh.nNodes - 0.5 - float(iNode)) / mesh.nNodes));
            s.t = texture(mesh.nodeTTexture, vec2(texCoord.y, (mesh.nNodes - 0.5 - float(iNode)) / mesh.nNodes));
            dq = dqmul(s, dq);
        }
    }

    for(int i = 0; i < 4; i++){
        uint iNode = (ns.y >> i * 8) & 0xffu;
        if(iNode != 0xffu) {
            DualQuat s;
            s.q = texture(mesh.nodeQTexture, vec2(texCoord.y, (mesh.nNodes - 0.5 - float(iNode)) / mesh.nNodes));
            s.t = texture(mesh.nodeTTexture, vec2(texCoord.y, (mesh.nNodes - 0.5 - float(iNode)) / mesh.nNodes));
            dq = dqmul(s, dq);
        }
    }

    for(int i = 0; i < 4; i++){
        uint iNode = (ns.z >> i * 8) & 0xffu;
        if(iNode != 0xffu) {
            DualQuat s;
            s.q = texture(mesh.nodeQTexture, vec2(texCoord.y, (mesh.nNodes - 0.5 - float(iNode)) / mesh.nNodes));
            s.t = texture(mesh.nodeTTexture, vec2(texCoord.y, (mesh.nNodes - 0.5 - float(iNode)) / mesh.nNodes));
            dq = dqmul(s, dq);
        }
    }

    for(int i = 0; i < 4; i++){
        uint iNode = (ns.w >> i * 8) & 0xffu;
        if(iNode != 0xffu) {
            DualQuat s;
            s.q = texture(mesh.nodeQTexture, vec2(texCoord.y, (mesh.nNodes - 0.5 - float(iNode)) / mesh.nNodes));
            s.t = texture(mesh.nodeTTexture, vec2(texCoord.y, (mesh.nNodes - 0.5 - float(iNode)) / mesh.nNodes));
            dq = dqmul(s, dq);
        }
    }


    fragQ = dq.q; //dq.q;
    fragT = dq.t; //dq.t;
    return;
}