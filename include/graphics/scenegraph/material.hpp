#ifndef SG_MATERIAL
#define SG_MATERIAL

#include "exceptions/genexc.hpp"
#include "graphics/shader.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <exception>
#include <unordered_map>

namespace BlueBear {
  namespace Graphics {

    class Texture;

    namespace SceneGraph {

      using TextureList = std::vector< std::shared_ptr< Texture > >;

      class Material {
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        TextureList diffuseTextures;
        TextureList specularTextures;
        float shininess = 0.0f;
        float opacity = 1.0f;
        bool useAmbient = false;

        std::vector< unsigned int > lockedTextureUnits;
        struct MaterialUniforms {
          Shader::Uniform ambient;
          Shader::Uniform diffuse;
          Shader::Uniform specular;
          std::vector< Shader::Uniform > diffuseArray;
          std::vector< Shader::Uniform > specularArray;
          Shader::Uniform shininess;
          Shader::Uniform opacity;
        };
        std::unordered_map< const void*, MaterialUniforms > uniforms;
        int maxDiffuseTextures = 0;
        int maxSpecularTextures = 0;

        const MaterialUniforms& getMaterialUniforms( const Shader* shader );
        void checkTextureUnits();

      public:
        EXCEPTION_TYPE( ExceededTextureUnitsException, "Exceeded the maximum texture units for this hardware." );
        EXCEPTION_TYPE( TextureUnitUnavailableException, "Unable to acquire a texture unit for draw." );

        Material( glm::vec3 ambientColor, glm::vec3 diffuseColor, glm::vec3 specularColor, float shininess, float opacity );
        Material( glm::vec3 diffuseColor, glm::vec3 specularColor, float shininess, float opacity );

        Material( glm::vec3 ambientColor, TextureList diffuseTextures, TextureList specularTextures, float shininess, float opacity );
        Material( TextureList diffuseTextures, TextureList specularTextures, float shininess, float opacity );

        Material( glm::vec3 ambientColor, TextureList diffuseTextures, glm::vec3 specularColor, float shininess, float opacity );
        Material( glm::vec3 ambientColor, glm::vec3 diffuseColor, TextureList specularTextures, float shininess, float opacity );

        virtual ~Material() = default;

        const glm::vec3& getAmbientColor() const;
        const glm::vec3& getDiffuseColor() const;
        const glm::vec3& getSpecularColor() const;

        const TextureList& getDiffuseTextureList() const;
        const TextureList& getSpecularTextureList() const;

        float getShininess() const;
        float getOpacity() const;

        void sendDeferredTextures();

        void send( const Shader& shader );

        void releaseTextureUnits();
      };

    }
  }
}


#endif
