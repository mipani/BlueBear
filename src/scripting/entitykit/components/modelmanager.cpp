#include "scripting/entitykit/components/modelmanager.hpp"
#include "scripting/luakit/utility.hpp"
#include "state/householdgameplaystate.hpp"
#include "device/display/adapter/component/worldrenderer.hpp"
#include "graphics/scenegraph/model.hpp"
#include "tools/utility.hpp"
#include "log.hpp"
#include <set>

namespace BlueBear::Scripting::EntityKit::Components {

  ModelManager::ModelManager() : SystemComponent::SystemComponent( "system.component.model_manager" ) {}

  void ModelManager::submitLuaContributions( sol::state& lua, sol::table types ) {
    types.new_usertype< ModelManager >( "ModelManager",
      "new", sol::no_constructor,
      "init", &ModelManager::init,
      "get_potential_models", &ModelManager::getPotentialModels,
      "place_object", &ModelManager::placeObject,
      "remove_object", &ModelManager::removeObject,
      sol::base_classes, sol::bases< SystemComponent, Component >()
    );
  }

  void ModelManager::init( sol::object incoming ) {
    if( incoming.is< sol::table >() ) {
      sol::table asTable = incoming.as< sol::table >();
      for( auto& pair : asTable ) {
        if( pair.second.is< std::string >() ) {
          potentialModels.push_back( pair.second.as< std::string >() );
        }
      }
    }
  }

  void ModelManager::load( const Json::Value& data ) {
    std::vector< std::pair< std::string, std::string > > paths;

    for( auto it = data.begin(); it != data.end(); ++it ) {
      auto pair = Tools::Utility::jsonIteratorToPair( it );

      potentialModels.emplace_back( pair.first );
      paths.emplace_back( pair.first, pair.second.get().asString() );
    }

    if( paths.size() ) {
      relevantState->getWorldRenderer().loadPaths( paths );
    }
  }

  void ModelManager::drop() {
    for( const auto& pair : models ) {
      relevantState->getWorldRenderer().removeObject( pair.second );
    }
  }

  std::shared_ptr< Graphics::SceneGraph::Model > ModelManager::placeObject( const std::string& modelId, sol::table classes ) {
    std::shared_ptr< Graphics::SceneGraph::Model > model = relevantState->getWorldRenderer().placeObject( modelId, LuaKit::Utility::tableToSet< std::string >( classes ) );

    models.emplace_back( modelId, model );

    return model;
  }

  void ModelManager::removeObject( Graphics::SceneGraph::Model& model ) {
    auto asPtr = model.shared_from_this();

    for( auto it = models.begin(); it != models.end(); ) {
      if( it->second == asPtr ) {
        relevantState->getWorldRenderer().removeObject( asPtr );
        it = models.erase( it );

        return;
      } else {
        ++it;
      }
    }
  }

  std::vector< std::string > ModelManager::getPotentialModels() {
    return potentialModels;
  }

  std::vector< std::pair< std::string, std::shared_ptr< Graphics::SceneGraph::Model > > > ModelManager::getModels() const {
    return models;
  }

}
