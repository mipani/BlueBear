#include "graphics/gui/luaelement.hpp"
#include "graphics/display.hpp"
#include "tools/utility.hpp"
#include "eventmanager.hpp"
#include "log.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFGUI/SFGUI.hpp>
#include <string>

namespace BlueBear {
  namespace Graphics {
    namespace GUI {

      std::map< void*, std::map< sfg::Signal::SignalID, LuaElement::SignalBinding > > LuaElement::masterSignalMap;

      /**
       * @static
       */
      int LuaElement::lua_onEvent( lua_State* L ) {
        LuaElement** userData = ( LuaElement** ) luaL_checkudata( L, 1, "bluebear_widget" );
        Display::MainGameState* self = ( Display::MainGameState* )lua_touserdata( L, lua_upvalueindex( 1 ) );

        // function "event" self

        if( lua_isstring( L, -2 ) ) {
          const char* eventType = lua_tostring( L, -2 );

          switch( Tools::Utility::hash( eventType ) ) {
            case Tools::Utility::hash( "click" ):
              {
                LuaElement& element = **userData;

                // Create the bucket for this widget if it doesn't exist, otherwise, return a new bucket
                auto& signalMap = masterSignalMap[ element.widget.get() ];

                // If there's a previous sfg::Widget::OnLeftClick registered for this widget instance, unref and kill it
                auto pair = signalMap.find( sfg::Widget::OnLeftClick );
                if( pair != signalMap.end() ) {
                  // One click listener at a time
                  // Disconnect the handler
                  element.widget->GetSignal( sfg::Widget::OnLeftClick ).Disconnect( pair->second.slotHandle );

                  // Un-ref this function we're about to erase
                  luaL_unref( L, LUA_REGISTRYINDEX, pair->second.reference );
                }

                // Track the master reference
                // Unref this if the pointer is ever removed!
                LuaReference masterReference = luaL_ref( L, LUA_REGISTRYINDEX ); // "event" self
                unsigned int handle = element.widget->GetSignal( sfg::Widget::OnLeftClick ).Connect( [ L, self, masterReference ]() {
                  // Create new "disposable" reference that will get ferried through
                  lua_rawgeti( L, LUA_REGISTRYINDEX, masterReference ); // object
                  self->instance.eventManager.UI_ACTION_EVENT.trigger( luaL_ref( L, LUA_REGISTRYINDEX ) ); // EMPTY
                } );

                signalMap[ sfg::Widget::OnLeftClick ] = LuaElement::SignalBinding{ masterReference, handle };

                lua_pushboolean( L, true ); // true "event" self
                return 1; // true
              }
              break;
            default:
              Log::getInstance().warn( "LuaElement::lua_onEvent", "Invalid event type specified: " + std::string( eventType ) );
          }
        } else {
          Log::getInstance().warn( "LuaElement::lua_onEvent", "Invalid event type passed to on()" );
        }

        return 0;
      }

      /**
       * @static
       */
      int LuaElement::lua_offEvent( lua_State* L ) {
        LuaElement** userData = ( LuaElement** ) luaL_checkudata( L, 1, "bluebear_widget" );

        // "event type" self
        if( lua_isstring( L, -1 ) ) {

          const char* eventType = lua_tostring( L, -1 );
          switch( Tools::Utility::hash( eventType ) ) {
            case Tools::Utility::hash( "click" ):
              {
                LuaElement& element = **userData;

                auto signalMapIt = masterSignalMap.find( element.widget.get() );
                if( signalMapIt != masterSignalMap.end() ) {
                  auto& signalMap = signalMapIt->second;

                  auto pair = signalMap.find( sfg::Widget::OnLeftClick );
                  if( pair != signalMap.end() ) {
                    element.widget->GetSignal( sfg::Widget::OnLeftClick ).Disconnect( pair->second.slotHandle );
                    luaL_unref( L, LUA_REGISTRYINDEX, pair->second.reference );

                    signalMap.erase( sfg::Widget::OnLeftClick );
                  }
                }
              }
              break;
            default:
              Log::getInstance().warn( "LuaElement::lua_offEvent", "Invalid event type specified: " + std::string( eventType ) );
          }

        } else {
          Log::getInstance().warn( "LuaElement::lua_offEvent", "Invalid event type passed to off()" );
        }

        return 0;
      }

