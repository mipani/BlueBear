#ifndef SG_RIGGED
#define SG_RIGGED

#include <glm/glm.hpp>
#include <memory>
#include <utility>
#include <string>

namespace BlueBear {
  namespace Graphics {
    class Shader;

    namespace SceneGraph {
      namespace Mesh {

        struct RiggedVertex {
          glm::vec3 position;
          glm::vec3 normal;
          glm::ivec4 boneIDs = glm::ivec4( 0, 0, 0, 0 );
          glm::vec4 boneWeights = glm::vec4( 1.0f, 0.0f, 0.0f, 0.0f );

          bool operator==( const RiggedVertex& rhs ) const;

          static void setupShaderAttributes();
          static std::pair< std::string, std::string > getDefaultShader();
        };

      }
    }
  }
}


#endif
