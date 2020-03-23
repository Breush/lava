#include "./shmag-reader.hpp"

// @note Due to windows.h leaking so much BS into global namespace,
// we cannot rely on using namespace lava::chamber.
using namespace lava;

using namespace lava::chamber;
using namespace lava::magma;

namespace {
    static uint32_t g_globalBasicOffset = 0u;
    static uint32_t g_globalTextureOffset = 0u;
}

ShmagReader::ShmagReader(const fs::Path& shaderPath)
    : m_path(shaderPath)
{
    std::ifstream fileStream(shaderPath.string());
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    auto code = buffer.str();

    m_lexer = std::make_unique<Lexer>(code);

    std::stringstream adaptedCode;
    while (auto token = m_lexer->nextToken()) {
        auto context = parseCurrentIdentifier();

        if (context == "struct") {
            parseGBuffer();

            injectGBufferDefinitions(adaptedCode);
        }
        else if (context == "global") {
            parseIdentifier("uniform");
            m_globalUniformDefinitions = parseUniform(g_globalBasicOffset, g_globalTextureOffset);

            injectGlobalUniformDefinitions(adaptedCode);
        }
        else if (context == "geometry") {
            parseGeometry(adaptedCode);
        }
        else if (context == "epiphany") {
            parseEpiphany(adaptedCode);
        }
        else {
            errorExpected("context: struct (gBuffer), global uniform, geometry, epiphany");
        }
    }

    if (m_errorsCount == 0u) {
        m_processedString = adaptedCode.str();
    }
}

// ----- Global

void ShmagReader::parseGBuffer()
{
    parseGBufferDeclarations();

    parseIdentifier("gBuffer");
    parseToken(chamber::TokenType::Semicolon);
}

void ShmagReader::parseGBufferDeclarations()
{
    parseToken(chamber::TokenType::LeftBrace);

    m_gBufferDeclarations.clear();
    while (auto token = getNotToken(chamber::TokenType::RightBrace)) {
        auto gBufferDeclaration = parseGBufferDeclaration();
        m_gBufferDeclarations.emplace_back(gBufferDeclaration);
    }
}

ShmagReader::GBufferDeclaration ShmagReader::parseGBufferDeclaration()
{
    GBufferDeclaration gBufferDeclaration;

    // Type
    auto type = parseCurrentIdentifier();

    if (type == "bool") {
        gBufferDeclaration.type = GBufferType::Bool;
    }
    else if (type == "float") {
        gBufferDeclaration.type = GBufferType::Float;
    }
    else if (type == "vec2") {
        gBufferDeclaration.type = GBufferType::Vec2;
    }
    else if (type == "vec3") {
        gBufferDeclaration.type = GBufferType::Vec3;
    }
    else if (type == "nvec3") {
        gBufferDeclaration.type = GBufferType::NormalizedVec3;
    }
    else if (type == "vec4") {
        gBufferDeclaration.type = GBufferType::Vec4;
    }
    else {
        errorExpected("type: bool, float, vec3, nvec3, vec4");
    }

    // Optional range
    auto token = m_lexer->nextToken();
    if (token->type == chamber::TokenType::LeftParenthesis) {
        auto range = parseFloat();

        if (range == 8.f)
            gBufferDeclaration.range = GBufferRange::e8;
        else if (range == 16.f)
            gBufferDeclaration.range = GBufferRange::e16;
        else if (range == 32.f)
            gBufferDeclaration.range = GBufferRange::e32;
        else if (range == 64.f)
            gBufferDeclaration.range = GBufferRange::e64;
        else {
            errorExpected("bitsize: 8, 16, 32, 64");
        }

        parseToken(chamber::TokenType::RightParenthesis);
        token = m_lexer->nextToken();
    }

    // Name
    gBufferDeclaration.name = parseCurrentIdentifier();

    // Semicolon
    parseToken(chamber::TokenType::Semicolon);

    return gBufferDeclaration;
}

UniformDefinitions ShmagReader::parseUniform(uint32_t& baseOffset, uint32_t& textureOffset)
{
    UniformDefinitions uniformDefinitions;

    parseToken(chamber::TokenType::LeftBrace);

    while (auto token = getNotToken(chamber::TokenType::RightBrace)) {
        auto uniformDefinition = parseUniformDefinition(baseOffset, textureOffset);
        uniformDefinitions.emplace_back(uniformDefinition);
    }

    parseToken(chamber::TokenType::Semicolon);

    return uniformDefinitions;
}

