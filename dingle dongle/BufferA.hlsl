//#define COLOR vec3(0.1, 0.2, 1.)
#define COLOR (sin(time*vec3(.345, .456, .657))*.4+.6)
#define OFFSET vec2(0.5*cos(0.5*time-sin(time*0.5)), 0.5*sin(0.5*time+cos(time*0.75)))
#define time iTime

float saturate(float x){
    return clamp(x,0.0,1.0);
}

vec3 CenterLight(vec2 p){
    p -= OFFSET;
    p *= 12.;
 	float l = 1. / dot(p, p);
    return l*l * COLOR;
}
vec3 Straws(vec2 p){
    p -= OFFSET;
	float a = atan(p.y, p.x);
    float d = dot(p,p);
    
    float l = saturate(1.-length(p));
    
    l *= 0.5 + 0.5*sin(a*50.+d*5.+sin(time*1.)*20.);
    
    l *= 0.5 + 0.3 * sin(d*10.+a*10.-time*5.);
    return l*l*COLOR;
}
vec3 Lights(vec2 p){
    p -= OFFSET;
    
    float res = 0.0;
    
    for(float i = 0.; i < 500.; ++i){
        float t = time + i*i;
        
        vec2 q = p + vec2(cos(t), sin(t)) * sin(i*11.+time*0.5);
        float l = 0.0003 / pow(length(q), 1.5);
        
        res += l;
    }
    
    res = res*res;
    
    return res * COLOR;
}

vec4 mainImage( in vec2 fragCoord )
{
    vec2 uvs = fragCoord.xy/iResolution.xy;
    vec2 uv = (fragCoord*2.-iResolution.xy)/iResolution.y;

    vec3 col = vec3(0.0);
    
    vec2 p = uv;
    
    col += CenterLight(p);
 	col += Straws(p);
    col += Lights(p);
    
    col = max(min(vec3(1.0), col), vec3(0.0));
    vec3 newFrame = pow(col, vec3(1./0.8));
    vec3 oldFrame = texture(iChannel0, uvs).rgb;
    return vec4(newFrame + oldFrame,1.0)*0.85;
}

















