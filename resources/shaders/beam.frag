#version 330 core
in vec2 vUV;
in vec4 vColor;
out vec4 FragColor;
uniform float uTime;
void main()
{
    float s = abs(vUV.x * 2.0 - 1.0);
    float core = pow(max(1.0 - s, 0.0), 2.5);
    float flick = 0.9 + 0.1 * sin(uTime * 30.0 + vUV.y * 40.0);
    FragColor = vec4(vColor.rgb * vColor.a * core * flick, 0.0);
}