UniformDefinition ShmagReader::parseUniformDefinition(uint32_t& baseOffset, uint32_t& textureOffset)
{
    UniformDefinition uniformDefinition;

    // Type
    auto type = parseCurrentIdentifier();

    if (type == "float") {
        uniformDefinition.type = UniformType::Float;
        uniformDefinition.offset = baseOffset++;
    }
    else if (type == "uint") {
        uniformDefinition.type = UniformType::Uint;
        uniformDefinition.offset = baseOffset++;
    }
    else if (type == "bool") {
        uniformDefinition.type = UniformType::Bool;
        uniformDefinition.offset = baseOffset++;
    }
    else if (type == "vec2") {
        uniformDefinition.type = UniformType::Vec2;
        uniformDefinition.offset = baseOffset++;
    }
    else if (type == "vec3") {
        uniformDefinition.type = UniformType::Vec3;
        uniformDefinition.offset = baseOffset++;
    }
    else if (type == "vec4") {
        uniformDefinition.type = UniformType::Vec4;
        uniformDefinition.offset = baseOffset++;
    }
    else if (type == "texture2d") {
        uniformDefinition.type = UniformType::Texture;
        uniformDefinition.offset = textureOffset++;
    }
    else if (type == "textureCube") {
        uniformDefinition.type = UniformType::CubeTexture;
        uniformDefinition.offset = 0u; // @fixme Currently handling only one cube texture.
    }
    else {
        errorExpected("type: bool, uint, float, vec2, vec3, vec4, texture2d, textureCube");
    }

    // Name
    uniformDefinition.name = parseIdentifier();
    uniformDefinition.arraySize = parseArraySize();

    if (uniformDefinition.arraySize > 0u) {
        if (uniformDefinition.type == UniformType::Uint) {
            baseOffset += (uniformDefinition.arraySize - 1u);
        }
        else {
            logger.error("magma.shmag-reader") << "Unhandled array on types that are not: uint." << std::endl;
        }
    }

    // Default value
    if (m_lexer->currentToken()->type == chamber::TokenType::Equal) {
        switch (uniformDefinition.type) {
        case UniformType::Float: uniformDefinition.fallback.floatValue = parseFloat(); break;
        case UniformType::Uint: uniformDefinition.fallback.uintValue = parseUint(); break;
        case UniformType::Bool: uniformDefinition.fallback.uintValue = parseBool(); break;
        case UniformType::Vec2: uniformDefinition.fallback.vec2Value = parseVec2(); break;
        case UniformType::Vec3: uniformDefinition.fallback.vec3Value = parseVec3(); break;
        case UniformType::Vec4: uniformDefinition.fallback.vec4Value = parseVec4(); break;
        case UniformType::Texture:
        case UniformType::CubeTexture: {
            auto textureType = parseString();

            if (textureType == "white") {
                uniformDefinition.fallback.textureTypeValue = UniformTextureType::White;
            }
            else if (textureType == "normal") {
                uniformDefinition.fallback.textureTypeValue = UniformTextureType::Normal;
            }
            else if (textureType == "invisible") {
                uniformDefinition.fallback.textureTypeValue = UniformTextureType::Invisible;
            }
            else {
                errorExpected("texture type: \"white\", \"normal\", \"invisible\"");
            }
            break;
        }
        default: break;
        }

        // Semicolon
        parseToken(chamber::TokenType::Semicolon);
    }
    else if (m_lexer->currentToken()->type == chamber::TokenType::Semicolon) {
        // We clean the biggest member, so that everything is defaulted to 0
        memset(&uniformDefinition.fallback.uintArrayValue, 0, sizeof(UniformFallback));
    }
    else {
        errorExpected(chamber::TokenType::Semicolon);
    }

    return uniformDefinition;
}

// ----- Geometry

