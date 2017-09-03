#version 450
#extension GL_ARB_separate_shader_objects : enable

#lava:define MAX_VIEW_COUNT // How many simultaneous views can be enabled at the same time

layout(set = 0, binding = 0) uniform ViewsUbo {
    uint count;
} views;

layout(set = 0, binding = 1) uniform ViewportUbo {
    float x;
    float y;
    float width;
    float height;
} viewports[MAX_VIEW_COUNT];

layout(set = 0, binding = 2) uniform sampler2D sourceSamplers[MAX_VIEW_COUNT];

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outPresent;

//----- Program

void main()
{
    outPresent = vec3(1, 1, 1);

    for (uint i = 0; i < views.count; ++i) {
        // Convert UVs to viewport coordinates
        float x = (inUv.x - viewports[i].x) / viewports[i].width;
        float y = (inUv.y - viewports[i].y) / viewports[i].height;

        if (x < 0 || x > 1 || y < 0 || y > 1) {
            continue;
        }

        // @todo We could choose how to compose (fully erase or use alpha?)
        outPresent = texture(sourceSamplers[i], vec2(x, y)).xyz;
    }
}
