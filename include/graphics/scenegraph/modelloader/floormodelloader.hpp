#ifndef BB_FLOOR_MODEL_LOADER
#define BB_FLOOR_MODEL_LOADER

#include "graphics/scenegraph/modelloader/proceduralmodelloader.hpp"
#include "graphics/scenegraph/mesh/texturedvertex.hpp"
#include "graphics/scenegraph/mesh/meshdefinition.hpp"
#include "models/infrastructure.hpp"
#include "exceptions/genexc.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <array>

namespace BlueBear {
  namespace Models {
    class FloorTile;
  }

  namespace Scripting {
    class Tile;
  }

  namespace Graphics {
    namespace Utilities{ class ShaderManager; }
    class Shader;

    namespace SceneGraph {
      class Model;

      namespace ModelLoader {
        class FloorModelLoader : public ProceduralModelLoader {
          std::shared_ptr< Shader > shader;
          const std::vector< Models::Infrastructure::FloorLevel >& levels;

          sf::Image generateTexture( const Models::Infrastructure::FloorLevel& currentLevel );

        public:
          FloorModelLoader( const std::vector< Models::Infrastructure::FloorLevel >& levels, Utilities::ShaderManager& shaderManager );

          virtual std::shared_ptr< Model > get();
        };

      }
    }
  }
}

#endif