void ShmagReader::parseGeometry(std::stringstream& adaptedCode)
{
    adaptedCode << "@magma:impl:begin geometry" << std::endl;

    parseToken(chamber::TokenType::LeftBrace);

    // Optional gBuffer definition
    while (auto token = getNotToken(chamber::TokenType::RightBrace)) {
        auto firstIdentifier = parseCurrentIdentifier();

        if (firstIdentifier == "uniform") {
            uint32_t baseOffset = 0u;
            uint32_t textureOffset = 0u;
            m_uniformDefinitions = parseUniform(baseOffset, textureOffset);
        }
        else if (firstIdentifier == "bool") {
            parseIdentifier("main");
            parseGeometryMain(adaptedCode);
        }
        else if (firstIdentifier == "float") {
            adaptedCode << firstIdentifier << " ";
            parseBlock(adaptedCode);
        }
        else {
            errorExpected("uniform, declaration, definition");
        }
    }

    adaptedCode << "@magma:impl:end geometry" << std::endl;
}

void ShmagReader::parseGeometryMain(std::stringstream& adaptedCode)
{
    parseToken(chamber::TokenType::LeftParenthesis);
    parseToken(chamber::TokenType::RightParenthesis);
    parseToken(chamber::TokenType::LeftBrace);

    adaptedCode << "bool @magma:impl:main (out GBufferData gBufferData, float fragmentDepth) {" << std::endl;

    // ----- Inject global definitions

    m_samplersMap.clear();
    m_samplerCubeName = "";

    injectGlobalUniformDefinitions(adaptedCode);
    injectGBufferDefinitions(adaptedCode);

    // ----- Inject uniform definitions

    injectGeometryUniformDefinitions(adaptedCode);

    // ----- Remap original code

    // @todo These are fixed?
    std::unordered_map<std::string, std::string> inMap = {{"uv", "inUv"}, {"cubeUvw", "inCubeUvw"}, {"tbn", "inTbn"}};

    adaptedCode << "// [shmag-reader] Remapped original geometry code." << std::endl;
    remapBlock(adaptedCode, inMap, [&]() { injectGeometryGBufferDataInsertion(adaptedCode); });
}

void ShmagReader::injectGeometryUniformDefinitions(std::stringstream& adaptedCode)
{
    adaptedCode << "// [shmag-reader] Injected uniform definitions." << std::endl;
    for (auto& uniformDefinition : m_uniformDefinitions) {
        const auto& name = uniformDefinition.name;
        auto offset = uniformDefinition.offset;

        if (uniformDefinition.type == UniformType::Texture) {
            auto sampler = "materialSamplers[" + std::to_string(m_samplersMap.size()) + "]";
            adaptedCode << "// sampler2D " << name << " = " << sampler << ";" << std::endl;
            m_samplersMap[name] = sampler;
        }
        else if (uniformDefinition.type == UniformType::CubeTexture) {
            adaptedCode << "// samplerCube " << name << " = materialCubeSamplers0;" << std::endl;
            m_samplerCubeName = name;
        }
        else if (uniformDefinition.type == UniformType::Vec2) {
            adaptedCode << "vec2 " << name << ";" << std::endl;
            adaptedCode << name << "[0] = uintBitsToFloat(material.data[" << offset << "][0]);" << std::endl;
            adaptedCode << name << "[1] = uintBitsToFloat(material.data[" << offset << "][1]);" << std::endl;
        }
        else if (uniformDefinition.type == UniformType::Vec3) {
            adaptedCode << "vec3 " << name << ";" << std::endl;
            adaptedCode << name << "[0] = uintBitsToFloat(material.data[" << offset << "][0]);" << std::endl;
            adaptedCode << name << "[1] = uintBitsToFloat(material.data[" << offset << "][1]);" << std::endl;
            adaptedCode << name << "[2] = uintBitsToFloat(material.data[" << offset << "][2]);" << std::endl;
        }
        else if (uniformDefinition.type == UniformType::Vec4) {
            adaptedCode << "vec4 " << name << ";" << std::endl;
            adaptedCode << name << "[0] = uintBitsToFloat(material.data[" << offset << "][0]);" << std::endl;
            adaptedCode << name << "[1] = uintBitsToFloat(material.data[" << offset << "][1]);" << std::endl;
            adaptedCode << name << "[2] = uintBitsToFloat(material.data[" << offset << "][2]);" << std::endl;
            adaptedCode << name << "[3] = uintBitsToFloat(material.data[" << offset << "][3]);" << std::endl;
        }
        else if (uniformDefinition.type == UniformType::Float) {
            adaptedCode << "float " << name << " = uintBitsToFloat(material.data[" << offset << "][0]);" << std::endl;
        }
        else if (uniformDefinition.type == UniformType::Uint) {
            if (uniformDefinition.arraySize == 0u) {
                adaptedCode << "uint " << name << " = material.data[" << offset << "][0];" << std::endl;
            }
            else {
                adaptedCode << "uint " << name << "[" << uniformDefinition.arraySize << "];" << std::endl;
                for (auto i = 0u; i < uniformDefinition.arraySize; i += 4u) {
                    adaptedCode << name << "[" << i << "] = material.data[" << offset << "][0];" << std::endl;
                    if (i + 1 < uniformDefinition.arraySize)
                        adaptedCode << name << "[" << i + 1 << "]  = material.data[" << offset << "][1];" << std::endl;
                    if (i + 2 < uniformDefinition.arraySize)
                        adaptedCode << name << "[" << i + 2 << "]  = material.data[" << offset << "][2];" << std::endl;
                    if (i + 3 < uniformDefinition.arraySize)
                        adaptedCode << name << "[" << i + 3 << "]  = material.data[" << offset << "][3];" << std::endl;
                    offset += 1u;
                }
            }
        }
        else if (uniformDefinition.type == UniformType::Bool) {
            adaptedCode << "bool " << name << " = (material.data[" << offset << "][0] != 0);" << std::endl;
        }
        else {
            logger.error("magma.shmag-reader") << "Unhandled uniform type." << std::endl;
        }
    }
    adaptedCode << std::endl;
}

