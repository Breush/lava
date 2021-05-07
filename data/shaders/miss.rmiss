#version 460
#extension GL_EXT_ray_tracing : enable

#include "./ray-payload.set"

layout(location = 0) rayPayloadInEXT Payload payload;

void main()
{
    // View-independent background gradient to simulate a basic sky background
    const vec3 gradientStart = vec3(0.5, 0.6, 1.0);
    const vec3 gradientEnd = vec3(1.0);
    vec3 unitDir = normalize(gl_WorldRayDirectionEXT);
    float t = 0.5 * (unitDir.z + 1.0);

    vec3 color = (1.0 - t) * gradientStart + t * gradientEnd;

    // UNDER blending to background color
    // https://developer.nvidia.com/content/transparency-or-translucency-rendering
    payload.color.rgb += payload.color.a * color.rgb;
    payload.color.a = 1.0;
}
