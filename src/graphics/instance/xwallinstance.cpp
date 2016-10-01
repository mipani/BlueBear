#include "graphics/instance/xwallinstance.hpp"
#include "graphics/instance/wallinstance.hpp"
#include "graphics/model.hpp"
#include "graphics/imagebuilder/imagesource.hpp"
#include "graphics/imagebuilder/directimagesource.hpp"
#include <memory>
#include <map>
#include <string>

namespace BlueBear {
  namespace Graphics {

    std::shared_ptr< Model > XWallInstance::Piece( nullptr );
    std::shared_ptr< Model > XWallInstance::EdgePiece( nullptr );

    XWallInstance::XWallInstance( GLuint shaderProgram, TextureCache& hostTextureCache ) : WallInstance::WallInstance( *XWallInstance::Piece, shaderProgram, hostTextureCache ) {}

    void XWallInstance::setRotationAttributes( unsigned int rotation, std::map< std::string, std::unique_ptr< ImageSource > >& settings ) {

    }

  }
}
