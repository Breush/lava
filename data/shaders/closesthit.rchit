#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_16bit_storage : require

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;

#include "./ray-payload.set"

layout(location = 0) rayPayloadInEXT Payload payload;

struct Vertex
{
    vec3 pos;
    vec2 uv;
    vec3 normal;
    vec4 tangent;
};

// @fixme Make files?
layout(binding = 2, set = 0) buffer Instance {
    vec4 transform0;
    vec4 transform1;
    vec4 transform2;
} instances[];
layout(binding = 3, set = 0) buffer Indices { uint16_t i[]; } indices[];
layout(binding = 4, set = 0) buffer Vertices { float v[]; } vertices[]; // @note There's no "packed" layout in SPIR-V, so we use a raw array.

hitAttributeEXT vec2 attribs;

// Our indices are stored as uint16, we extract them from the uint buffer.
uvec3 unpackIndex(uint id)
{
    uvec3 index;
    uint primitiveId = 3 * gl_PrimitiveID;
    index.x = uint(indices[id].i[primitiveId]);
    index.y = uint(indices[id].i[primitiveId + 1]);
    index.z = uint(indices[id].i[primitiveId + 2]);
    return index;
}

Vertex unpack(uint id, uint index)
{
    // Unpack the vertices from the SSBO using the glTF vertex structure
    const uint m = index * /* sizeof(Vertex) */  12;

    Vertex v;
    v.pos = vec3(vertices[id].v[m + 0], vertices[id].v[m + 1], vertices[id].v[m + 2]);
    v.uv = vec2(vertices[id].v[m + 3], vertices[id].v[m + 4]);
    v.normal = vec3(vertices[id].v[m + 5], vertices[id].v[m + 6], vertices[id].v[m + 7]);
    v.tangent = vec4(vertices[id].v[m + 8], vertices[id].v[m + 9], vertices[id].v[m + 10], vertices[id].v[m + 11]);

    return v;
}

void main()
{
    uint id = gl_InstanceCustomIndexEXT; // @fixme Need a instance -> meshId conversion. Here, it's all right so far

    // @fixme Get transform from scene!
    mat4 meshTransform = transpose(mat4(instances[id].transform0,
                                        instances[id].transform1,
                                        instances[id].transform2,
                                        vec4(0, 0, 0, 1)));

    uvec3 index = unpackIndex(id);

    Vertex v0 = unpack(id, index.x);
    Vertex v1 = unpack(id, index.y);
    Vertex v2 = unpack(id, index.z);

    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 normal = mat3(meshTransform) * normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);

    // Basic lighting
    vec3 lightVector = normalize(vec3(-3, -2, -1)); // @fixme Light from UBO
    float dotProduct = max(dot(lightVector, normal), 0.2); // @fixme Ambiant

    // UNDER blending
    // https://developer.nvidia.com/content/transparency-or-translucency-rendering
    vec4 color = vec4(vec3(dotProduct), 0.5); // @fixme Albedo
    payload.color.rgb += payload.color.a * color.a * color.rgb;
    payload.color.a *= (1 - color.a);
    // payload.dist = gl_RayTmaxEXT;
    // payload.normal = normal;

    if (true && payload.color.a < 0.99) { // Translucent material, going through
        // Computing the coordinates of the hit position
        vec4 worldPos = meshTransform * vec4(v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z, 1);

        float tmin = 0.0001;
        float tmax = 10000.0;
        traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, worldPos.xyz + tmin * gl_WorldRayDirectionEXT,
                    tmin, gl_WorldRayDirectionEXT, tmax, 0);
    }
}