void ShmagReader::injectGeometryGBufferDataInsertion(std::stringstream& adaptedCode)
{
    uint16_t dataOffset = 0u;

    // @todo Handle ranges to reduce size by a lot!

    adaptedCode << std::endl;
    adaptedCode << "// [shmag-reader] Injected final G-Buffer data insertion." << std::endl;
    for (auto& gBufferDeclaration : m_gBufferDeclarations) {
        auto name = "gBuffer." + gBufferDeclaration.name;
        if (gBufferDeclaration.type == GBufferType::Bool) {
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = (" << name << ") ? 1 : 0;" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::Float) {
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << ");" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::Vec2) {
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[0]);" << std::endl;
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[1]);" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::Vec3) {
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[0]);" << std::endl;
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[1]);" << std::endl;
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[2]);" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::NormalizedVec3) {
            // Storing spherical coordinates
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(acos(" << name << "[2]));" << std::endl;
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(atan(" << name << "[1]"
                        << ", " << name << "[0]));" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::Vec4) {
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[0]);" << std::endl;
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[1]);" << std::endl;
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[2]);" << std::endl;
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << "[3]);" << std::endl;
        }
        else {
            logger.error("magma.shmag-reader") << "Unhandled G-Buffer declaration type." << std::endl;
        }
    }
    adaptedCode << std::endl;
}

// ----- Epiphany

void ShmagReader::parseEpiphany(std::stringstream& adaptedCode)
{
    adaptedCode << "@magma:impl:begin epiphany" << std::endl;

    parseToken(chamber::TokenType::LeftBrace);

    // Optional gBuffer definition
    while (auto token = getNotToken(chamber::TokenType::RightBrace)) {
        auto firstIdentifier = parseCurrentIdentifier();
        adaptedCode << firstIdentifier << " ";

        if (firstIdentifier == "vec4") {
            auto secondIdentifier = parseIdentifier();
            if (secondIdentifier == "main") {
                parseEpiphanyMain(adaptedCode);
            }
            else {
                adaptedCode << secondIdentifier << " ";
                parseBlock(adaptedCode);
            }
        }
        else if (firstIdentifier == "struct") {
            parseBlock(adaptedCode, true);
        }
        else if (firstIdentifier == "float" || firstIdentifier == "vec3") {
            parseBlock(adaptedCode);
        }
        else {
            errorExpected("declaration, definition");
        }
    }

    adaptedCode << "@magma:impl:end epiphany" << std::endl;
}

