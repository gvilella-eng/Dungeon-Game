#version 330 core
in vec4 vColor;
in float vDist;
in vec3 vWorld;
out vec4 FragColor;
uniform int uAdditive;
uniform int uGlow;
uniform vec3 uAmb;
uniform vec4 uTorchPos;     // xyz, range
uniform vec3 uTorchCol;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform int uFullbright;
uniform int uClipOn;
uniform vec4 uClip;
void main()
{
    if (uClipOn > 0 && dot(vWorld, uClip.xyz) > uClip.w) discard;
    vec2 q = gl_PointCoord * 2.0 - 1.0;
    float d = dot(q, q);
    if (d > 1.0) discard;
    float a = (1.0 - d); a *= a;
    vec3 c = vColor.rgb;
    float f = uFogDensity > 0.0 ? exp(-uFogDensity * vDist) : 1.0;
    if (uGlow == 0) {
        vec3 L = uTorchPos.xyz - vWorld;
        float dd = length(L);
        float x = dd / max(uTorchPos.w, 0.1);
        float w = clamp(1.0 - x * x * x * x, 0.0, 1.0);
        vec3 lit = uAmb + uTorchCol * (w * w / (1.0 + 0.25 * dd * dd));
        if (uFullbright > 0) lit = vec3(1.0);
        c *= lit * 1.5;
    } else c *= 2.0;
    float alpha = a * vColor.a * (uAdditive > 0 ? f : 1.0);
    if (uAdditive > 0) FragColor = vec4(c * alpha, 0.0);
    else { c = mix(uFogColor, c, f); FragColor = vec4(c, alpha); }
}