      /**
       * @static
       */
      int LuaElement::lua_getWidgetByID( lua_State* L ) {
        std::string selector;

        // called from bluebear.gui.get_widget_by_id
        if( lua_isstring( L, -1 ) ) {
          selector = std::string( lua_tostring( L, -1 ) );
        } else {
          Log::getInstance().warn( "LuaElement::lua_getWidgetByID", "Argument provided to get_widget_by_id was not a string." );
          return 0;
        }

        Display::MainGameState* self = ( Display::MainGameState* )lua_touserdata( L, lua_upvalueindex( 1 ) );
        std::shared_ptr< sfg::Widget > parentWidget;
        if( lua_gettop( L ) == 1 ) {
          parentWidget = self->gui.rootContainer;
        } else {
          std::shared_ptr< sfg::Widget >* widgetPtr = *( ( std::shared_ptr< sfg::Widget >** ) luaL_checkudata( L, 1, "bluebear_widget" ) );
          parentWidget = *widgetPtr;
        }

        // Holy moly is this going to get confusing quick
        // I also have the least amount of confidence in this code you could possibly imagine
        std::shared_ptr< sfg::Widget > widget = parentWidget->GetWidgetById( selector );
        if( widget ) {
          getUserdataFromWidget( L, widget ); // userdata
          return 1;
        } else {
          std::string error = std::string( "Could not find widget with ID " ) + selector;
          return luaL_error( L, error.c_str() );
        }
      }

      /**
       * @static
       */
      int LuaElement::lua_getWidgetsByClass( lua_State* L ) {
        std::string selector;
        std::shared_ptr< sfg::Widget > parentWidget;

        // Check for presence of string selector
        if( lua_isstring( L, -1 ) ) {
          selector = lua_tostring( L, -1 );
        } else {
          Log::getInstance().warn( "LuaElement::lua_getWidgetsByClass", "Argument provided to get_widgets_by_class was not a string." );
          return 0;
        }

        // Determine parent widget depending on where method was called
        if( lua_gettop( L ) == 1 ) {
          // Method was called from stateless bluebear.gui object
          Display::MainGameState* state = ( Display::MainGameState* )lua_touserdata( L, lua_upvalueindex( 1 ) );
          parentWidget = state->gui.rootContainer;
        } else {
          // Method was called from a LuaElement instance
          std::shared_ptr< sfg::Widget >* widgetPtr = *( ( std::shared_ptr< sfg::Widget >** ) luaL_checkudata( L, 1, "bluebear_widget" ) );
          parentWidget = *widgetPtr;
        }

        // Perform the actual SFGUI call
        sfg::Widget::WidgetsList widgets = parentWidget->GetWidgetsByClass( selector );

        auto size = widgets.size();
        if( size ) {
          // At least one widget is present - these items must be chained in a Lua table
          // Create a Lua table, then push a new LuaInstance wrapper for every widget found as part of this class
          lua_createtable( L, size, 0 ); // table

          for( int i = 0; i != size; i++ ) {
            getUserdataFromWidget( L, widgets[ i ] ); // userdata table
            lua_rawseti( L, -2, i + 1 ); // table
          }

          return 1;
        } else {
          // No widgets are present
          std::string error = std::string( "Could not find any widgets with class " + selector );
          return luaL_error( L, error.c_str() );
        }
      }

      /**
       * @static
       */
      int LuaElement::lua_gc( lua_State* L ) {
        LuaElement* widgetPtr = *( ( LuaElement** ) luaL_checkudata( L, 1, "bluebear_widget" ) );

        // Destroy the std::shared_ptr< sfg::Widget >. This should decrease the reference count by one.
        //Log::getInstance().debug( "LuaElement::lua_gc", "Deleting " + Tools::Utility::pointerToString( widgetPtr ) );
        delete widgetPtr;

        return 0;
      }

      /**
       *
       * STACK ARGS: (none)
       * RETURNS: userdata
       */
      void LuaElement::getUserdataFromWidget( lua_State* L, std::shared_ptr< sfg::Widget > widget ) {
        LuaElement** userData = ( LuaElement** )lua_newuserdata( L, sizeof( LuaElement* ) ); // userdata
        *userData = new LuaElement();

        ( **userData ).widget = widget;

        luaL_getmetatable( L, "bluebear_widget" ); // metatable userdata
        lua_setmetatable( L, -2 ); // userdata
      }

    }
  }
}