void ShmagReader::parseBlock(std::stringstream& adaptedCode, bool expectSemicolon)
{
    while (auto token = getNotToken(chamber::TokenType::LeftBrace)) {
        adaptedCode << token->string << " ";

        // It might be a function declaration, with a semicolon and not braces,
        // so we end there.
        if (token->type == chamber::TokenType::Semicolon) {
            adaptedCode << std::endl;
            return;
        }
    }
    adaptedCode << "{" << std::endl;

    auto bracesCount = 1u;
    while (auto token = m_lexer->nextToken()) {
        adaptedCode << token->string << " ";

        // @todo Make a better pretty print? (Using it in shader-manager too.)
        if (token->type == chamber::TokenType::Semicolon) {
            adaptedCode << std::endl;
        }
        else if (token->type == chamber::TokenType::LeftBrace) {
            adaptedCode << std::endl;
            bracesCount++;
        }
        else if (token->type == chamber::TokenType::RightBrace) {
            bracesCount--;
            if (bracesCount == 0u && expectSemicolon) break;
            adaptedCode << std::endl << std::endl;
        }

        if (bracesCount == 0u) break;
    }

    if (expectSemicolon) {
        parseToken(chamber::TokenType::Semicolon);
        adaptedCode << ";" << std::endl << std::endl;
    }
}

void ShmagReader::parseEpiphanyMain(std::stringstream& adaptedCode)
{
    parseToken(chamber::TokenType::LeftParenthesis);
    parseToken(chamber::TokenType::RightParenthesis);
    parseToken(chamber::TokenType::LeftBrace);

    adaptedCode << "@magma:impl:main (GBufferData gBufferData, float fragmentDepth) {" << std::endl;
    adaptedCode << "    vec2 fragmentPosition = gl_FragCoord.xy / vec2(camera.extent);" << std::endl;

    // ----- Inject global definitions

    m_samplersMap.clear();
    m_samplerCubeName = "";

    injectGlobalUniformDefinitions(adaptedCode);
    injectGBufferDefinitions(adaptedCode);

    // ----- Inject node extraction

    injectEpiphanyGBufferDataExtraction(adaptedCode);

    // ----- Remap original code

    adaptedCode << "// [shmag-reader] Remapped original epiphany code." << std::endl;
    remapBlock(adaptedCode, {}, []() {});
}

void ShmagReader::injectEpiphanyGBufferDataExtraction(std::stringstream& adaptedCode)
{
    uint16_t dataOffset = 0u;

    // @todo Handle ranges!

    adaptedCode << "// [shmag-reader] Injected G-Buffer data extraction." << std::endl;
    for (auto& gBufferDeclaration : m_gBufferDeclarations) {
        auto name = "gBuffer." + gBufferDeclaration.name;
        if (gBufferDeclaration.type == GBufferType::Bool) {
            adaptedCode << name << " = (gBufferData.data[" << dataOffset++ << "] == 1) ? true : false;" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::Float) {
            adaptedCode << name << " = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::Vec2) {
            adaptedCode << name << "[0] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << name << "[1] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::Vec3) {
            adaptedCode << name << "[0] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << name << "[1] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << name << "[2] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::NormalizedVec3) {
            // From spherical coordinates
            auto phi = "gBuffer_" + gBufferDeclaration.name + "_phi";
            auto theta = "gBuffer_" + gBufferDeclaration.name + "_theta";
            adaptedCode << "float " << theta << " = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << "float " << phi << " = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << name << "[0] = sin(" << theta << ") * cos(" << phi << ");" << std::endl;
            adaptedCode << name << "[1] = sin(" << theta << ") * sin(" << phi << ");" << std::endl;
            adaptedCode << name << "[2] = cos(" << theta << ");" << std::endl;
        }
        else if (gBufferDeclaration.type == GBufferType::Vec4) {
            adaptedCode << name << "[0] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << name << "[1] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << name << "[2] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << name << "[3] = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
        }
        else {
            logger.error("magma.shmag-reader") << "Unhandled G-Buffer declaration type." << std::endl;
        }
    }
    adaptedCode << std::endl;
}

// ----- Common

