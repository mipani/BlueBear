#include "configmanager.hpp"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <fstream>

namespace BlueBear {

  /**
   * Clumsily define the defaults application-wide in this constructor.
   * Is there a way to do this in bulk using C++11 uniform initialisation?
   */
  ConfigManager::ConfigManager() {
    configRoot[ "fps_overview" ] = 30;
    configRoot[ "fps_homeview" ] = 60;
    configRoot[ "vsync_limiter_overview" ] = false;
    configRoot[ "vsync_limiter_homeview" ] = false;
    configRoot[ "ticks_per_second" ] = 30;
    configRoot[ "min_log_level" ] = 0;
    configRoot[ "logfile_path" ] = "bluebear.log";
    configRoot[ "logger_mode" ] = 0;
    configRoot[ "viewport_x" ] = 1024;
    configRoot[ "viewport_y" ] = 768;
    configRoot[ "current_locale" ] = "en_US";
    configRoot[ "threadpool_count" ] = 2;
    configRoot[ "threadpool_detect" ] = true;
    configRoot[ "font_osd" ] = "system/ui/dosvga.ttf";
    configRoot[ "font_ui" ] = "system/ui/terminus.ttf";

    // Load settings.json from file
    std::ifstream settingsFile( SETTINGS_PATH );
    Json::Value settingsJSON;
    Json::Reader reader;

    if( reader.parse( settingsFile, settingsJSON ) ) {
      // Override defaults
      // iterators - barf
      for( Json::Value::iterator jsonIterator = settingsJSON.begin(); jsonIterator != settingsJSON.end(); ++jsonIterator ) {
        // BARF
        Json::Value key = jsonIterator.key();
        Json::Value value = *jsonIterator;

        // Store every value as a string because strong typing is a PITA
        configRoot[ key.asString() ] = value;
      }
    }
  }

  std::string ConfigManager::getValue( const std::string& key ) {
    return configRoot[ key ].asString();
  }

  int ConfigManager::getIntValue( const std::string& key ) {
    return configRoot[ key ].asInt();
  }

  bool ConfigManager::getBoolValue( const std::string& key ) {
    return configRoot[ key ].asBool();
  }

  /**
   * Get a value. This always returns a string. If you want an int, use to_number
   * inside lua.
   */
  int ConfigManager::lua_getValue( lua_State* L ) {
    // Linker errors when using the singleton directly, I don't know why.
    ConfigManager& self = *( ( ConfigManager* )lua_touserdata( L, lua_upvalueindex( 1 ) ) );

    if( lua_isstring( L, -1 ) ) {
      std::string key = lua_tostring( L, -1 );
      std::string result = self.getValue( key );

      if( result == "" ) {
        // Looked up an invalid key
        lua_pushnil( L );
      } else {
        lua_pushstring( L, result.c_str() );
      }

      return 1;
    } else {
      return luaL_error( L, "ConfigManager: Failed to provide a string key." );
    }
  }
}
