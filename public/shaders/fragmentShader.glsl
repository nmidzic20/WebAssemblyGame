uniform float iTime;
uniform vec2 iResolution;

float Scale;

float map(vec3 p) {
    p = mod(p, 2.0) - 1.0;
    p = abs(p) - 1.0;
    if (p.x < p.z) p.xz = p.zx;
    if (p.y < p.z) p.yz = p.zy;
    if (p.x < p.y) p.xy = p.yx;
    float s = 1.0;
    for (int i = 0; i < 10; i++) {
        float r2 = 2.0 / clamp(dot(p, p), 0.1, 1.0);
        p = abs(p) * r2 - vec3(0.6, 0.6, 3.5);
        s *= r2;
    }
    Scale = log2(s);
    return length(p) / s;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec3 p, ro = vec3(0.5 + 0.2 * sin(iTime * 0.03), 0.05 * cos(iTime * 0.03), -0.1 * iTime),
    w = normalize(vec3(0.2, sin(iTime * 0.1), -1)),
    u = normalize(cross(w, vec3(0, 1, 0))),
    rd = mat3(u, cross(u, w), w) * normalize(vec3(uv, 2));
    float h = 0.4, d, i;
    for (i = 1.0; i < 100.0; i++) {
        p = ro + rd * h;
        d = map(p);
        if (d < 0.001 || h > 10.0) break;
        h += d;
    }
    fragColor.xyz = mix(vec3(1), vec3(cos(Scale * 2.5 + p * 1.8) * 0.5 + 0.5), 0.5) * 10.0 / i;
    if (i < 5.0) fragColor.xyz = vec3(0.5, 0.2, 0.1) * (5.0 - i);
    fragColor.a = 1.0;
}

void main() {
    mainImage(gl_FragColor, gl_FragCoord.xy);
}