#pragma once

#include <lava/chamber/lexer.hpp>
#include <lava/core/filesystem.hpp>
#include <unordered_map>

#include "./uniform.hpp"

namespace lava::magma {
    /// Converts a .shmag file to ShaderManager 'impl' standards.
    class ShmagReader {
    public:
        ShmagReader(const fs::Path& shaderPath);

        /// The shader processed as a string.
        const std::string& processedString() const { return m_processedString; }

        /// The extract uniform definitions.
        const UniformDefinitions& uniformDefinitions() const { return m_uniformDefinitions; }

        /// Whether the parse has errored (warnings logged).
        bool errored() const { return m_errorsCount > 0u; }

    protected:
        enum class GBufferType {
            Unknown,
            Float,
            Vec3,
            NormalizedVec3,
            Vec4,
        };

        enum class GBufferRange {
            Unknown,
            e8,
            e16,
            e32,
            e64,
        };

        struct GBufferDeclaration {
            GBufferType type;
            GBufferRange range = GBufferRange::e32;
            std::string name;
        };

        using GBufferDeclarations = std::vector<GBufferDeclaration>;

    protected:
        // GBuffer
        void parseGBuffer();
        void parseGBufferDeclarations();
        GBufferDeclaration parseGBufferDeclaration();

        // Geometry
        void parseGeometry(std::stringstream& adaptedCode);
        UniformDefinitions parseGeometryUniform();
        UniformDefinition parseGeometryUniformDefinition();
        void parseGeometryMain(std::stringstream& adaptedCode);

        void injectGeometryUniformDefinitions(std::stringstream& adaptedCode);
        void injectGeometryGBufferDataInsertion(std::stringstream& adaptedCode);

        // Epiphany
        void parseEpiphany(std::stringstream& adaptedCode);
        void parseEpiphanyBlock(std::stringstream& adaptedCode, bool expectSemicolon = false);
        void parseEpiphanyMain(std::stringstream& adaptedCode);

        void injectEpiphanyGBufferDataExtraction(std::stringstream& adaptedCode);

        // Common
        void injectGBufferDefinitions(std::stringstream& adaptedCode);

        void parseToken(chamber::TokenType tokenType);
        void parseIdentifier(const std::string& expectedIdentifier);
        std::string parseIdentifier();
        float parseFloat();
        std::string parseString();
        glm::vec4 parseVec4();

        std::string parseCurrentIdentifier();

        std::optional<chamber::Lexer::Token> getNotToken(chamber::TokenType tokenType);

        // Errors
        void errorExpected(const std::string& expectedChoices);
        void errorExpected(chamber::TokenType expectedTokenType);

    private:
        std::string m_processedString;
        UniformDefinitions m_uniformDefinitions;

        // Resources
        fs::Path m_path;
        std::unique_ptr<chamber::Lexer> m_lexer;
        GBufferDeclarations m_gBufferDeclarations;
        std::unordered_map<std::string, std::string> m_samplersMap;

        uint32_t m_errorsCount = 0u;
    };
}
