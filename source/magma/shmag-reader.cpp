#include "./shmag-reader.hpp"

using namespace lava::chamber;
using namespace lava::magma;

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
        }
        else if (context == "geometry") {
            parseGeometry(adaptedCode);
        }
        else if (context == "epiphany") {
            parseEpiphany(adaptedCode);
        }
        else {
            errorExpected("context: struct (gBuffer), geometry, epiphany");
        }
    }

    if (m_errorsCount == 0u) {
        m_processedString = adaptedCode.str();
    }
}

// ----- G-Buffer

void ShmagReader::parseGBuffer()
{
    parseGBufferDeclarations();

    parseIdentifier("gBuffer");
    parseToken(TokenType::Semicolon);
}

void ShmagReader::parseGBufferDeclarations()
{
    parseToken(TokenType::LeftBrace);

    m_gBufferDeclarations.clear();
    while (auto token = getNotToken(TokenType::RightBrace)) {
        auto gBufferDeclaration = parseGBufferDeclaration();
        m_gBufferDeclarations.emplace_back(gBufferDeclaration);
    }
}

ShmagReader::GBufferDeclaration ShmagReader::parseGBufferDeclaration()
{
    GBufferDeclaration gBufferDeclaration;

    // Type
    auto type = parseCurrentIdentifier();

    if (type == "float") {
        gBufferDeclaration.type = GBufferType::Float;
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
        errorExpected("type: float, vec3, nvec3, vec4");
    }

    // Optional range
    auto token = m_lexer->nextToken();
    if (token->type == TokenType::LeftParenthesis) {
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

        parseToken(TokenType::RightParenthesis);
        token = m_lexer->nextToken();
    }

    // Name
    gBufferDeclaration.name = parseCurrentIdentifier();

    // Semicolon
    parseToken(TokenType::Semicolon);

    return gBufferDeclaration;
}

// ----- Geometry

void ShmagReader::parseGeometry(std::stringstream& adaptedCode)
{
    adaptedCode << "@magma:impl:begin geometry" << std::endl;

    parseToken(TokenType::LeftBrace);

    // Optional gBuffer definition
    while (auto token = getNotToken(TokenType::RightBrace)) {
        auto firstIdentifier = parseCurrentIdentifier();

        if (firstIdentifier == "uniform") {
            m_uniformDefinitions = parseGeometryUniform();
        }
        else if (firstIdentifier == "bool") {
            parseIdentifier("main");
            parseGeometryMain(adaptedCode);
        }
        else {
            errorExpected("uniform, declaration, definition");
        }
    }

    adaptedCode << "@magma:impl:end geometry" << std::endl;
}

UniformDefinitions ShmagReader::parseGeometryUniform()
{
    UniformDefinitions uniformDefinitions;

    parseToken(TokenType::LeftBrace);

    while (auto token = getNotToken(TokenType::RightBrace)) {
        auto uniformDefinition = parseGeometryUniformDefinition();
        uniformDefinitions.emplace_back(uniformDefinition);
    }

    parseToken(TokenType::Semicolon);

    return uniformDefinitions;
}

