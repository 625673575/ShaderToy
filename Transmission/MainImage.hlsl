#define PI 3.141592
#define eps 0.0001

float sdBox(vec3 pos, vec3 size) {
  vec3 d = abs(pos) - size;
  return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

float sdCircle(vec3 p, float r) {
  return length(p) - r;
}

float distFunc(vec3 pos) {
  float T = 5.0;
  float interval = 1.0;
  float cube_size = 0.18;
  float frame = 0.025;
  float sphere_size = 0.1;
  vec3 fold = pos;
  float ret, d;
  
  //frame cube
  fold = mod(fold, interval) - interval/2.0;
  d = sdBox(fold, vec3(cube_size-0.01));
  ret = d;
  d = sdBox(fold, vec3(cube_size, cube_size-frame, cube_size-frame));
  ret = max(ret, -d);
  d = sdBox(fold, vec3(cube_size-frame, cube_size, cube_size-frame));
  ret = max(ret, -d);
  d = sdBox(fold, vec3(cube_size-frame, cube_size-frame, cube_size));
  ret = max(ret, -d);
  
  //sphere
  float t = mod(iTime + floor(pos.y/interval) + 0.5*floor(pos.z/interval), T) - T/2.0;
  fold = pos;
  fold.y = mod(fold.y, interval);
  fold.z = mod(fold.z, interval) - interval/2.0;
  d = sdCircle(fold - vec3(interval/2.0, interval/2.0, 0.0)  - vec3(sign(t)*pow(t, 3.), 0.0, 0.0), sphere_size);
  ret = min(ret, d);
  
  return ret;
}

vec3 getNormal(vec3 pos) {
  return normalize(vec3(
      distFunc(vec3(pos.x+eps, pos.y, pos.z)) - distFunc(pos),
      distFunc(vec3(pos.x, pos.y+eps, pos.z)) - distFunc(pos),
      distFunc(vec3(pos.x, pos.y, pos.z+eps)) - distFunc(pos)
    ));
}

mat2 rotateMat(float angle) {
  return mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
}

vec3 transform(vec3 v) {
  vec3 ret = v;
  ret.xz =mul(ret.xz, rotateMat(PI/2.0));
  ret.yz =mul(ret.yz, rotateMat(iTime/5.0));
  return ret;
}

vec4 mainImage( in vec2 fragCoord )
{
  float d, b;
  vec3 col = vec3(0.0, 0.0, 0.0);
  vec3 transformed, normal;
  
  vec3 cameraPos = vec3(0.0, 0.0, 1.0);
  vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
  
  vec2 uv = (2.0*fragCoord - iResolution.xy)/iResolution.y;
  vec3 cur = transform(cameraPos) + vec3(0.0, 0.0, iTime/2.0);
  vec3 ray = normalize(transform(vec3(uv, 0.0) - cameraPos));
  
  for (float i = 0.; i < 64.; i+=1.0) {        
    d = distFunc(cur);
    if (d < eps) {
      normal = getNormal(cur);
      b = pow(1.0 - i/70.0, 2.0);
      vec3 m = abs(mod(cur, 5.0) - 2.5)/2.0;
      col = clamp(b, 0., 0.95)*m;
      break;
    }
    cur += d * ray;
  }

  return vec4(col,1.0);
}




