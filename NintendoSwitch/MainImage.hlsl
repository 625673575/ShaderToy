// Nintendo Switch by jackdavenport
// All code is free to use with credit! :)
// Created 2019

//------------------------------------------------------------------------------------------------//
#define MAX_ITER 32
#define MIN_DIST .001
#define MAX_DIST 20.
#define REFL_COUNT 1

//#define DEBUG_NO_LIGHT      // uncomment to disable shading
//#define DEBUG_SHOW_NORMALS  // uncomment to display normals

//------------------------------------------------------------------------------------------------//
static const vec3 lightPos1 = vec3(3., 1.5, -2.);
static const vec3 lightPos2 = vec3(-3., 3.5, 2.);
static const float lightIntensity = 1.05;
static const vec3 ambientLight = vec3(.2, .2, .1);
static const float consoleRot = .15;

//------------------------------------------------------------------------------------------------//
float dstJoystick(vec3 p) {
    return min(
        dstCappedCylinder(p - vec3(0., .03, 0.), .04, .06),
        dstCappedCylinder(p - vec3(0., .11, 0.), .04, .0005) - .02);
}
float dstButtonGrid(vec3 p) {
    float d = dstCappedCylinder(p, .03, .005);
    p.x -= .12; d = min(d, dstCappedCylinder(p, .03, .005));
    p.x += .06;
    p.z -= .08; d = min(d, dstCappedCylinder(p, .03, .005));
    p.z += .15; d = min(d, dstCappedCylinder(p, .03, .005));
    return d;
}
vec2 dstScene(vec3 p) {
    vec2 dst;
    // ground
    dst = vec2(dstPlane(p, vec4(0., 1., 0., -.05)), 0.);
    // console body
    p.y -= .04;
    p.yz = rot2D(p.yz, consoleRot);
    vec3 baseBox = vec3(.85, .025, .5);
    if (abs(p.x) <= 1.) {
        float base = dstRoundBox(p, baseBox, .025);
        if (p.z < -.45) {
            base = dstSubtract(base, dstRoundBox(p - vec3(0., .015, -.5), vec3(.007, .001, .003), .025));
            base = dstSubtract(base, dstRoundBox(p - vec3(-.09, .015, -.5), vec3(.002), .025));
            base = dstSubtract(base, dstRoundBox(p - vec3(.09, .015, -.5), vec3(.002), .025));
        }
        else if (p.z > .45) {
            if (p.x > -.5 && p.x < -.16) {
                vec3 q = p - vec3(0., 0., .53);
                q.x = mod(q.x, .07);
                base = dstSubtract(base, dstBox(q, vec3(.045, .025, .025)));
            }
            base = dstSubtract(base, dstRoundBox(p - vec3(-.74, -.025, .53), vec3(.06, .03, .03), .01));
            base = min(base, dstRoundBox(p - vec3(-.74, -.02, .505), vec3(.06, .02, .005), .01));
            base = min(base, dstRoundBox(p - vec3(.65, .0, .52), vec3(.05, .01, .01), .01));
        }
        dst = dstUnion(dst, base, 1.);
    }
    // joycons
    if (abs(p.x) > .8) {
        // base
        vec3 s = vec3(.93, 1., .55);
        float cutout = dstBox(p, baseBox * vec3(1., 3.5, 1.2));
        cutout = dstSubtract(dstCappedCylinder(dstElongate(p / s, vec3(.7, .0005, .47)), .5, .025) - .035, cutout);
        dst = dstUnion(dst, cutout, 2.);
        // buttons/joysticks
        float intShape = dstRoundBox(p - vec3(0., .07, -.02), baseBox * vec3(1., 1., .5), .3);
        if (p.x > .8) { // left controls
            dst = dstUnion(dst, dstJoystick(p - vec3(.98, 0., .25)), 3.);  // left stick
            dst = dstUnion(dst, dstButtonGrid(p - vec3(.93, .068, 0.)), 3.);  // left buttons
            dst = dstUnion(dst, dstBox(p - vec3(.94, .043, -.22), vec3(.025)), 3.); // capture button
            dst = dstUnion(dst, dstBox(p - vec3(.94, .044, .42), vec3(.025, .025, .005)), 3.); // - button
            dst = dstUnion(dst, dstIntersect(dstBox(p - vec3(1., 0., .44), vec3(.13, .025, .1)), intShape), 3.); // r button
            vec3 q = p - vec3(.97, -.07, .37);
            q.xz = rot2D(q.xz, .5);
            dst = dstUnion(dst, dstIntersect(dstRoundBox(q, vec3(.1, .04, .04), .042), intShape), 3.); // zl button
        }
        else if (p.x < -.8) { // right controls
            dst = dstUnion(dst, dstJoystick(p - vec3(-.99, 0., -.1)), 3.); // right stick
            dst = dstUnion(dst, dstButtonGrid(p - vec3(-1.05, .065, .2)), 3.);  // right buttons
            dst = dstUnion(dst, dstCappedCylinder(p - vec3(-.92, .065, -.22), .03, .005), 4.); // home button
            dst = dstUnion(dst, dstBox(p - vec3(-.92, .044, .42), vec3(.025, .025, .005)), 3.); // + button
            dst = dstUnion(dst, dstBox(p - vec3(-.92, .044, .42), vec3(.005, .025, .025)), 3.);
            dst = dstUnion(dst, dstIntersect(dstBox(p - vec3(-1., 0., .44), vec3(.13, .025, .1)), intShape), 3.); // r button
            vec3 q = p - vec3(-.97, -.07, .37);
            q.xz = rot2D(q.xz, -.5);
            dst = dstUnion(dst, dstIntersect(dstRoundBox(q, vec3(.1, .04, .04), .042), intShape), 3.); // zr button
            q = p - vec3(-.97, .006, -.42);
            q.xz = rot2D(q.xz, .525);
            dst = dstUnion(dst, dstIntersect(dstRoundBox(q, vec3(.05, .005, .04), .022), intShape), 4.); // ir sensor
        }
    }
    // end scene
    return dst;
}

