#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
uniform vec2 uTexel;     // 1/tamanho da entrada (menor)
uniform float uRadius;
void main()
{
    vec2 t = uTexel * uRadius;
    vec3 r = texture(uTex, vUV).rgb * 4.0;
    r += texture(uTex, vUV + vec2(-t.x, 0)).rgb * 2.0 + texture(uTex, vUV + vec2(t.x, 0)).rgb * 2.0;
    r += texture(uTex, vUV + vec2(0, -t.y)).rgb * 2.0 + texture(uTex, vUV + vec2(0, t.y)).rgb * 2.0;
    r += texture(uTex, vUV + vec2(-t.x, -t.y)).rgb + texture(uTex, vUV + vec2(t.x, -t.y)).rgb;
    r += texture(uTex, vUV + vec2(-t.x, t.y)).rgb + texture(uTex, vUV + vec2(t.x, t.y)).rgb;
    FragColor = vec4(r / 16.0, 1.0);
}
