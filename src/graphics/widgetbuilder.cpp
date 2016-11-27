#include "graphics/widgetbuilder.hpp"
#include "log.hpp"
#include <string>
#include <memory>
#include <SFGUI/Widget.hpp>
#include <SFGUI/Window.hpp>
#include <SFGUI/Container.hpp>
#include <SFGUI/Label.hpp>
#include <tinyxml2.h>

namespace BlueBear {
  namespace Graphics {

    constexpr unsigned int WidgetBuilder::hash(const char* str, int h) {
      return !str[h] ? 5381 : (hash(str, h+1) * 33) ^ str[h];
    }

    std::shared_ptr< sfg::Widget > WidgetBuilder::nodeToWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Widget > widget;

      const char* tagType = element->Name();

      switch( hash( tagType ) ) {

        case hash( "Window" ):
          widget = newWindowWidget( element );
          addChildren( std::static_pointer_cast< sfg::Container >( widget ), element );
          break;

        case hash( "Label" ):
          widget = newLabelWidget( element );
          break;

        default:
          Log::getInstance().error( "WidgetBuilder::nodeToWidget", "Invalid CME tag specified: " + std::string( tagType ) );
          throw InvalidCMEWidgetException();
      }

      return widget;
    }

    void WidgetBuilder::addChildren( std::shared_ptr< sfg::Container > widget, tinyxml2::XMLElement* element ) {
      for ( tinyxml2::XMLElement* child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement() ) {
        widget->Add( nodeToWidget( child ) );
      }
    }

    void WidgetBuilder::setIdAndClass( std::shared_ptr< sfg::Widget > widget, tinyxml2::XMLElement* element ) {
      const char* id = element->Attribute( "id" );
      const char* clss = element->Attribute( "class" );

      if ( id ) {
        widget->SetId( id );
      }

      if ( clss ) {
        widget->SetClass( clss );
      }
    }

    std::shared_ptr< sfg::Window > WidgetBuilder::newWindowWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Window > window = sfg::Window::Create();

      setIdAndClass( window, element );

      return window;
    }

    std::shared_ptr< sfg::Label > WidgetBuilder::newLabelWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Label > label;

      // STUB !!

      return label;
    }

  }
}
