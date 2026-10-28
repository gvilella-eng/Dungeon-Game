#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec2 aMat;   // x = material, y = ao (0..255)
layout(location = 4) in vec2 aFlow;  // direcao de fluxo -1..1

uniform mat4 uVP;
uniform mat4 uModel;
uniform mat3 uNormalMat;

out vec3 vWorld;
out vec3 vNormal;
out vec4 vColor;
flat out int vMat;
out float vAo;
out vec2 vFlow;

void main()
{
    vec4 w = uModel * vec4(aPos, 1.0);
    vWorld = w.xyz;
    vNormal = normalize(uNormalMat * aNormal);
    vColor = aColor;
    vMat = int(aMat.x + 0.5);
    vAo = aMat.y / 255.0;
    vFlow = aFlow;
    gl_Position = uVP * w;
}
