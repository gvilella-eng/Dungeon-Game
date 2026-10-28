#version 330 core
in vec3 vWorld;
in vec3 vNormal;
in vec4 vColor;
flat in int vMat;
in float vAo;
in vec2 vFlow;
out vec4 FragColor;

uniform vec3 uCamPos;
uniform float uTime;
uniform int uFullbright;
uniform int uBackTint;
uniform int uNoFog;
uniform int uPass;          // 0 opaco, 1 transparente
uniform int uXray;          // 1: casca da caverna translucida
uniform int uClipOn;
uniform vec4 uClip;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform vec3 uAmbSky;
uniform vec3 uAmbGround;
uniform vec3 uAmbUp;
uniform float uAmbStr;
uniform int uNumLights;
uniform vec4 uLightPos[32];   // xyz, range
uniform vec4 uLightCol[32];   // rgb * intensidade
uniform float uHallu;
uniform float uDark;          // visao no escuro (0/1)
uniform vec4 uMatParams[48];  // rough, metal, emissive, wet
uniform vec4 uMatBase[48];    // rgb, transparente
uniform sampler3D uNoise;

const float PI = 3.14159265;

vec3 lightingAt(vec3 P, vec3 N, vec3 V, vec3 albedo, float rough, float metal, float ao)
{
    vec3 specCol = mix(vec3(0.04), albedo, metal);
    vec3 diffCol = albedo * (1.0 - metal);
    float shin = max(2.0 / (rough * rough + 0.0008) - 2.0, 1.0);
    float norm = (shin + 8.0) / 25.13;
    vec3 total = vec3(0.0);
    float aoD = mix(1.0, ao, 0.6);
    for (int i = 0; i < uNumLights; ++i) {
        vec3 L = uLightPos[i].xyz - P;
        float d2 = dot(L, L);
        float r = uLightPos[i].w;
        if (d2 > r * r) continue;
        float d = sqrt(d2);
        L /= max(d, 1e-4);
        float x = d / r;
        float win = clamp(1.0 - x * x * x * x, 0.0, 1.0);
        float att = win * win / (1.0 + 0.25 * d2);
        float nl = max((dot(N, L) + 0.12) / 1.12, 0.0);
        vec3 H = normalize(L + V);
        float nh = max(dot(N, H), 0.0);
        float fres = pow(1.0 - max(dot(H, V), 0.0), 5.0);
        vec3 F = specCol + (1.0 - specCol) * fres * 0.6;
        float sp = min(pow(nh, shin) * norm, 6.0);
        vec3 c = uLightCol[i].rgb * att;
        total += c * (diffCol * nl * aoD + F * sp * max(dot(N, L), 0.0));
    }
    return total;
}

vec3 ridgeDecode(vec4 t) { return (t.rgb * 2.0 - 1.0) * 4.0; }