void ShmagReader::injectGlobalUniformDefinitions(std::stringstream& adaptedCode)
{
    adaptedCode << "// [shmag-reader] Injected global uniform definitions." << std::endl;

    for (auto& uniformDefinition : m_globalUniformDefinitions) {
        const auto& name = uniformDefinition.name;

        if (uniformDefinition.type == UniformType::Texture) {
            auto sampler = "materialGlobalSamplers[" + std::to_string(uniformDefinition.offset) + "]";
            adaptedCode << "// sampler2D " << name << " = " << sampler << ";" << std::endl;
            m_samplersMap[name] = sampler;
        }
        else {
            logger.error("magma.shmag-reader") << "Unhandled global uniform type." << std::endl;
        }
    }
    adaptedCode << std::endl;
}

void ShmagReader::injectGBufferDefinitions(std::stringstream& adaptedCode)
{
    adaptedCode << "// [shmag-reader] Injected G-Buffer definitions." << std::endl;
    adaptedCode << "struct {" << std::endl;
    for (auto& gBufferDeclaration : m_gBufferDeclarations) {
        if (gBufferDeclaration.type == GBufferType::Bool)
            adaptedCode << "bool " << gBufferDeclaration.name << ";" << std::endl;
        else if (gBufferDeclaration.type == GBufferType::Float)
            adaptedCode << "float " << gBufferDeclaration.name << ";" << std::endl;
        else if (gBufferDeclaration.type == GBufferType::Vec2)
            adaptedCode << "vec2 " << gBufferDeclaration.name << ";" << std::endl;
        else if (gBufferDeclaration.type == GBufferType::Vec3)
            adaptedCode << "vec3 " << gBufferDeclaration.name << ";" << std::endl;
        else if (gBufferDeclaration.type == GBufferType::NormalizedVec3)
            adaptedCode << "vec3 " << gBufferDeclaration.name << ";" << std::endl;
        else if (gBufferDeclaration.type == GBufferType::Vec4)
            adaptedCode << "vec4 " << gBufferDeclaration.name << ";" << std::endl;
        else
            logger.error("magma.shmag-reader") << "Unhandled G-Buffer declaration type." << std::endl;
    }
    adaptedCode << "} gBuffer;" << std::endl << std::endl;
}

void ShmagReader::parseToken(chamber::TokenType tokenType)
{
    auto token = m_lexer->nextToken();
    if (token->type != tokenType) {
        errorExpected(tokenType);
    }
}

void ShmagReader::parseIdentifier(const std::string& expectedIdentifier)
{
    auto token = m_lexer->nextToken();
    if (token->type != chamber::TokenType::Identifier || token->string != expectedIdentifier) {
        errorExpected("'" + expectedIdentifier + "'");
    }
}

std::string ShmagReader::parseIdentifier()
{
    auto token = m_lexer->nextToken();
    return parseCurrentIdentifier();
}

uint32_t ShmagReader::parseArraySize()
{
    auto token = m_lexer->nextToken();
    if (token->type != chamber::TokenType::LeftBracket) {
        return 0u;
    }

    auto number = parseUint();

    parseToken(chamber::TokenType::RightBracket);

    token = m_lexer->nextToken();
    return number;
}

uint32_t ShmagReader::parseBool()
{
    auto token = m_lexer->nextToken();
    if (token->type != chamber::TokenType::Identifier) {
        errorExpected(chamber::TokenType::Identifier);
    }
    return (token->string == "false") ? 0 : 1;
}

uint32_t ShmagReader::parseUint()
{
    auto token = m_lexer->nextToken();
    if (token->type != chamber::TokenType::Number) {
        errorExpected(chamber::TokenType::Number);
    }
    return static_cast<uint32_t>(token->number);
}

float ShmagReader::parseFloat()
{
    auto token = m_lexer->nextToken();
    if (token->type != chamber::TokenType::Number) {
        errorExpected(chamber::TokenType::Number);
    }
    return static_cast<float>(token->number);
}

std::string ShmagReader::parseString()
{
    auto token = m_lexer->nextToken();
    if (token->type != chamber::TokenType::String) {
        errorExpected(chamber::TokenType::String);
    }
    return token->string;
}

