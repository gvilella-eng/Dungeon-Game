#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
uniform vec2 uTexel;     // 1/tamanho da textura de ENTRADA
uniform int uFirst;      // 1: aplica limiar
vec3 thr(vec3 c) {
    float l = max(c.r, max(c.g, c.b));
    float k = max(l - 1.0, 0.0) / max(l, 1e-4);
    return c * k;
}
vec3 fetch(vec2 uv) { vec3 c = texture(uTex, uv).rgb; return uFirst > 0 ? thr(min(c, vec3(60.0))) : c; }
void main()
{
    vec2 t = uTexel;
    vec3 a = fetch(vUV + t * vec2(-2, -2)), b = fetch(vUV + t * vec2(0, -2)), c = fetch(vUV + t * vec2(2, -2));
    vec3 d = fetch(vUV + t * vec2(-2, 0)),  e = fetch(vUV),                   f = fetch(vUV + t * vec2(2, 0));
    vec3 g = fetch(vUV + t * vec2(-2, 2)),  h = fetch(vUV + t * vec2(0, 2)),  i = fetch(vUV + t * vec2(2, 2));
    vec3 j = fetch(vUV + t * vec2(-1, -1)), k = fetch(vUV + t * vec2(1, -1));
    vec3 l = fetch(vUV + t * vec2(-1, 1)),  m = fetch(vUV + t * vec2(1, 1));
    vec3 r = e * 0.125 + (a + c + g + i) * 0.03125 + (b + d + f + h) * 0.0625 + (j + k + l + m) * 0.125;
    FragColor = vec4(r, 1.0);
}