void main()
{
    if (uClipOn > 0 && dot(vWorld, uClip.xyz) > uClip.w) discard;

    vec3 P = vWorld;
    vec3 N = normalize(vNormal);
    bool back = !gl_FrontFacing;
    if (back) N = -N;
    vec3 Vv = uCamPos - P;
    float dist = length(Vv);
    vec3 V = Vv / max(dist, 1e-4);
    int m = vMat;
    vec4 mp = uMatParams[m];
    vec3 albedo; float rough = mp.x; float metal = mp.y; vec3 emis = vec3(0.0); float wet = mp.w;
    float alpha = 1.0;
    float ao = vAo;
    float upd = dot(N, uAmbUp);

    if (m < 16) {
        // ---------------- rocha SDF ----------------
        const mat3 R1 = mat3(0.36, -0.80, 0.48, 0.48, 0.60, 0.64, -0.80, 0.0, 0.60);
        const mat3 R2 = mat3(0.0, 1.0, 0.0, 0.6, 0.0, 0.8, 0.8, 0.0, -0.6);
        mat3 R3 = R1 * R2;
        vec4 a = texture(uNoise, R1 * P * 0.2347);
        vec4 b = texture(uNoise, R2 * P * 0.9173 + 0.37);
        vec4 c = texture(uNoise, R3 * P * 3.0711 + 0.71);
        vec3 base = uMatBase[m].rgb;
        float vr = vColor.r, vg = vColor.g;
        float fl = smoothstep(0.55, 0.82, upd);
        float strength = 0.7;
        float strata = sin(P.y * 2.7 + a.a * 6.0) * 0.5 + 0.5;
        albedo = base * (0.6 + 0.8 * a.a) * (0.85 + 0.3 * b.a) * (0.75 + 0.5 * vr);
        if (m == 0 || m == 1) {
            albedo *= 0.86 + 0.28 * strata;
            vec3 dirt = vec3(0.14, 0.115, 0.09) * (0.55 + 0.9 * c.a) * (0.8 + 0.4 * vg);
            albedo = mix(albedo, dirt, fl * (m == 0 ? 0.85 : 0.65));
            wet = mix(wet, 0.9, smoothstep(0.58, 0.7, vg) * 0.7);
        } else if (m == 2) {
            vec3 ash = vec3(0.13, 0.125, 0.125) * (0.6 + 0.8 * c.a);
            albedo = mix(albedo, ash, fl);
            strength = 0.18; wet = mix(0.7, 0.05, fl);
            rough = mix(0.1, 0.9, fl);
        } else if (m == 3) {
            float quartz = smoothstep(0.62, 0.70, c.a * 0.55 + b.a * 0.45);
            albedo = mix(base * (0.55 + 0.9 * a.a), vec3(0.78, 0.7, 0.95), quartz);
            albedo = mix(albedo, vec3(0.12, 0.08, 0.16) * (0.6 + c.a), fl * 0.5);
            emis = base * (0.10 + 0.18 * b.a) + vec3(0.5, 0.35, 0.7) * quartz * 0.12;
            rough = mix(0.3, 0.08, quartz); strength = 0.35;
        } else if (m == 4) {
            vec3 ash = vec3(0.10, 0.05, 0.045) * (0.6 + 0.8 * c.a);
            albedo = mix(albedo, ash, fl * 0.8);
            float crack = (1.0 - smoothstep(0.0, 0.035, abs(b.a - 0.5))) * smoothstep(0.42, 0.62, a.a);
            float crack2 = (1.0 - smoothstep(0.0, 0.03, abs(a.a - 0.5))) * smoothstep(0.45, 0.65, c.a);
            float pulse = 0.55 + 0.45 * sin(uTime * 1.7 + a.a * 9.0);
            emis = vec3(1.0, 0.27, 0.04) * (crack + crack2 * 0.9) * pulse * 1.4;
            strength = 0.6;
        } else if (m == 5) {
            float veins = 1.0 - smoothstep(0.0, 0.05, abs(b.a - 0.5));
            albedo = base * (0.7 + 0.5 * a.a);
            emis = vec3(0.55, 0.15, 0.95) * veins * (0.28 + 0.12 * sin(uTime * 0.9 + a.a * 7.0));
            rough = 0.2; strength = 0.07; wet = 0.4;
        } else if (m == 6) {
            albedo = base * (0.8 + 0.4 * a.a);
            emis = vec3(0.03, 0.07, 0.09); strength = 0.12; rough = 0.15;
        } else if (m == 7) {
            vec3 rock = vec3(0.26, 0.25, 0.25) * (0.55 + 0.9 * a.a);
            vec3 snow = vec3(0.82, 0.86, 0.94) * (0.9 + 0.15 * b.a);
            float amt = smoothstep(0.30, 0.72, upd + (a.a - 0.5) * 0.7);
            albedo = mix(rock, snow, amt);
            rough = 0.88; strength = mix(0.5, 0.12, amt); wet = 0.0;
        } else if (m == 8) {
            albedo = vec3(0.17, 0.16, 0.155) * (0.6 + 0.8 * c.a);
            strength = 0.4;
        }
        // normal por gradiente do ruido 3D (sem esticamento)
        vec3 g = (ridgeDecode(a) * R1) * 0.5 + (ridgeDecode(b) * R2) * 0.45 + (ridgeDecode(c) * R3) * 0.32;
        float pulseB = 1.0 + uHallu * 0.9 * sin(uTime * 1.8 + P.y * 0.5);
        N = normalize(N - (g - N * dot(g, N)) * strength * pulseB);
        if (uHallu > 0.001) {
            vec3 flesh = vec3(0.42, 0.09, 0.09) * (0.6 + 0.9 * a.a);
            float veins = 1.0 - smoothstep(0.0, 0.07, abs(b.a - 0.5));
            albedo = mix(albedo, flesh, uHallu);
            emis += vec3(1.0, 0.33, 0.05) * veins * uHallu * (0.8 + 0.7 * sin(uTime * 2.3 + a.a * 8.0));
            wet = mix(wet, 0.85, uHallu); rough = mix(rough, 0.35, uHallu);
        }
        rough = mix(rough, 0.28, wet * smoothstep(0.3, 0.65, b.a) * 0.8);
    } else if (m >= 40) {
        // ---------------- liquidos ----------------
        vec2 fl = vFlow;
        vec3 base = uMatBase[m].rgb;
        vec2 uv = P.xz * 0.35 - fl * uTime * 0.8;
        vec4 w1 = texture(uNoise, vec3(uv, uTime * 0.05));
        vec4 w2 = texture(uNoise, vec3(uv * 2.3 + 0.37, -uTime * 0.07));
        vec3 g = ridgeDecode(w1) + ridgeDecode(w2) * 0.6;
        float ripple = (m == 41) ? 0.5 : 0.22;
        if (m == 42) {
            // lava
            vec2 u2 = P.xz * 0.14 - fl * uTime * 0.06;
            float t = texture(uNoise, vec3(u2, uTime * 0.02)).a;
            float t2 = texture(uNoise, vec3(u2 * 2.7 + 0.2, uTime * 0.03)).a;
            float crust = smoothstep(0.50, 0.62, t * 0.7 + t2 * 0.3);
            vec3 hot = vec3(2.2, 0.62, 0.09) * (0.8 + 0.4 * sin(uTime * 1.3 + t * 6.0));
            vec3 cold = vec3(0.16, 0.035, 0.02);
            emis = mix(hot, cold * 2.0, crust);
            albedo = vec3(0.05, 0.02, 0.015);
            rough = 0.8; metal = 0.0;
            N = normalize(vec3(0.0, 1.0, 0.0) + vec3(g.x, 0.0, g.z) * 0.08);
        } else {
            N = normalize(N + vec3(g.x, 0.0, g.z) * ripple);
            albedo = base;
            rough = 0.04;
            float fres = pow(1.0 - max(dot(N, V), 0.0), 4.0);
            alpha = clamp(0.6 + fres * 0.4, 0.0, 1.0);
            if (m == 41) {
                float bubbles = smoothstep(0.66, 0.72, w1.a * 0.6 + w2.a * 0.5);
                albedo = mix(albedo, vec3(0.9), bubbles * 0.5);
                emis = vec3(0.9, 0.25, 0.06) * 0.35 * (0.6 + 0.4 * w2.a);
                alpha = max(alpha, bubbles * 0.8);
            } else if (m == 43) {
                float sw = w1.a * 0.7 + w2.a * 0.5;
                emis = vec3(0.18, 0.95, 0.35) * (0.35 + 0.8 * sw) * 0.7;
                alpha = 0.55 + 0.2 * sw;
            } else {
                float foam = smoothstep(0.72, 0.8, w1.a * 0.5 + w2.a * 0.6);
                albedo = mix(albedo, vec3(0.8, 0.9, 0.95), foam * 0.6);
            }
        }
        wet = 1.0;
    } else {
        // ---------------- props ----------------
        vec3 vc = pow(vColor.rgb, vec3(2.2));
        float grain = texture(uNoise, P * 2.0).a;
        float gr2 = texture(uNoise, P * 7.0).a;
        albedo = vc * (0.82 + 0.36 * grain) * (0.93 + 0.14 * gr2);
        if (m == 16 || m == 17) albedo *= 0.8 + 0.4 * sin(P.y * 25.0 + grain * 6.0) * 0.5 + 0.2;
        if (mp.z > 0.0) emis = vc * mp.z;
        alpha = vColor.a;
        if (uMatBase[m].w > 0.5) {
            float fres = pow(1.0 - max(dot(N, V), 0.0), 3.0);
            alpha = clamp(alpha + fres * 0.45, 0.0, 1.0);
        }
        ao = 1.0;
    }

    vec3 col;
    if (uFullbright > 0) {
        vec3 Ld = normalize(vec3(0.4, 0.85, 0.3));
        float nl = max(dot(N, Ld), 0.0);
        float nl2 = max(dot(N, -Ld), 0.0) * 0.25;
        col = albedo * (0.38 + 0.62 * nl + nl2) * mix(1.0, ao, 0.7) * 1.35 + emis * 0.6;
        // brilho especular leve para ler a forma
        vec3 H = normalize(Ld + V);
        col += pow(max(dot(N, H), 0.0), 40.0) * 0.08;
        if (m >= 40 && m != 42) col = mix(col, albedo * 1.2 + emis, 0.35);
        if (m == 42) col = emis;
    } else {
        vec3 hemi = mix(uAmbGround, uAmbSky, dot(N, uAmbUp) * 0.5 + 0.5);
        col = albedo * hemi * uAmbStr * mix(1.0, ao, 0.95);
        if (m != 42) col += lightingAt(P, N, V, albedo, rough, metal, ao);
        if (uDark > 0.5) {
            float lum = dot(albedo, vec3(0.33, 0.34, 0.33)) + 0.05;
            col += vec3(lum) * 0.55 * (1.0 - smoothstep(4.0, 18.0, dist)) * mix(0.4, 1.0, ao) * max(dot(N, V), 0.15);
        }
        col += emis;
    }

    if (back && uBackTint > 0 && m < 16) {
        col = mix(col, vec3(0.05, 0.09, 0.16) + albedo * 0.12, 0.8);
    }

    if (uNoFog == 0 && uFullbright == 0) {
        float f = 1.0 - exp(-uFogDensity * dist);
        col = mix(col, uFogColor, clamp(f, 0.0, 1.0));
    }

    if (uXray > 0 && m < 16) {
        float nl = 0.35 + 0.65 * max(dot(N, normalize(vec3(0.4, 0.85, 0.3))), 0.0);
        FragColor = vec4(vec3(0.25, 0.45, 0.75) * nl * (back ? 0.6 : 1.0), 0.28);
        return;
    }

    if (uPass == 1) {
        // premultiplicado: emissivo/especular nao escalam com alpha
        vec3 litDiff = col - emis;
        FragColor = vec4(litDiff * alpha + emis, alpha);
    } else {
        FragColor = vec4(col, 1.0);
    }
}
