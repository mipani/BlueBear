#include "graphics/widgetbuilder.hpp"
#include "graphics/imagecache.hpp"
#include "graphics/imagebuilder/pathimagesource.hpp"
#include "configmanager.hpp"
#include "log.hpp"
#include "tools/utility.hpp"
#include "eventmanager.hpp"
#include <string>
#include <memory>
#include <SFGUI/Widget.hpp>
#include <SFGUI/Window.hpp>
#include <SFGUI/Container.hpp>
#include <SFGUI/Label.hpp>
#include <SFGUI/Box.hpp>
#include <SFGUI/Alignment.hpp>
#include <SFGUI/Misc.hpp>
#include <SFGUI/Button.hpp>
#include <tinyxml2.h>
#include <vector>
#include <functional>

namespace BlueBear {
  namespace Graphics {

    WidgetBuilder::WidgetBuilder( EventManager& eventManager, ImageCache& imageCache, const std::string& path ) : eventManager( eventManager ), imageCache( imageCache ) {
      document.LoadFile( path.c_str() );

      // Object is unusable if the file failed to load
      if( document.ErrorID() ) {
        Log::getInstance().error( "WidgetBuilder::WidgetBuilder",
          "WidgetBuilder construction failed; could not parse XML file " + path
        );
        throw FailedToLoadXMLException();
      }
    }

    constexpr unsigned int WidgetBuilder::hash(const char* str, int h) {
      return !str[h] ? 5381 : (hash(str, h+1) * 33) ^ str[h];
    }

