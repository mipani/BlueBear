#include "graphics/scenegraph/modelloader/floormodelloader.hpp"
#include "graphics/scenegraph/mesh/indexedmeshgenerator.hpp"
#include "graphics/scenegraph/transform.hpp"
#include "graphics/scenegraph/drawable.hpp"
#include "graphics/scenegraph/model.hpp"
#include "graphics/scenegraph/material.hpp"
#include "graphics/texture.hpp"
#include "graphics/shader.hpp"
#include "configmanager.hpp"
#include "log.hpp"

namespace BlueBear {
  namespace Graphics {
    namespace SceneGraph {
      namespace ModelLoader {

        FloorModelLoader::FloorModelLoader( const std::vector< Models::Infrastructure::FloorLevel >& levels ) :
          levels( levels ) {}

        sf::Image FloorModelLoader::generateTexture( const Models::Infrastructure::FloorLevel& currentLevel ) {
          sf::Image result;

          int side = ConfigManager::getInstance().getIntValue( "floor_texture_size" );
          result.create( currentLevel.dimensions.x * side, currentLevel.dimensions.y * side );

          return result;
        }

        glm::vec2 FloorModelLoader::getTextureCoords( const glm::vec2& vertexCoord, const glm::vec2& boundaries ) {
          return {
            vertexCoord.x / boundaries.x,
            1.0 - ( vertexCoord.y / boundaries.y )
          };
        }

        std::shared_ptr< Model > FloorModelLoader::get() {
          int side = ConfigManager::getInstance().getIntValue( "floor_texture_size" );
          std::shared_ptr< Model > finalResult = Model::create( "__floorrig", {} );
          std::shared_ptr< Shader > shader = std::make_shared< Shader >( Mesh::TexturedVertex::getDefaultShader().first, Mesh::TexturedVertex::getDefaultShader().second );

          float baseElevation = 0.0f;
          for( const Models::Infrastructure::FloorLevel& floorLevel : levels ) {
            Mesh::IndexedMeshGenerator< Mesh::TexturedVertex > generator;
            sf::Image meshTexture = generateTexture( floorLevel );

            glm::vec2 vertexCount = { floorLevel.dimensions.x + 1.0, floorLevel.dimensions.y + 1.0 };
            for( int y = 0; y != floorLevel.dimensions.y; y++ ) {
              for( int x = 0; x != floorLevel.dimensions.x; x++ ) {
                glm::vec2 floats = { x, y };

                if( floorLevel.tiles[ y ][ x ] ) {
                  meshTexture.copy( *floorLevel.tiles[ y ][ x ]->surface, x * side, y * side );

                  generator.addTriangle(
                    { { floats.x - 0.5, floats.y + 0.5, baseElevation + floorLevel.vertices[ y ][ x ] },     { 0.0, 0.0, 1.0 }, getTextureCoords( { x, y }, vertexCount ) },
                    { { floats.x + 0.5, floats.y + 0.5, baseElevation + floorLevel.vertices[ y ][ x + 1 ] }, { 0.0, 0.0, 1.0 }, getTextureCoords( { x + 1, y }, vertexCount ) },
                    { { floats.x - 0.5, floats.y - 0.5, baseElevation + floorLevel.vertices[ y + 1 ][ x ] }, { 0.0, 0.0, 1.0 }, getTextureCoords( { x, y + 1 }, vertexCount ) }
                  );
                  generator.addTriangle(
                    { { floats.x + 0.5, floats.y + 0.5, baseElevation + floorLevel.vertices[ y ][ x + 1 ] },     { 0.0, 0.0, 1.0 }, getTextureCoords( { x + 1, y }, vertexCount ) },
                    { { floats.x - 0.5, floats.y - 0.5, baseElevation + floorLevel.vertices[ y + 1 ][ x ] },     { 0.0, 0.0, 1.0 }, getTextureCoords( { x, y + 1 }, vertexCount ) },
                    { { floats.x + 0.5, floats.y - 0.5, baseElevation + floorLevel.vertices[ y + 1 ][ x + 1 ] }, { 0.0, 0.0, 1.0 }, getTextureCoords( { x + 1, y + 1 }, vertexCount ) }
                  );
                }
              }
            }

            generator.generateNormals();

            std::shared_ptr< Model > result = Model::create( "__floorlevel", { {
              generator.generateMesh(),
              shader,
              std::make_shared< Material >( std::vector< std::shared_ptr< Texture > >{ std::make_shared< Texture >( meshTexture ) }, std::vector< std::shared_ptr< Texture > >{}, 0.0f, 1.0f )
            } } );

            glm::vec2 asFloat = { floorLevel.dimensions.x, floorLevel.dimensions.y };
            result->getLocalTransform().setPosition( { -( asFloat.x / 2.0 ), asFloat.y / 2.0, 0.0 } );
            finalResult->addChild( result );

            baseElevation += 10.0f;
          }

          return finalResult;
        }

      }
    }
  }
}