glm::vec2 ShmagReader::parseVec2()
{
    glm::vec2 vector;

    parseIdentifier("vec2");
    parseToken(chamber::TokenType::LeftParenthesis);

    vector[0u] = parseFloat();
    parseToken(chamber::TokenType::Comma);
    vector[1u] = parseFloat();

    parseToken(chamber::TokenType::RightParenthesis);

    return vector;
}

glm::vec3 ShmagReader::parseVec3()
{
    glm::vec3 vector;

    parseIdentifier("vec3");
    parseToken(chamber::TokenType::LeftParenthesis);

    for (auto i = 0u; i < 2u; ++i) {
        vector[i] = parseFloat();
        parseToken(chamber::TokenType::Comma);
    }
    vector[2u] = parseFloat();

    parseToken(chamber::TokenType::RightParenthesis);

    return vector;
}

glm::vec4 ShmagReader::parseVec4()
{
    glm::vec4 vector;

    parseIdentifier("vec4");
    parseToken(chamber::TokenType::LeftParenthesis);

    for (auto i = 0u; i < 3u; ++i) {
        vector[i] = parseFloat();
        parseToken(chamber::TokenType::Comma);
    }
    vector[3u] = parseFloat();

    parseToken(chamber::TokenType::RightParenthesis);

    return vector;
}

std::optional<Lexer::Token> ShmagReader::getNotToken(chamber::TokenType tokenType)
{
    auto token = m_lexer->nextToken();
    return (token->type != tokenType) ? token : std::nullopt;
}

std::string ShmagReader::parseCurrentIdentifier()
{
    auto token = m_lexer->currentToken();
    if (token->type != chamber::TokenType::Identifier) {
        errorExpected(chamber::TokenType::Identifier);
    }
    return token->string;
}

void ShmagReader::remapBlock(std::stringstream& adaptedCode, const std::unordered_map<std::string, std::string>& extraMap,
                             std::function<void(void)> onReturn)
{
    auto bracesCount = 1u;
    auto tokensCountBeforeEndl = 0u;
    while (auto token = m_lexer->nextToken()) {
        auto tokenString = token->string;
        if (token->type == chamber::TokenType::Identifier) {
            if (m_samplersMap.find(token->string) != m_samplersMap.end())
                tokenString = m_samplersMap[token->string];
            else if (m_samplerCubeName == token->string)
                tokenString = "materialCubeSamplers0";
            else if (extraMap.find(token->string) != extraMap.end())
                tokenString = extraMap.at(token->string);
            else if (token->string == "return")
                onReturn();
        }
        else if (token->type == chamber::TokenType::Sharp) {
            // So that "#define name value" is followed by std::endl
            tokensCountBeforeEndl = 4;
        }

        adaptedCode << tokenString << " ";

        if (tokensCountBeforeEndl > 0u) {
            tokensCountBeforeEndl -= 1u;
            if (tokensCountBeforeEndl == 0u) {
                adaptedCode << std::endl;
            }
        }

        // @todo Make a better pretty print? (Using it in shader-manager too.)
        if (token->type == chamber::TokenType::Semicolon) {
            adaptedCode << std::endl;
        }
        else if (token->type == chamber::TokenType::LeftBrace) {
            adaptedCode << std::endl;
            bracesCount++;
        }
        else if (token->type == chamber::TokenType::RightBrace) {
            adaptedCode << std::endl << std::endl;
            bracesCount--;
        }

        if (bracesCount == 0u) break;
    }
}

// ----- Errors

void ShmagReader::errorExpected(const std::string& expectedChoices)
{
    if (m_errorsCount == 1u) return;
    m_errorsCount += 1u;

    auto token = m_lexer->currentToken();
    auto tokenContext = m_lexer->currentTokenContext();

    logger.warning("magma.shmag-reader") << "In file " << m_path.c_str() << ":" << tokenContext.line << "." << std::endl;
    if (token) {
        logger.warning("magma.shmag-reader") << "Unexpected token " << token->type << " '" << token->string << "'." << std::endl;
    }
    logger.warning("magma.shmag-reader") << "Expected " << expectedChoices << "." << std::endl;
}

void ShmagReader::errorExpected(chamber::TokenType expectedTokenType)
{
    errorExpected(stringify(expectedTokenType));
}