    /**
     * This is where it all happens. Call this method to turn your XML file into a collection of widgets.
     */
    std::vector< std::shared_ptr< sfg::Widget > > WidgetBuilder::getWidgets() {
      std::vector< std::shared_ptr< sfg::Widget > > widgets;

      for ( tinyxml2::XMLElement* node = document.RootElement(); node != NULL; node = node->NextSiblingElement() ) {
        widgets.push_back( nodeToWidget( node ) );
      }

      return widgets;
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

        case hash( "Box" ):
          widget = newBoxWidget( element );
          packChildren( std::static_pointer_cast< sfg::Box >( widget ), element );
          break;

        case hash( "Alignment" ):
          widget = newAlignmentWidget( element );
          addChildren( std::static_pointer_cast< sfg::Container >( widget ), element );
          break;

        case hash( "Button" ):
          widget = newButtonWidget( element );
          // SFGUI has the ability to add children to a button but I don't think it makes much sense.
          /* addChildren( std::static_pointer_cast< sfg::Container >( widget ), element ); */
          break;

        case hash( "Entry" ):
          widget = newEntryWidget( element );
          break;

        case hash( "Image" ):
          widget = newImageWidget( element );
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

    void WidgetBuilder::packChildren( std::shared_ptr< sfg::Box > widget, tinyxml2::XMLElement* element ) {
      bool packFromStart = false;
      const char* _pack = element->Attribute( "pack" );
      if( !_pack ) {
        _pack = "end";
      }
      std::string pack( _pack );

      if( pack == "start" ) {
        packFromStart = true;
      } else if ( pack == "end" ) {
        packFromStart = false;
      } else {
        Log::getInstance().warn( "WidgetBuilder::packChildren", "Invalid value for \"pack\" attribute: " + pack + ", defaulting to \"end\"" );
      }

      for ( tinyxml2::XMLElement* child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement() ) {
        bool expand = true;
        bool fill = true;

        child->QueryBoolAttribute( "expand", &expand );
        child->QueryBoolAttribute( "fill", &fill );

        if( packFromStart ) {
          widget->PackStart( nodeToWidget( child ), expand, fill );
        } else {
          widget->PackEnd( nodeToWidget( child ), expand, fill );
        }
      }
    }

    /**
     * Set basic properties on all widgets
     */
    void WidgetBuilder::setBasicProperties( std::shared_ptr< sfg::Widget > widget, tinyxml2::XMLElement* element ) {
      const char* id = element->Attribute( "id" );
      const char* clss = element->Attribute( "class" );
      const char* visible = element->Attribute( "visible" );

      if ( id ) {
        widget->SetId( id );
      }

      if ( clss ) {
        widget->SetClass( clss );
      }

      if ( visible ) {
        std::string value( visible );

        if( value == "true" ) {
          widget->Show( true );
        } else if ( value == "false" ) {
          widget->Show( false );
        } else {
          Log::getInstance().warn( "WidgetBuilder::setBasicProperties",  "Invalid value for \"visible\" attribute: " + std::string( value ) + ", defaulting to \"true\"" );
        }
      }
    }

    void WidgetBuilder::correctXBoundary( float* input ) {
      static int VIEWPORT_X = ConfigManager::getInstance().getIntValue( "viewport_x" );

      if( *input < 0.0f ) {
        *input = VIEWPORT_X + *input;
      }
    }

    void WidgetBuilder::correctYBoundary( float* input ) {
      static int VIEWPORT_Y = ConfigManager::getInstance().getIntValue( "viewport_y" );

      if( *input < 0.0f ) {
        *input = VIEWPORT_Y + *input;
      }
    }

    /**
     * Attach events that should be on every widget
     */
    void WidgetBuilder::setDefaultEvents( std::shared_ptr< sfg::Widget > widget, tinyxml2::XMLElement* element ) {
      widget->GetSignal( sfg::Widget::OnMouseLeftRelease ).Connect( [ &eventManager = eventManager ]() {
        eventManager.SFGUI_EAT_EVENT.trigger( SFGUIEatEvent::Event::EAT_MOUSE_EVENT );
      } );

      widget->GetSignal( sfg::Widget::OnMouseRightRelease ).Connect( [ &eventManager = eventManager ]() {
        eventManager.SFGUI_EAT_EVENT.trigger( SFGUIEatEvent::Event::EAT_MOUSE_EVENT );
      } );
    }

    void WidgetBuilder::setAlignment( std::shared_ptr< sfg::Misc > widget, tinyxml2::XMLElement* element ) {
      sf::Vector2f alignment = widget->GetAlignment();

      element->QueryFloatAttribute( "alignment_x", &alignment.x );
      element->QueryFloatAttribute( "alignment_y", &alignment.y );

      widget->SetAlignment( alignment );
    }

    void WidgetBuilder::setAllocationAndRequisition( std::shared_ptr< sfg::Widget > widget, tinyxml2::XMLElement* element ) {
      sf::FloatRect allocation = widget->GetAllocation();

      element->QueryFloatAttribute( "top", &allocation.top );
      correctYBoundary( &allocation.top );
      element->QueryFloatAttribute( "left", &allocation.left );
      correctXBoundary( &allocation.left );
      element->QueryFloatAttribute( "width", &allocation.width );
      element->QueryFloatAttribute( "height", &allocation.height );

      widget->SetAllocation( allocation );

      sf::Vector2f requisition = widget->GetRequisition();

      element->QueryFloatAttribute( "min-width", &requisition.x );
      element->QueryFloatAttribute( "min-height", &requisition.y );

      widget->SetRequisition( requisition );
    }

    std::shared_ptr< sfg::Window > WidgetBuilder::newWindowWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Window > window = sfg::Window::Create();

      setBasicProperties( window, element );
      setAllocationAndRequisition( window, element );
      setDefaultEvents( window, element );

      const char* title = element->Attribute( "title" );
      if( title ) {
        window->SetTitle( title );
      }

      // number |= 1 << x; to set a bit
      // number &= ~(1 << x); to clear a bit
      // (ID >> position) & 1 to get specific bit

      // Unpack these properties
      bool titlebar = window->HasStyle( sfg::Window::Style::TITLEBAR );
      bool background = window->HasStyle( sfg::Window::Style::BACKGROUND );
      bool resize = window->HasStyle( sfg::Window::Style::RESIZE );
      bool shadow = window->HasStyle( sfg::Window::Style::SHADOW );
      bool close = window->HasStyle( sfg::Window::Style::CLOSE );

      element->QueryBoolAttribute( "titlebar", &titlebar );
      element->QueryBoolAttribute( "background", &background );
      element->QueryBoolAttribute( "resize", &resize );
      element->QueryBoolAttribute( "shadow", &shadow );
      element->QueryBoolAttribute( "close", &close );

      window->SetStyle(
        ( titlebar ? 1 : 0 ) |
        ( ( background ? 1 : 0 ) << 1 ) |
        ( ( resize ? 1 : 0 ) << 2 ) |
        ( ( shadow ? 1 : 0 ) << 3 ) |
        ( ( close ? 1 : 0 ) << 4 )
      );

      return window;
    }

