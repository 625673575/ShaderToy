vec4 mainImage(in vec2 fragCoord)
{
vec3 finalColor;
	vec2 uv = fragCoord.xy*5.0;
    //center of the image
    vec2 c = iResolution.xy/2.0;
    finalColor = vec3( 0.3*_cross(uv, c, 240.0) );
    finalColor += ( circle(uv, c, 100.0, 1.0)
                  + circle(uv, c, 165.0, 1.0) ) * blue1;
    finalColor += (circle(uv, c, 240.0, 2.0) );//+ dots(uv,c,240.0)) * blue4;
    finalColor += circle3(uv, c, 313.0, 4.0) * blue1;
    finalColor += triangles(uv, c, 315.0 + 30.0*sin(iTime)) * blue2;
    finalColor += movingLine(uv, c, 240.0) * blue3;
    finalColor += circle(uv, c, 10.0, 1.0) * blue3;
    finalColor += 0.7 * circle2(uv, c, 262.0, 1.0, 0.5+0.2*cos(iTime)) * blue3;
    if( length(uv-c) < 240.0 )
    {
        //animate some bips with random movements
    	vec2 p = 130.0*MOV(1.3,1.0,1.0,1.4,3.0+0.1*iTime);
   		finalColor += bip1(uv, c+p) * vec3(1,1,1);
        p = 130.0*MOV(0.9,-1.1,1.7,0.8,-2.0+sin(0.1*iTime)+0.15*iTime);
        finalColor += bip1(uv, c+p) * vec3(1,1,1);
        p = 50.0*MOV(1.54,1.7,1.37,1.8,sin(0.1*iTime+7.0)+0.2*iTime);
        finalColor += bip2(uv,c+p) * red;
    }
	return vec4(finalColor,1.0);
} 
























