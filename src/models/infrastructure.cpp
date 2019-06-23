#include "models/infrastructure.hpp"
#include "tools/utility.hpp"
#include <vector>
#include "log.hpp"

namespace BlueBear::Models {

  Json::Value Infrastructure::save() {
    return Json::Value::null;
  }

  void Infrastructure::load( const Json::Value& data, Utilities::WorldCache& worldCache ) {
    if( data != Json::Value::null ) {
      const Json::Value& levelsJson = data[ "levels" ];
      for( auto it = levelsJson.begin(); it != levelsJson.end(); ++it ) {
        const Json::Value& level = *it;
        FloorLevel current;
        current.dimensions = { level[ "dimensions" ][ 0 ].asUInt(), level[ "dimensions" ][ 1 ].asUInt() };

        const Json::Value& innerTiles = level[ "tiles" ];
        for( auto it = innerTiles.begin(); it != innerTiles.end(); ++it ) {
          const Json::Value& set = *it;
          std::vector< std::optional< FloorTile > > tileSet;
          for( auto it = set.begin(); it != set.end(); ++it ) {
            std::string desiredTile = it->asString();
            if( desiredTile != "" ) {
              auto originalTile = worldCache.getFloorTile( desiredTile );
              if( !originalTile ) {
                Log::getInstance().error( "Infrastructure::load", "Tile found in lot but not registered: " + desiredTile );
              }
              tileSet.emplace_back( std::move( originalTile ) );
            } else {
              tileSet.emplace_back();
            }
          }

          current.tiles.emplace_back( std::move( tileSet ) );
        }

        const Json::Value& vertices = level[ "vertices" ];
        for( auto it = vertices.begin(); it != vertices.end(); ++it ) {
          const Json::Value& jsonVertices = *it;
          std::vector< float > vertexSet;
          for( auto it = jsonVertices.begin(); it != jsonVertices.end(); ++it ) {
            vertexSet.push_back( it->asFloat() );
          }
          current.vertices.emplace_back( std::move( vertexSet ) );
        }

        const Json::Value& wallSegments = level[ "wallpaper" ];
        for( const Json::Value& wallSegment : wallSegments ) {
          current.wallSegments.emplace_back( wallSegment, worldCache );
        }

        levels.emplace_back( std::move( current ) );
      }
    }
  }

  void Infrastructure::load( const Json::Value& data ) {
    Log::getInstance().error( "Infrastructure::load", "Called the wrong load method. This is probably a bug." );
  }

  std::vector< Infrastructure::FloorLevel >& Infrastructure::getLevels() {
    return levels;
  }
}