    std::shared_ptr< sfg::Label > WidgetBuilder::newLabelWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Label > label;

      const char* labelValue = element->GetText();

      if( labelValue ) {
        label = sfg::Label::Create( labelValue );
        setBasicProperties( label, element );
        setAllocationAndRequisition( label, element );
        setAlignment( label, element );
      } else {
        label = sfg::Label::Create( "" );
      }

      setDefaultEvents( label, element );

      return label;
    }

    std::shared_ptr< sfg::Box > WidgetBuilder::newBoxWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Box > box;

      const char* orientation = element->Attribute( "orientation" );
      if( !orientation ) {
        orientation = "horizontal";
      }

      sfg::Box::Orientation orientationFlag;

      switch( hash( orientation ) ) {
        case hash( "vertical" ):
          orientationFlag = sfg::Box::Orientation::VERTICAL;
          break;
        default:
          Log::getInstance().warn( "WidgetBuilder::newBoxWidget", "Invalid value for \"orientation\" attribute: " + std::string( orientation ) + ", defaulting to \"horizontal\"" );
        case hash( "horizontal" ):
          orientationFlag = sfg::Box::Orientation::HORIZONTAL;
          break;
      }

      float spacing = 0.0f;
      element->QueryFloatAttribute( "spacing", &spacing );

      box = sfg::Box::Create( orientationFlag, spacing );
      setBasicProperties( box, element );
      setDefaultEvents( box, element );

      return box;
    }

    std::shared_ptr< sfg::Alignment > WidgetBuilder::newAlignmentWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Alignment > alignment = sfg::Alignment::Create();

      setBasicProperties( alignment, element );
      setAllocationAndRequisition( alignment, element );
      setAlignment( alignment, element );
      setDefaultEvents( alignment, element );

      sf::Vector2f scale = alignment->GetScale();
      element->QueryFloatAttribute( "scale_x", &scale.x );
      element->QueryFloatAttribute( "scale_y", &scale.y );

      alignment->SetScale( scale );

      return alignment;
    }

    /**
     * TODO: Support for sfg::Image, and the ability to add one to a Button
     */
    std::shared_ptr< sfg::Button > WidgetBuilder::newButtonWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Button > button = sfg::Button::Create( element->GetText() );

      setBasicProperties( button, element );
      setAllocationAndRequisition( button, element );
      setDefaultEvents( button, element );

      return button;
    }

    std::shared_ptr< sfg::Entry > WidgetBuilder::newEntryWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Entry > entry = sfg::Entry::Create( element->GetText() );

      setBasicProperties( entry, element );
      setAllocationAndRequisition( entry, element );
      setDefaultEvents( entry, element );

      // Use eventManager to trigger a disable key events on eventManager.SFGUI_SIGNAL_EVENT when focusing in,
      // and an enable key events when focusing out.
      entry->GetSignal( sfg::Widget::OnKeyPress ).Connect( [ &eventManager = eventManager ]() {
        eventManager.SFGUI_EAT_EVENT.trigger( SFGUIEatEvent::Event::EAT_KEYBOARD_EVENT );
      } );

      return entry;
    }

    std::shared_ptr< sfg::Image > WidgetBuilder::newImageWidget( tinyxml2::XMLElement* element ) {
      std::shared_ptr< sfg::Image > image = sfg::Image::Create();

      setBasicProperties( image, element );
      setAllocationAndRequisition( image, element );
      setAlignment( image, element );
      setDefaultEvents( image, element );

      // This should be somewhat fault-tolerant since images go missing all the damn time
      std::string path = element->GetText();

      try {
        PathImageSource piss = PathImageSource( path );
        image->SetImage( *imageCache.getImage( piss ) );
      } catch( std::exception& e ) {
        Log::getInstance().warn( "WidgetBuilder::newImageWidget", "Could not load image for image widget: " + std::string( e.what() ) );
      }

      return image;
    }

  }
}
