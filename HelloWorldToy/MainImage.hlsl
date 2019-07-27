vec4 mainImage(in vec2 fragCoord)
{
    vec2 uv=fragCoord/iResolution.xy*sin(iTime);
    return texture(iChannel0, uv.xy)+vec4(0.5,0.5,0.5,1.0);
}
