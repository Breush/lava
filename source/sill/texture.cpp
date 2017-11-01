#include <lava/sill/texture.hpp>

#include <lava/chamber/macros.hpp>

#include "./texture-impl.hpp"

using namespace lava::sill;

$pimpl_class(Texture, GameEngine&, engine);

Texture::Texture(GameEngine& engine, const std::string& imagePath)
    : Texture(engine)
{
    loadFromFile(imagePath);
}

// Loaders
$pimpl_method(Texture, void, loadFromFile, const std::string&, imagePath);
$pimpl_method(Texture, void, loadFromMemory, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height, uint8_t,
              channels);
