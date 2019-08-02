// Nintendo Switch by jackdavenport
// All code is free to use with credit! :)
// Created 2019

//------------------------------------------------------------------------------------------------//
// Signed Distance Fields
// Source: https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float dstPlane(vec3 p, vec4 plane) {
    return dot(p, plane.xyz) - plane.w;
}
float dstBox(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.)) + min(max(q.x, max(q.y, q.z)), 0.);
}
float dstRoundBox(vec3 p, vec3 b, float r) {
    return dstBox(p, b) - r;
}
float dstCappedCylinder(vec3 p, float h, float r)
{
    vec2 d = abs(vec2(length(p.xz), p.y)) - vec2(h, r);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}
float dstBox2D(vec2 p, vec2 b) {
    vec2 q = abs(p) - b;
    return length(max(q, 0.)) + min(max(q.x, q.y), 0.);
}

//------------------------------------------------------------------------------------------------//
// Helpful directive functions
#define rgb(r,g,b) (vec3(r,g,b)*0.00392156862) /* the number is 1/255 */
#define saturate(x) clamp(x,0.,1.)

//------------------------------------------------------------------------------------------------//
// Distance Functions/Booleans
// Some of these are from iq's website
// Source: https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
vec2 dstUnion(vec2 a, float bt, float bid) {
    if (a.x < bt) return a;
    return vec2(bt, bid);
}
float dstSubtract(float a, float b) {
    return max(a, -b);
}
float dstIntersect(float a, float b) {
    return max(a, b);
}
vec3 dstElongate(vec3 p, vec3 h) {
    return p - clamp(p, -h, h);
}

//------------------------------------------------------------------------------------------------//
// Materials/Lighting
struct Material {
    vec3 albedo;
    vec3 specular;
    float shininess;
    float reflectivity;
};
void getMaterial(inout Material mat, in vec3 p, in vec3 n, in vec2 t);

// Thanks knarkowicz!
// Source: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

//------------------------------------------------------------------------------------------------//
// Math Functions
vec2 rot2D(vec2 p, float a) {
    float s = sin(a), c = cos(a);
    return mul(mat2(c, s, -s, c) , p);
}
float expFog(float dist, float density) {
    return 1. - exp(-dist * density);
}

//------------------------------------------------------------------------------------------------//
// Camera Functions
void cameraPos(inout vec3 ro, in float time, in vec4 mouse, in vec2 res) {
    if (mouse.z < .5) {
        float theta = 3.14159 * time;
        float s = sin(theta), c = cos(theta);
        ro.x = s * 2.;
        ro.z = -c * 2.;
    }
    else {
        float yaw = 3.14159 * 2. * (mouse.x / res.x);
        float pitch = max(3.14159 * .5 * (mouse.y / res.y), .4);

        float sy = sin(yaw), cy = cos(yaw);
        float sp = sin(pitch), cp = cos(pitch);

        ro.x = sy * cp * 2.;
        ro.y = sp * 2.;
        ro.z = -cy * cp * 2.;
    }
}
void lookAt(in vec3 focalPoint, in vec3 eyePos, in vec3 upDir, inout vec3 rd) {
    vec3 f = normalize(focalPoint - eyePos);
    vec3 u = normalize(cross(f, upDir));
    vec3 v = normalize(cross(u, f));
    rd = mul(mat3(u, v, f) , rd);
}




























































