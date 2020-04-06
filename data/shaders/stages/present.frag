#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#softdefine MAX_VIEW_COUNT // How many simultaneous views can be enabled at the same time

layout(set = 0, binding = 0) uniform ViewsUbo {
    uint count;
} views;

layout(set = 0, binding = 1) uniform ViewportUbo {
    float x;
    float y;
    float width;
    float height;
    uint channelCount;
} viewports[MAX_VIEW_COUNT];

layout(set = 0, binding = 2) uniform sampler2D sourceSamplers[MAX_VIEW_COUNT];

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outPresent;

//----- Program

void main()
{
    outPresent = vec3(1 - length(inUv.xy - 0.5));

    for (uint i = 0; i < views.count; ++i) {
        // Convert UVs to viewport coordinates
        float x = (inUv.x - viewports[i].x) / viewports[i].width;
        float y = (inUv.y - viewports[i].y) / viewports[i].height;

        if (x < 0 || x > 1 || y < 0 || y > 1) {
            continue;
        }

        // Compose colors
        if (viewports[i].channelCount == 4) {
            vec4 color = texture(sourceSamplers[i], vec2(x, y));
            outPresent = mix(outPresent, color.rgb, color.a);
        }
        else if (viewports[i].channelCount == 3) {
            vec3 color = texture(sourceSamplers[i], vec2(x, y)).rgb;
            outPresent = color;
        }
        else if (viewports[i].channelCount == 1) {
            float color = texture(sourceSamplers[i], vec2(x, y)).r;
            outPresent = vec3(color);
        }
        else {
            outPresent = vec3(0.9, 0.7, 0.5);
        }
    }
}
