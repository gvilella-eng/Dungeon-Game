#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aSize;
layout(location = 2) in vec4 aColor;
uniform mat4 uVP;
uniform float uProjScale;   // altura_px * proj[1][1] / 2
uniform vec3 uCamPos;
out vec4 vColor;
out float vDist;
out vec3 vWorld;
void main()
{
    gl_Position = uVP * vec4(aPos, 1.0);
    vDist = length(aPos - uCamPos);
    gl_PointSize = clamp(aSize * uProjScale / max(gl_Position.w, 0.1), 1.0, 220.0);
    vColor = aColor;
    vWorld = aPos;
}
