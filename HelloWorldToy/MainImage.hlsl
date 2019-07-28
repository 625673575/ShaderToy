vec4 mainImage(in vec2 fragCoord)
{
    vec2 uv=fragCoord/iResolution.xy+0.2*sin(iTime);
    return texture(iChannel0, uv.xy);
}





















