vec4 mainImage(in vec2 fragCoord)
{
    vec2 uv = fragCoord.xy / iResolution.xy;
    
    return texture(iChannel0, uv)*0.5;
} 












