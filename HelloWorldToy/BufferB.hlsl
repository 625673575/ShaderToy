static const float PI = 3.14159265;
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
vec2 rand2(vec2 co){
	return vec2(rand(co),rand(co * 0.5) * 0.5);
}
float HAL(vec2 p,vec2 o,float r){
	return length(p - o) > r ? 1.0 : -1.0;
}
vec4 mainImage(in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.x;
    float s = 1.0;
    // Time varying pixel color
    vec3 col = vec3(length(uv));
    for(float xi = 0.0; xi < 8.0; xi += 1.0){
		s *= HAL(uv,
                 	vec2( 0.15 * xi,rand(vec2(xi * 0.1 + floor(iTime * 0.5))) * 0.6),
             fract(iTime * 0.5) * 2.0 < 1.75 ?
             smoothstep(0.0,0.5,fract(iTime * 0.5) * 2.0 - 0.15 * xi)
                 * 0.25:
             smoothstep(0.0,0.5,-fract(iTime * 0.5) * 2.0 + 2.05 )   
                 * 0.25);
    }

    // Output to screen
    vec3 col1 = vec3(225.0,204.0,155.0) / 255.0;
    vec3 col2 = vec3(195.0,73.0,57.0) / 255.0;
    return s > 0.000000000005 ? vec4(col1,0):vec4(col2,0);
}








