#version 450
#extension GL_ARB_separate_shader_objects : enable

#lava:include "./sets/deep-deferred-g-buffer.set"
#lava:include "./sets/deep-deferred-camera.set"
#lava:include "./sets/deep-deferred-light.set"

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outColor;

//----- Headers

vec4 colorFromNode(GBufferNode node);

//----- Program

mat4 viewTransformInverse = inverse(camera.viewTransform);
mat4 projectionTransformInverse = inverse(camera.projectionTransform);

void main()
{
    uint headerIndex = uint(gl_FragCoord.y * gBufferHeader.width + gl_FragCoord.x);

    //----- Sorting fragments by depth.
    
    GBufferNode sortedNodes[GBUFFER_MAX_NODE_DEPTH];
    uint listIndex = gBufferHeader.data[headerIndex];

    uint sortedNodeCount = 0;
    while (listIndex != 0 && sortedNodeCount < 5) {
        GBufferNode node = gBufferList.data[listIndex];
        listIndex = node.next;

        // Insertion sort
        uint insertionIndex = 0;
        while (insertionIndex < sortedNodeCount) {
            if (node.depth > sortedNodes[insertionIndex].depth) {
                break;
            }
            insertionIndex += 1;
        }

        uint lastIndex = min(GBUFFER_MAX_NODE_DEPTH - 1, sortedNodeCount);
        for (uint i = lastIndex; i > insertionIndex; --i) {
            sortedNodes[i] = sortedNodes[i - 1];
        }

        sortedNodes[insertionIndex] = node;
        sortedNodeCount += 1;
    }

    //----- Compositing.

    // Iterate over the gBuffer list, adding lights each time,
    // and composing translucent fragments.

    // @todo Allow clear color to be configurable
    vec3 color = vec3(0.2, 0.6, 0.4);
    for (uint i = 0; i < sortedNodeCount; ++i) {
        GBufferNode node = sortedNodes[i];
        vec4 nodeColor = colorFromNode(node);
        color = mix(color, nodeColor.rgb, nodeColor.a);
    }

    outColor = color;
}

//----- Implementations

vec3 wPositionFromDepth(float depth, vec2 coord) {
    vec4 pPosition = vec4(2 * coord - 1, depth, 1);
    vec4 vPosition = projectionTransformInverse * pPosition;
    vPosition /= vPosition.w;
    return (viewTransformInverse * vPosition).xyz;
}

vec4 colorFromNode(GBufferNode node) {
    vec3 normal = 2 * node.normal.xyz - 1;

    // No normal => flat shading
    // (the normal should already be normalized)
    if (dot(normal, normal) <= 0.5) {
        return vec4(node.albedo, node.opacity);
    }

    float depth = node.depth;
    vec3 albedo = node.albedo;
    float opacity = node.opacity;
    float occlusion = node.occlusion;
    float roughness = node.roughness;
    float metallic = node.metallic;
    
    // General ambient
    vec3 wPosition = wPositionFromDepth(depth, inUv);
    vec3 v = normalize(camera.wPosition.xyz - wPosition.xyz);
    float ambient = 0.2;

    // Material-specific
    float ka = 1 - 0.2 * roughness;
    float kd = 0.5 + 0.2 * roughness;
    float ks = 0.25 - 0.23 * roughness;
    float alpha = pow(2, 1 + 7 * metallic);

    // For each light
    // @note Distance should affect intensity
    float id = 1;
    float is = 1;

    float diffuse = 0;
    float specular = 0;

    // Check whether the lighting should have an effect
    vec3 lightVector = light.wPosition.xyz - wPosition.xyz;
    float lightDistance = length(lightVector);
    if (lightDistance < light.radius) {
        float i = 1 - (lightDistance * lightDistance) / (light.radius * light.radius);

        vec3 l = normalize(lightVector);
        float cosTheta = dot(normal, l);
        if (cosTheta > 0) {
            diffuse += i * id * cosTheta;

            vec3 r = normalize(2 * cosTheta * normal - l);
            float cosOmega = dot(r, v);
            if (cosOmega > 0) {
                specular += i * is * pow(cosOmega, alpha);
            }
        }
    }

    // Combining all lights k
    return vec4((ka * ambient + kd * occlusion * diffuse + ks * specular) * albedo, opacity);
}
