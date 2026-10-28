#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uScene;
uniform sampler2D uBloom;
uniform float uTime;
uniform float uExposure;
uniform float uBloomStr;
uniform float uDark;        // visao no escuro
uniform float uHallu;
uniform float uPsychic;
uniform float uFlash;       // dano
uniform vec3 uFlashColor;
uniform vec4 uFade;         // rgb, alpha
uniform float uUnder;       // submerso
uniform float uVignette;
uniform float uGrain;

vec3 aces(vec3 x)
{
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
float hash(vec2 p) { return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }

void main()
{
    vec2 uv = vUV;
    vec2 ctr = uv - 0.5;
    // distorcoes
    if (uHallu > 0.0) uv += vec2(sin(uv.y * 12.0 + uTime * 1.6), cos(uv.x * 10.0 + uTime * 1.3)) * 0.006 * uHallu;
    if (uPsychic > 0.0) uv += vec2(sin(uv.y * 20.0 + uTime * 2.2), sin(uv.x * 17.0 - uTime * 1.9)) * 0.0035 * uPsychic;
    if (uUnder > 0.0) uv += vec2(sin(uv.y * 25.0 + uTime * 1.5), cos(uv.x * 22.0 + uTime * 1.2)) * 0.003 * uUnder;
    vec3 col;
    if (uPsychic > 0.0) {
        float ca = (0.004 + 0.006 * (0.5 + 0.5 * sin(uTime * 1.7))) * uPsychic * (0.4 + length(ctr) * 2.0);
        col = vec3(texture(uScene, uv + ctr * ca).r, texture(uScene, uv).g, texture(uScene, uv - ctr * ca).b);
    } else col = texture(uScene, uv).rgb;
    vec3 bl = texture(uBloom, uv).rgb;
    col += bl * uBloomStr;
    col *= uExposure;
    if (uHallu > 0.0) col *= mix(vec3(1.0), vec3(1.25, 0.9, 0.7), uHallu);
    if (uUnder > 0.0) col = mix(col, col * vec3(0.35, 0.75, 0.9) + vec3(0.0, 0.02, 0.035), uUnder);
    col = aces(col);
    if (uHallu > 0.0) { float l = dot(col, vec3(0.3, 0.59, 0.11)); col = mix(col, mix(vec3(l), col, 1.5), uHallu); }
    col = pow(col, vec3(1.0 / 2.2));
    if (uDark > 0.0) {
        float l = dot(col, vec3(0.3, 0.59, 0.11));
        col = mix(col, vec3(l) * vec3(0.8, 0.95, 0.85) * 1.15, uDark);
    }
    float v = smoothstep(0.95, 0.25, length(ctr) * 1.25);
    col *= mix(1.0, v, uVignette);
    col += (hash(vUV * 1000.0 + fract(uTime) * 71.0) - 0.5) * uGrain;
    col = mix(col, uFlashColor, clamp(uFlash, 0.0, 1.0) * (0.25 + length(ctr) * 0.9));
    col = mix(col, uFade.rgb, uFade.a);
    FragColor = vec4(col, 1.0);
}
