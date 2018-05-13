#include "scripting/entitykit/registry.hpp"
#include "scripting/entitykit/entity.hpp"
#include "scripting/entitykit/components/modelmanager.hpp"
#include "scripting/luakit/dynamicusertype.hpp"
#include "graphics/scenegraph/model.hpp"
#include "tools/utility.hpp"
#include "eventmanager.hpp"
#include "log.hpp"
#include <functional>

namespace BlueBear::Scripting::EntityKit {

  Registry::Registry() {
    eventManager.LUA_STATE_READY.listen( this, std::bind( &Registry::submitLuaContributions, this, std::placeholders::_1 ) );
  }

  Registry::~Registry() {
    eventManager.LUA_STATE_READY.stopListening( this );
  }

  void Registry::submitLuaContributions( sol::state& lua ) {
    if( lua[ "bluebear" ][ "entity" ] == sol::nil ) {
      lua[ "bluebear" ][ "entity" ] = lua.create_table();
    }

    sol::table entity = lua[ "bluebear" ][ "entity" ];

    entity.set_function( "register_component", &Registry::registerComponent, this );
    entity.set_function( "register_entity", &Registry::registerEntity, this );
    entity.set_function( "create_new_entity", &Registry::newEntity, this );

    sol::table types = lua[ "bluebear" ][ "entity" ][ "types" ] = lua.create_table();
    Component::submitLuaContributions( lua, types );
    Entity::submitLuaContributions( lua, types );
    Components::ModelManager::submitLuaContributions( lua, types );
  }

  void Registry::registerComponent( const std::string& id, sol::table table ) {
    if( !componentRegistered( id ) ) {
      components[ id ] = table;
    } else {
      Log::getInstance().warn( "Registry::registerComponent", "Component " + id + " already registered!" );
    }
  }

  void Registry::registerEntity( const std::string& id, sol::table componentlist ) {
    if( !entityRegistered( id ) ) {
      std::vector< std::string > list;

      // Verify all attached components exist and have been registered
      for( auto& pair : componentlist ) {
        if( pair.second.is< std::string >() ) {
          std::string component = pair.second.as< std::string >();
          if( !componentRegistered( component ) ) {
            Log::getInstance().error( "Registry::registerEntity", "Component " + component + " has not been registered!" );
            return;
          } else {
            list.push_back( component );
          }
        }
      }

      entities[ id ] = list;
    } else {
      Log::getInstance().warn( "Registry::registerEntity", "Entity " + id + " already registered!" );
    }
  }

  bool Registry::entityRegistered( const std::string& id ) {
    return entities.find( id ) != entities.end();
  }

  bool Registry::componentRegistered( const std::string& id ) {
    switch( Tools::Utility::hash( id.c_str() ) ) {
      case Tools::Utility::hash( "system.component.model_manager" ):
        return true;
      default:
        return components.find( id ) != components.end();
    }
  }

  std::map< std::string, sol::object > Registry::tableToMap( sol::table table ) {
    std::map< std::string, sol::object > result;

    for( std::pair< sol::object, sol::object >& pair : table ) {
      if( pair.first.is< std::string >() ) {
        result[ pair.first.as< std::string >() ] = pair.second;
      }
    }

    return result;
  }

  std::shared_ptr< Component > Registry::newComponent( const std::string& id ) {
    switch( Tools::Utility::hash( id.c_str() ) ) {
      case Tools::Utility::hash( "system.component.model_manager" ):
        return std::make_shared< Components::ModelManager >();
      default: {
        auto it = components.find( id );
        if( it == components.end() ) {
          Log::getInstance().error( "Registry::newComponent", "This should not be happening!" );
        }

        return std::make_shared< Component >( tableToMap( it->second ) );
      }
    }
  }

  std::shared_ptr< Entity > Registry::newEntity( const std::string& id, sol::table constructors ) {
    auto it = entities.find( id );
    if( it == entities.end() ) {
      Log::getInstance().error( "Registry::newEntity", "Component " + id + " has not been registered!" );
      throw InvalidIDException();
    }

    std::map< std::string, std::shared_ptr< Component > > components;
    for( const std::string& component : it->second ) {
      components[ component ] = newComponent( component );
      components[ component ]->init( constructors[ component ] );
    }

    return std::make_shared< Entity >( components );
  }

}
