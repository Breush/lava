#include <lava/sill/i-mesh.hpp>

using namespace lava;
using namespace lava::sill;

namespace {
    void updateNodeEntitySpaceMatrix(MeshNode& node, const glm::mat4& parentEntitySpaceMatrix)
    {
        node.entitySpaceMatrix = parentEntitySpaceMatrix * node.parentSpaceMatrix;

        for (auto childIndex : node.children) {
            auto& childNode = *(&node + childIndex);
            updateNodeEntitySpaceMatrix(childNode, node.entitySpaceMatrix);
        }
    }

    void printNodeHierarchy(const MeshNode& node, std::ostream& s, uint32_t tabs)
    {
        for (auto i = 0u; i < tabs; ++i) {
            s << "    ";
        }

        s << "[MeshNode] " << node.name;
        if (node.group) {
            s << " (mesh [instance " << node.instanceIndex << "] - [" << node.group->primitives().size() << " primitives])";
        }
        s << std::endl;

        for (auto childIndex : node.children) {
            auto& childNode = *(&node + childIndex);
            printNodeHierarchy(childNode, s, tabs + 1u);
        }
    }
}

IMesh::IMesh(magma::Scene& scene, bool autoInstancingEnabled)
    : m_scene(scene)
    , m_autoInstancingEnabled(autoInstancingEnabled)
{
}

void IMesh::reserveNodes(uint32_t nodesCount)
{
    m_nodes.reserve(nodesCount);
}

uint32_t IMesh::addNode()
{
    m_nodesDirty = true;
    m_nodes.emplace_back();
    return m_nodes.size() - 1u;
}

uint32_t IMesh::addInstancedNode(uint32_t sourceNodeIndex)
{
    auto nodeIndex = addNode();

    auto& node = m_nodes.back();
    const auto& sourceNode = m_nodes[sourceNodeIndex];
    node.name = sourceNode.name;
    node.group = sourceNode.group;
    nodeMatrix(nodeIndex, sourceNode.matrix);

    // Warn that all the primitive are instances.
    if (m_autoInstancingEnabled && node.group) {
        // @todo There's currently no way to remove one node at a time,
        // but if it becomes possible, one would need to warn the primitive
        // that there is one less instance to render.
        for (auto primitive : node.group->primitives()) {
            node.instanceIndex = primitive->addInstance();
        }
    }

    // Recursively add all nodes that are below.
    // @note As new nodes are added by children, the references to node and sourceNode
    // might be invalidated, thus the necessary copy.
    auto sourceNodeChildren = sourceNode.children;
    for (auto child : sourceNodeChildren) {
        auto childNodeIndex = addInstancedNode(sourceNodeIndex + child);
        nodeAddAbsoluteChild(nodeIndex, childNodeIndex);
    }

    return nodeIndex;
}

MeshGroup& IMesh::nodeMakeGroup(uint32_t nodeIndex)
{
    m_nodesDirty = true;
    auto& node = m_nodes[nodeIndex];
    node.group = std::make_shared<MeshGroup>(m_scene, m_autoInstancingEnabled);
    return *node.group;
}

void IMesh::nodeName(uint32_t nodeIndex, std::string name)
{
    m_nodesDirty = true;
    auto& node = m_nodes[nodeIndex];
    node.name = std::move(name);
}

void IMesh::nodeMatrix(uint32_t nodeIndex, const glm::mat4& matrix)
{
    m_nodesDirty = true;
    auto& node = m_nodes[nodeIndex];

    glm::vec3 skew;
    glm::vec4 perspective;
    node.matrix = matrix;
    node.parentSpaceMatrix = matrix;
    glm::decompose(matrix, node.scaling, node.rotation, node.translation, skew, perspective);

    if (node.parent != 0) {
        auto& parentNode = m_nodes[nodeIndex + node.parent];
        updateNodeEntitySpaceMatrix(node, parentNode.entitySpaceMatrix);
    } else {
        updateNodeEntitySpaceMatrix(node, glm::mat4{1.f});
    }
}

void IMesh::nodeAddAbsoluteChild(uint32_t nodeIndex, uint32_t childNodeIndex)
{
    m_nodesDirty = true;
    auto& node = m_nodes[nodeIndex];
    node.children.emplace_back(childNodeIndex - nodeIndex);

    auto& childNode = m_nodes[childNodeIndex];
    childNode.parent = nodeIndex - childNodeIndex;

    updateNodeEntitySpaceMatrix(childNode, node.entitySpaceMatrix);
}

// ----- Animations

void IMesh::addAnimation(const std::string& hrid, const MeshAnimation& animation)
{
    m_animations.emplace(hrid, animation);
}

// ----- Attributes

void IMesh::renderCategory(RenderCategory renderCategory)
{
    for (auto& node : m_nodes) {
        if (node.group == nullptr) continue;

        for (auto& primitive : node.group->primitives()) {
            primitive->renderCategory(renderCategory);
        }
    }
}

void IMesh::enabled(bool enabled)
{
    for (auto& node : m_nodes) {
        if (node.group == nullptr) continue;

        for (auto& primitive : node.group->primitives()) {
            primitive->enabled(enabled);
        }
    }
}

BoundingSphere IMesh::boundingSphere() const
{
    BoundingSphere boundingSphere;

    for (auto& node : m_nodes) {
        if (node.group == nullptr) continue;

        for (auto& primitive : node.group->primitives()) {
            boundingSphere = mergeBoundingSpheres(boundingSphere, primitive->boundingSphere(node.instanceIndex));
        }
    }

    return boundingSphere;
}

void IMesh::printHierarchy(std::ostream& s) const
{
    s << "[IMesh]" << std::endl;

    // @note The root nodes have just no parent!
    for (auto& node : m_nodes) {
        if (node.parent != 0) continue;
        printNodeHierarchy(node, s, 1u);
    }
}

// ----- Internal

void IMesh::updateNodesEntitySpaceMatrices()
{
    for (auto& node : m_nodes) {
        if (node.parent != 0) continue;
        updateNodeEntitySpaceMatrix(node, glm::mat4{1.f});
    }
}
