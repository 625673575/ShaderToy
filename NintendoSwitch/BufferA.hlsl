// Twisting Pylon by jackdavenport
// All code is free to use with credit! :)
// Created 2016
// Link to original: https://www.shadertoy.com/view/XstXW7

#define MAX_ITERATIONS 256
#define MIN_DISTANCE .001

#define LIGHT_COL vec3(1.)
#define LIGHT_DIR normalize(vec3(90.,80.,-45.))

struct Ray { vec3 ori;  vec3 dir; };
struct Dst { float dst; int id; };
struct Hit { vec3 p;    int id; };


Dst dstPillar(vec3 p, vec3 pos, vec3 box) {

    p.xz = rot2D(p.xz, (iTime + p.y) * 0.785398163);

    vec3    d = abs(pos - p) - box;
    float dst = min(max(d.x, max(d.y, d.z)), 0.) + length(max(d, 0.));
    Dst r;
    r.dst = dst;
    r.id = 0;
    return r;

}

Dst dstFloor(vec3 p, float y) {
    Dst r;
    r.dst = p.y - y;
    r.id = 1;
    return r;

}

Dst dstMin(Dst a, Dst b) {

    if (b.dst < a.dst) return b;
    return a;

}

Dst dstScene(vec3 p) {

    Dst dst = dstPillar(p, vec3(0.), vec3(.5, 2., .5));
    dst = dstMin(dst, dstFloor(p, -2.));

    return dst;

}

Hit raymarch(Ray ray) {

    vec3 p = ray.ori;
    int id = -1;

    for (int i = 0; i < MAX_ITERATIONS; i++) {

        Dst scn = dstScene(p);
        p += ray.dir * scn.dst * .75;

        if (scn.dst < MIN_DISTANCE) {

            id = scn.id;
            break;

        }

    }
    Hit r;
    r.p = p;
    r.id = id;
    return r;

}

// Shadow code from the incredible iq
// Source: https://www.shadertoy.com/view/Xds3zN
float softShadow(in vec3 ro, in vec3 rd, in float mint, in float tmax)
{
    float res = 1.0;
    float t = mint;
    for (int i = 0; i < 24; i++)
    {
        float h = dstScene(ro + rd * t).dst;
        res = min(res, 32.0 * h / t);
        t += clamp(h, 0.05, 0.50);
        if (h<0.001 || t>tmax) break;
    }
    return clamp(res, 0.0, 1.0);

}

vec3 calcNormal(vec3 p) {

    vec2 eps = vec2(.001, 0.);
    vec3   n = vec3(dstScene(p + eps.xyy).dst - dstScene(p - eps.xyy).dst,
        dstScene(p + eps.yxy).dst - dstScene(p - eps.yxy).dst,
        dstScene(p + eps.yyx).dst - dstScene(p - eps.yyx).dst);
    return normalize(n);

}

vec3 calcLighting(vec3 n, float s, Hit scn) {

    float d = max(dot(LIGHT_DIR, n), 0.);
    d *= s;

    return LIGHT_COL * d;

}

vec3 getPylonDiffuse(vec3 n, float s, Hit scn) {

    return calcLighting(n, s, scn);

}

vec3 getFloorDiffuse(Hit scn) {

    vec2 uv = mod(scn.p.xz / 3.5, 1.);
    float s = softShadow(scn.p, LIGHT_DIR, .0015, 10.);

    return texture(iChannel1, uv).xyz * calcLighting(vec3(0., 1., 0.), s, scn);

}

vec3 shade(Ray ray) {

    Hit scn = raymarch(ray);
    vec3 col = texture(iChannel0, ray.dir.xy).xyz;

    if (scn.id == 0) {

        vec3 n = calcNormal(scn.p);
        vec3 r = reflect(ray.dir, n);

        Ray rr;
        rr.ori = scn.p + r * .001;
        rr.dir = r;
        Hit rh = raymarch(rr);

        float sh = softShadow(scn.p, LIGHT_DIR, .0015, 10.);
        vec3  dc = getPylonDiffuse(n, sh, scn);
        vec3 rc =
            rh.id == 0 ? getPylonDiffuse(calcNormal(rh.p), softShadow(scn.p, LIGHT_DIR, .0015, 10.), rh) :
            rh.id == 1 ? getFloorDiffuse(rh) :
            texture(iChannel0, rr.dir.xy).xyz;

        vec3 s = LIGHT_COL * pow(max(dot(LIGHT_DIR, r), 0.), 30.) * softShadow(scn.p, LIGHT_DIR, .0015, 10.);
        float f = mix(.1, .9, 1. - max(pow(-dot(ray.dir, n), .1), 0.));
        return mix(dc, rc, f) + s;


    }
    else if (scn.id == 1) {

        col = getFloorDiffuse(scn);

    }

    col = clamp(col, 0., 1.); // make sure colours are clamped for texturing
    return col;

}

vec4 mainImage(in vec2 fragCoord)
{
    vec2 uv = (fragCoord - iResolution.xy * .5) / iResolution.y;

    vec3 ori = vec3(0., 0., -5.5);
    vec3 dir = vec3(uv, 1.);
    Ray r;
    r.ori = ori;
    r.dir = dir;
    vec3 col = shade(r);
    return vec4(col, 1.);
}




























































