#include "./barrier.hpp"

#include "../game-state.hpp"
#include "../serializer.hpp"
#include "../environment.hpp"

#include <lava/chamber/math.hpp>

using namespace lava;

Barrier::Barrier(GameState& gameState)
    : Object(gameState)
{
    gameState.level.barriers.emplace_back(this);

    m_entity->name("barrier");
    m_entity->make<sill::TransformComponent>();
    m_entity->make<sill::MeshComponent>();
    m_entity->make<sill::AnimationComponent>();

    m_panesMaterial = gameState.engine->scene().makeMaterial("barrier");
}

void Barrier::clear(bool removeFromLevel)
{
    if (removeFromLevel) {
        auto barrierIt = std::find(m_gameState.level.barriers.begin(), m_gameState.level.barriers.end(), this);
        m_gameState.level.barriers.erase(barrierIt);
    }

    // @note Keep last, this destroys us!
    Object::clear(removeFromLevel);
}

void Barrier::unserialize(const nlohmann::json& data)
{
    powered(data["powered"]);
    pickingBlocked(data["pickingBlocked"]);
    teleportationBlocked(data["teleportationBlocked"]);

    m_controlPoints.clear();
    for (auto point : data["controlPoints"]) {
        ControlPoint controlpoint;
        controlpoint.point = ::unserializeVec2(point);
        m_controlPoints.emplace_back(controlpoint);
    }
}

nlohmann::json Barrier::serialize() const
{
    nlohmann::json data = {
        {"powered", m_powered},
        {"pickingBlocked", m_pickingBlocked},
        {"teleportationBlocked", m_teleportationBlocked},
        {"controlPoints", nlohmann::json::array()},
    };

    auto& controlPointsData = data["controlPoints"];
    for (const auto& controlPoint : m_controlPoints) {
        controlPointsData.emplace_back(::serialize(controlPoint.point));
    }

    return data;
}

void Barrier::consolidateReferences()
{
    updateFromControlPoints();
}

void Barrier::uiWidgets(std::vector<UiWidget>& widgets)
{
    Object::uiWidgets(widgets);

    widgets.emplace_back("powered", "powered",
        [this]() -> bool { return powered(); },
        [this](bool powered) { this->powered(powered); }
    );
    widgets.emplace_back("pickingBlocked", "pickingBlocked",
        [this]() -> bool { return pickingBlocked(); },
        [this](bool pickingBlocked) { this->pickingBlocked(pickingBlocked); }
    );
    widgets.emplace_back("teleportationBlocked", "teleportationBlocked",
        [this]() -> bool { return teleportationBlocked(); },
        [this](bool teleportationBlocked) { this->teleportationBlocked(teleportationBlocked); }
    );
}

void Barrier::editorOnClicked(const glm::vec3& hitPoint)
{
    float minDistance = INFINITY;
    for (auto i = 0u; i < m_controlPoints.size(); ++i) {
        const auto& controlPoint = m_controlPoints[i];
        float distance = glm::distance(controlPoint.position, hitPoint);
        if (distance < minDistance) {
            minDistance = distance;
            m_editorControlPointIndex = i;
        }
    }
}

const glm::vec3& Barrier::editorOrigin() const
{
    return m_controlPoints[m_editorControlPointIndex].position;
}

void Barrier::editorTranslate(const glm::vec3& translation)
{
    m_controlPoints[m_editorControlPointIndex].point += glm::vec2(translation);

    updateFromControlPoints();
}

// -----

bool Barrier::intersectSegment(const glm::vec3& start, const glm::vec3& end) const
{
    // We solve this question in 2D by checking projections from the top.
    // This works because the barrier is considered infinitely high.

    // @fixme Would be nice to have some thickness to the barrier,
    // so that the collisions with the barriers are not making visual clipping.

    for (const auto& pane : m_panes) {
        const auto& paneStart = pane.p0.point;
        const auto& paneEnd = pane.p1.point;

        auto paneD = paneEnd - paneStart;
        auto p0 = paneD.y * (paneEnd.x - start.x) - paneD.x * (paneEnd.y - start.y);
        auto p1 = paneD.y * (paneEnd.x - end.x) - paneD.x * (paneEnd.y - end.y);

        if (p0 * p1 <= 0.f) {
            auto d = end - start;
            auto p2 = d.y * (end.x - paneStart.x) - d.x * (end.y - paneStart.y);
            auto p3 = d.y * (end.x - paneEnd.x) - d.x * (end.y - paneEnd.y);
            if (p2 * p3 <= 0.f) {
                return true;
            }
        }
    }

    return false;
}

