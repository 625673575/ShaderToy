vec4 mainImage(in vec2 fragCoord)
{
    vec2 uv=fragCoord/iResolution;
    return texture(iChannel0, uv.xy);
}




