UniformDefinition ShmagReader::parseGeometryUniformDefinition()
{
    UniformDefinition uniformDefinition;

    // Type
    auto type = parseCurrentIdentifier();

    if (type == "float") {
        uniformDefinition.type = UniformType::Float;
    }
    else if (type == "vec4") {
        uniformDefinition.type = UniformType::Vec4;
    }
    else if (type == "texture2d") {
        uniformDefinition.type = UniformType::Texture;
    }
    else {
        errorExpected("type: float, vec4, texture2d");
    }

    // Name
    uniformDefinition.name = parseIdentifier();

    // Default value
    parseToken(TokenType::Equal);

    switch (uniformDefinition.type) {
    case UniformType::Float: {
        uniformDefinition.fallback.floatValue = parseFloat();
        break;
    }
    case UniformType::Vec4: {
        uniformDefinition.fallback.vec4Value = parseVec4();
        break;
    }
    case UniformType::Texture: {
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
    parseToken(TokenType::Semicolon);

    return uniformDefinition;
}

void ShmagReader::parseGeometryMain(std::stringstream& adaptedCode)
{
    parseToken(TokenType::LeftParenthesis);
    parseToken(TokenType::RightParenthesis);
    parseToken(TokenType::LeftBrace);

    adaptedCode << "bool @magma:impl:main (out GBufferData gBufferData, float fragmentDepth) {" << std::endl;

    // ----- Inject G-Buffer definitions

    injectGBufferDefinitions(adaptedCode);

    // ----- Inject uniform definitions

    injectGeometryUniformDefinitions(adaptedCode);

    // ----- Remap original code

    // @todo These are fixed?
    std::unordered_map<std::string, std::string> inMap = {{"uv", "inUv"}, {"tbn", "inTbn"}};

    adaptedCode << "// [shmag-reader] Remapped original geometry code." << std::endl;
    auto bracesCount = 1u;
    while (auto token = m_lexer->nextToken()) {
        auto tokenString = token->string;
        if (token->type == TokenType::Identifier) {
            if (m_samplersMap.find(token->string) != m_samplersMap.end())
                tokenString = m_samplersMap[token->string];
            else if (inMap.find(token->string) != inMap.end())
                tokenString = inMap[token->string];
            else if (token->string == "return")
                injectGeometryGBufferDataInsertion(adaptedCode);
        }

        adaptedCode << tokenString << " ";

        // @todo Make a better pretty print? (Using it in shader-manager too.)
        if (token->type == TokenType::Semicolon) {
            adaptedCode << std::endl;
        }
        else if (token->type == TokenType::LeftBrace) {
            adaptedCode << std::endl;
            bracesCount++;
        }
        else if (token->type == TokenType::RightBrace) {
            adaptedCode << std::endl << std::endl;
            bracesCount--;
        }

        if (bracesCount == 0u) break;
    }
}

void ShmagReader::injectGeometryUniformDefinitions(std::stringstream& adaptedCode)
{
    m_samplersMap.clear();

    adaptedCode << "// [shmag-reader] Injected uniform definitions." << std::endl;
    uint16_t dataOffset = 0u;
    for (auto& uniformDefinition : m_uniformDefinitions) {
        if (uniformDefinition.type == UniformType::Texture) {
            auto sampler = "samplers[" + std::to_string(m_samplersMap.size()) + "]";
            adaptedCode << "// sampler2D " << uniformDefinition.name << " = " << sampler << ";" << std::endl;
            m_samplersMap[uniformDefinition.name] = sampler;
        }
        else if (uniformDefinition.type == UniformType::Vec4) {
            adaptedCode << "vec4 " << uniformDefinition.name << ";" << std::endl;
            adaptedCode << uniformDefinition.name << "[0] = uintBitsToFloat(material.data[" << dataOffset << "][0]);"
                        << std::endl;
            adaptedCode << uniformDefinition.name << "[1] = uintBitsToFloat(material.data[" << dataOffset << "][1]);"
                        << std::endl;
            adaptedCode << uniformDefinition.name << "[2] = uintBitsToFloat(material.data[" << dataOffset << "][2]);"
                        << std::endl;
            adaptedCode << uniformDefinition.name << "[3] = uintBitsToFloat(material.data[" << dataOffset << "][3]);"
                        << std::endl;
            dataOffset += 1u;
        }
        else if (uniformDefinition.type == UniformType::Float) {
            adaptedCode << "float " << uniformDefinition.name << " = uintBitsToFloat(material.data[" << dataOffset << "][0]);"
                        << std::endl;
            dataOffset += 1u;
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
        if (gBufferDeclaration.type == GBufferType::Float) {
            adaptedCode << "gBufferData.data[" << dataOffset++ << "] = floatBitsToUint(" << name << ");" << std::endl;
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

    parseToken(TokenType::LeftBrace);

    // Optional gBuffer definition
    while (auto token = getNotToken(TokenType::RightBrace)) {
        auto firstIdentifier = parseCurrentIdentifier();
        adaptedCode << firstIdentifier << " ";

        if (firstIdentifier == "vec4") {
            auto secondIdentifier = parseIdentifier();
            if (secondIdentifier == "main") {
                parseEpiphanyMain(adaptedCode);
            }
            else {
                adaptedCode << secondIdentifier << " ";
                parseEpiphanyBlock(adaptedCode);
            }
        }
        else if (firstIdentifier == "struct") {
            parseEpiphanyBlock(adaptedCode, true);
        }
        else if (firstIdentifier == "float" || firstIdentifier == "vec3") {
            parseEpiphanyBlock(adaptedCode);
        }
        else {
            errorExpected("declaration, definition");
        }
    }

    adaptedCode << "@magma:impl:end epiphany" << std::endl;
}

void ShmagReader::parseEpiphanyBlock(std::stringstream& adaptedCode, bool expectSemicolon)
{
    while (auto token = getNotToken(TokenType::LeftBrace)) {
        adaptedCode << token->string << " ";

        // It might be a function declaration, with a semicolon and not braces,
        // so we end there.
        if (token->type == TokenType::Semicolon) {
            adaptedCode << std::endl;
            return;
        }
    }
    adaptedCode << "{" << std::endl;

    auto bracesCount = 1u;
    while (auto token = m_lexer->nextToken()) {
        adaptedCode << token->string << " ";

        // @todo Make a better pretty print? (Using it in shader-manager too.)
        if (token->type == TokenType::Semicolon) {
            adaptedCode << std::endl;
        }
        else if (token->type == TokenType::LeftBrace) {
            adaptedCode << std::endl;
            bracesCount++;
        }
        else if (token->type == TokenType::RightBrace) {
            bracesCount--;
            if (bracesCount == 0u && expectSemicolon) break;
            adaptedCode << std::endl << std::endl;
        }

        if (bracesCount == 0u) break;
    }

    if (expectSemicolon) {
        parseToken(TokenType::Semicolon);
        adaptedCode << ";" << std::endl << std::endl;
    }
}

void ShmagReader::parseEpiphanyMain(std::stringstream& adaptedCode)
{
    parseToken(TokenType::LeftParenthesis);
    parseToken(TokenType::RightParenthesis);
    parseToken(TokenType::LeftBrace);

    adaptedCode << "@magma:impl:main (GBufferData gBufferData, float fragmentDepth) {" << std::endl;
    adaptedCode << "    vec2 fragmentPosition = gl_FragCoord.xy / vec2(camera.extent);" << std::endl;

    // ----- Inject G-Buffer definitions

    injectGBufferDefinitions(adaptedCode);

    // ----- Inject node extraction

    injectEpiphanyGBufferDataExtraction(adaptedCode);

    // ----- Remap original code

    adaptedCode << "// [shmag-reader] Remapped original epiphany code." << std::endl;
    auto bracesCount = 1u;
    while (auto token = m_lexer->nextToken()) {
        auto tokenString = token->string;
        adaptedCode << tokenString << " ";

        // @todo Make a better pretty print? (Using it in shader-manager too.)
        if (token->type == TokenType::Semicolon) {
            adaptedCode << std::endl;
        }
        else if (token->type == TokenType::LeftBrace) {
            adaptedCode << std::endl;
            bracesCount++;
        }
        else if (token->type == TokenType::RightBrace) {
            bracesCount--;
            adaptedCode << std::endl << std::endl;
        }

        if (bracesCount == 0u) break;
    }
}

void ShmagReader::injectEpiphanyGBufferDataExtraction(std::stringstream& adaptedCode)
{
    uint16_t dataOffset = 0u;

    // @todo Handle ranges!

    adaptedCode << "// [shmag-reader] Injected G-Buffer data extraction." << std::endl;
    for (auto& gBufferDeclaration : m_gBufferDeclarations) {
        auto name = "gBuffer." + gBufferDeclaration.name;
        if (gBufferDeclaration.type == GBufferType::Float) {
            adaptedCode << name << " = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
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
            adaptedCode << "float " << phi << " = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << "float " << theta << " = uintBitsToFloat(gBufferData.data[" << dataOffset++ << "]);" << std::endl;
            adaptedCode << name << "[0] = sin(" << phi << ") * cos(" << theta << ");" << std::endl;
            adaptedCode << name << "[1] = sin(" << phi << ") * sin(" << theta << ");" << std::endl;
            adaptedCode << name << "[2] = cos(" << phi << ");" << std::endl;
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

void ShmagReader::injectGBufferDefinitions(std::stringstream& adaptedCode)
{
    adaptedCode << "// [shmag-reader] Injected G-Buffer definitions." << std::endl;
    adaptedCode << "struct {" << std::endl;
    for (auto& gBufferDeclaration : m_gBufferDeclarations) {
        if (gBufferDeclaration.type == GBufferType::Float)
            adaptedCode << "float " << gBufferDeclaration.name << ";" << std::endl;
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

void ShmagReader::parseToken(TokenType tokenType)
{
    auto token = m_lexer->nextToken();
    if (token->type != tokenType) {
        errorExpected(tokenType);
    }
}

void ShmagReader::parseIdentifier(const std::string& expectedIdentifier)
{
    auto token = m_lexer->nextToken();
    if (token->type != TokenType::Identifier || token->string != expectedIdentifier) {
        errorExpected("'" + expectedIdentifier + "'");
    }
}

std::string ShmagReader::parseIdentifier()
{
    auto token = m_lexer->nextToken();
    return parseCurrentIdentifier();
}

float ShmagReader::parseFloat()
{
    auto token = m_lexer->nextToken();
    if (token->type != TokenType::Number) {
        errorExpected(TokenType::Number);
    }
    return static_cast<float>(token->number);
}

std::string ShmagReader::parseString()
{
    auto token = m_lexer->nextToken();
    if (token->type != TokenType::String) {
        errorExpected(TokenType::String);
    }
    return token->string;
}

glm::vec4 ShmagReader::parseVec4()
{
    glm::vec4 vector;

    parseIdentifier("vec4");
    parseToken(TokenType::LeftParenthesis);

    for (auto i = 0u; i < 3u; ++i) {
        vector[i] = parseFloat();
        parseToken(TokenType::Comma);
    }
    vector[3u] = parseFloat();

    parseToken(TokenType::RightParenthesis);

    return vector;
}

std::optional<Lexer::Token> ShmagReader::getNotToken(TokenType tokenType)
{
    auto token = m_lexer->nextToken();
    return (token->type != tokenType) ? token : std::nullopt;
}

std::string ShmagReader::parseCurrentIdentifier()
{
    auto token = m_lexer->currentToken();
    if (token->type != TokenType::Identifier) {
        errorExpected(TokenType::Identifier);
    }
    return token->string;
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

void ShmagReader::errorExpected(TokenType expectedTokenType)
{
    errorExpected(stringify(expectedTokenType));
}