float Barrier::distanceFrom(const Ray& ray) const
{
    // @note Panes are infinitely high, even if they don't look like it.

    float minDistance = 0.f;

    for (const auto& pane : m_panes) {
        Ray planeRay;
        planeRay.origin = pane.p0.position;
        planeRay.direction = pane.normal;
        auto distance = intersectPlane(ray, planeRay);
        if (distance <= 0.f) continue;

        // Detect if within pane's limits
        auto hitPoint = ray.origin + distance * ray.direction;
        auto hitProjection = glm::dot(hitPoint - pane.p0.position, pane.direction);
        if (hitProjection < 0.f || hitProjection > pane.length2d) continue;

        // @todo Let user see about the hit point!

        if (minDistance == 0.f || distance < minDistance) {
            minDistance = distance;
        }
    }

    return minDistance;
}

// -----

void Barrier::powered(bool powered)
{
    if (m_powered == powered) return;
    m_powered = powered;

    animation().start(sill::AnimationFlag::MaterialUniform, *m_panesMaterial, "poweredRatio", 0.5f);
    animation().target(sill::AnimationFlag::MaterialUniform, *m_panesMaterial, "poweredRatio", (powered) ? 1.f : 0.f);
}

void Barrier::pickingBlocked(bool pickingBlocked)
{
    if (m_pickingBlocked == pickingBlocked) return;
    m_pickingBlocked = pickingBlocked;
}

void Barrier::teleportationBlocked(bool teleportationBlocked)
{
    if (m_teleportationBlocked == teleportationBlocked) return;
    m_teleportationBlocked = teleportationBlocked;
}

// -----

void Barrier::updateFromControlPoints()
{
    auto& meshComponent = m_entity->get<sill::MeshComponent>();
    meshComponent.removeNodes();

    if (m_controlPoints.empty()) return;

    for (auto& controlPoint : m_controlPoints) {
        // @fixme We're not taking the absolute control points coordinates.
        // => The barrier is a movable object too.
        controlPoint.position = glm::vec3{controlPoint.point.x, controlPoint.point.y, 100.f};

        Ray ray;
        ray.origin = controlPoint.position;
        ray.direction = glm::vec3{0.f, 0.f, -1.f};
        controlPoint.position.z -= distanceToTerrain(m_gameState, ray, nullptr);

        // Cube is placed at the controls points
        sill::makers::boxMeshMaker(0.125f)(meshComponent);
        meshComponent.nodes().back().transform(glm::translate(glm::mat4(1.f), controlPoint.position));
    }

    m_panes.clear();
    m_panes.reserve(m_controlPoints.size() - 1u);
    for (auto i = 0u; i < m_controlPoints.size() - 1u; ++i) {
        m_panes.emplace_back(m_controlPoints[i], m_controlPoints[i + 1]);
    }

    // Creating panes between control points
    std::vector<glm::vec3> positions(4u);
    std::vector<glm::vec2> uvs(4u);
    std::vector<uint16_t> indices;

    uvs[0u] = glm::vec2{0.f, 0.f};
    uvs[1u] = glm::vec2{1.f, 0.f};
    uvs[2u] = glm::vec2{1.f, 1.f};
    uvs[3u] = glm::vec2{0.f, 1.f};

    indices.reserve(12u);
    indices.emplace_back(0u);
    indices.emplace_back(1u);
    indices.emplace_back(2u);
    indices.emplace_back(2u);
    indices.emplace_back(3u);
    indices.emplace_back(0u);

    // Copy indices for double sided panes
    for (auto i = 0u; i < 6u; i += 3) {
        indices.emplace_back(indices[i + 1]);
        indices.emplace_back(indices[i]);
        indices.emplace_back(indices[i + 2]);
    }

    for (const auto& pane : m_panes) {
        constexpr const auto paneHeight = 4.f;
        positions[0u] = pane.p0.position + glm::vec3{0.f, 0.f, -1.f};
        positions[1u] = pane.p1.position + glm::vec3{0.f, 0.f, -1.f};
        positions[2u] = pane.p1.position + glm::vec3{0.f, 0.f, paneHeight};
        positions[3u] = pane.p0.position + glm::vec3{0.f, 0.f, paneHeight};

        auto meshGroup = std::make_unique<sill::MeshGroup>(meshComponent.scene());
        auto& primitive = meshGroup->addPrimitive();
        primitive.verticesCount(positions.size());
        primitive.verticesPositions(positions);
        primitive.verticesUvs(uvs);
        primitive.indices(indices);
        primitive.material(m_panesMaterial);
        primitive.category(RenderCategory::Translucent);

        auto& node = meshComponent.addNode();
        node.meshGroup = std::move(meshGroup);
    }
}

// -----

Barrier* findBarrierByName(GameState& gameState, const std::string& name)
{
    for (auto barrier : gameState.level.barriers) {
        if (barrier->name() == name) {
            return barrier;
        }
    }

    return nullptr;
}

Barrier* findBarrier(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto barrier : gameState.level.barriers) {
        if (&barrier->entity() == &entity) {
            return barrier;
        }
    }

    return nullptr;
}

uint32_t findBarrierIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (uint32_t i = 0u; i < gameState.level.barriers.size(); ++i) {
        auto barrier = gameState.level.barriers[i];
        if (&barrier->entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