vec2 raymarch(vec3 ro, vec3 rd, in float maxDist) {
    vec2 t = vec2(0., -1.);

    [unroll]for (uint i = 0; i < MAX_ITER; i++) {
        vec2 d = dstScene(ro + rd * t.x);
        if (d.x < MIN_DIST || t.x >= maxDist) {
            t.y = d.y;
            //break;
        }
        // multiplied to reduce visual artefacts
        // if anyone knows a way to avoid doing this, let me know :)
        t.x += d.x * .5;
    }
    return t;
}

//// source: https://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm
float softshadow(in vec3 ro, in vec3 rd, float mint, float maxt, float k)
{
    float res = 1.0;
    //[unroll]for (float t = mint; t < maxt; )
    {
        float t = mint;
        float h = dstScene(ro + rd * t).x;
        if (h < 0.001)
            return 0.0;
        res = min(res, k * h / t);
        t += h;
    }
    return res;
}

vec3 calcNormal(vec3 p, float t) {
    vec2 e = vec2(MIN_DIST * t, 0.);
    vec3 n = vec3(dstScene(p + e.xyy).x - dstScene(p - e.xyy).x,
        dstScene(p + e.yxy).x - dstScene(p - e.yxy).x,
        dstScene(p + e.yyx).x - dstScene(p - e.yyx).x);
    return normalize(n);
}

void calcLighting(vec3 p, vec3 n, vec3 rd, Material mat, out vec3 col) {
#ifndef DEBUG_NO_LIGHT
    vec3 diff = vec3(ambientLight);
    vec3 spec = vec3(0.);
    
    //[unroll] for (uint i = 0; i < 2; i++) {
        // calc light vector and distance
        vec3 lv = lightPos1;
        float ld = length(lv);
        lv /= ld;

        // calculate shadows
        float shadow = softshadow(p, lv, .01, ld, 8.);

        // calculate lighting
        float ndotl = max(dot(n, lv), 0.);
        diff += ndotl * shadow * lightIntensity;
        if (dot(mat.specular, mat.specular) > 0.) {
            vec3 h = normalize(lv - rd);
            float ndoth = max(dot(n, h), 0.);
            spec += mat.specular * pow(ndoth, mat.shininess) * shadow * lightIntensity;
        }

        lv = lightPos2;
        ld = length(lv);
        lv /= ld;

        // calculate shadows
        shadow = 0.0f;//softshadow(p, lv, .01, ld, 8.);

        // calculate lighting
        ndotl = max(dot(n, lv), 0.);
        diff += ndotl * shadow * lightIntensity;
        if (dot(mat.specular, mat.specular) > 0.) {
            vec3 h = normalize(lv - rd);
            float ndoth = max(dot(n, h), 0.);
            spec += mat.specular * pow(ndoth, mat.shininess) * shadow * lightIntensity;
        }
    //}

    // output final colour
    col = mat.albedo * diff + spec;
#else
    col = mat.albedo;
#endif
}

vec3 shade(vec3 ro, vec3 rd) {
    vec3 col = vec3(0.);
    float coeff = 1.;
    
    [unroll] for (uint i = 0; i < REFL_COUNT; i++) {
        vec2 scn = raymarch(ro, rd, MAX_DIST);

        if (scn.y > -1. && scn.x < MAX_DIST) {
            vec3 p = ro + rd * scn.x;
            vec3 n = calcNormal(p, scn.x);

#ifndef DEBUG_SHOW_NORMALS
            vec3 a = vec3(0.);

            Material mat;
            getMaterial(mat, p, n, scn);
            calcLighting(p, n, rd, mat, a);

            if (i == 0) {
                coeff *= 1. - saturate((scn.x - 5.) / 7.5);
            }

            if (mat.reflectivity > 0.) {
                float fres = 1. - clamp(pow(max(-dot(rd, n), 0.), .4), 0., 1.);
                fres *= mat.reflectivity;

                col += a * coeff * (1. - fres);
                coeff *= fres;

                vec3 r = normalize(reflect(rd, n));
                ro = p + r * .01;
                rd = r;
            }
            else {
                col += a * coeff;
                //break;
            }
#else
            col = n * .5 + .5;
            //break;
#endif
        }
        else if (i > 0) {
            col += texture(iChannel2, rd.xy).xyz * coeff;
            //break;
        }
        else {
           // break;
        }
    }


    // post processing
    //col = pow(col, vec3(1.));
    col = ACESFilm(col);
    return col;
}

vec4 mainImage(in vec2 fragCoord) {
    vec2 uv = (fragCoord - iResolution.xy * .5) / iResolution.y;
    vec3 ro = vec3(0., 1.2, 0.);
    vec3 rd = vec3(uv, 1.);

    cameraPos(ro, iTime * .06, iMouse, iResolution.xy);
    lookAt(vec3(0., 0., 0.), ro, vec3(0., 1., 0.), rd);

    return vec4(shade(ro, normalize(rd)),1.0);
}

//------------------------------------------------------------------------------------------------//

void getMaterial(inout Material mat, in vec3 p, in vec3 n, in vec2 t) {
    if (t.y == 0.) { // ground material
        vec4 tex = texture(iChannel0, p.xz / 3.5);
        mat.albedo = tex.xyz;
        mat.specular = vec3(.2 + .7 * tex.x);
        mat.shininess = 10. + 50. * tex.y;
        mat.reflectivity = .8;
    }
    else if (t.y == 1.) { // switch body
        p.yz = rot2D(p.yz, consoleRot);
        float screen = step(0., dstBox2D(p.xz, vec2(.73, .43)) - .03);
        if (screen < .5 && p.y > .05) {
            float innerScreen = step(0., dstBox2D(p.xz, vec2(.67, .38)));
            mat.albedo = mix(texture(iChannel1, (p.xz * vec2(.7, 1.3)) + vec2(.5, .5)).xyz, vec3(.1), innerScreen);
        }
        else {
            mat.albedo = rgb(38, 38, 38);
        }
        mat.specular = vec3(mix(1., .4, screen));
        mat.shininess = mix(60., 30., screen);
        mat.reflectivity = 1. - .9 * screen;
    }
    else if (t.y == 2.) { // joycons base
        mat.albedo = mix(rgb(247, 57, 47), rgb(46, 182, 255), step(0., p.x));
        mat.specular = vec3(.2);
        mat.shininess = 20.;
        mat.reflectivity = .2;
    }
    else if (t.y == 3.) { // joysticks/buttons
        mat.albedo = rgb(38, 38, 38);
        mat.specular = vec3(.5);
        mat.shininess = 8.;
        mat.reflectivity = .1;
    }
    else if (t.y == 4.) { // home button
        mat.albedo = rgb(54, 53, 52);
        mat.specular = vec3(.5);
        mat.shininess = 8.;
        mat.reflectivity = .1;
    }
    else if (t.y == 5.) { // ir sensor
        mat.albedo = vec3(.05);
        mat.specular = vec3(1.);
        mat.shininess = 80.;
        mat.reflectivity = 1.;
    }
    else { // default material
        mat.albedo = vec3(1., 0., 1.);
        mat.specular = vec3(0.);
        mat.shininess = 0.;
        mat.reflectivity = 0.;
    }
}
















































