#ifndef NEW_WIDGET_WINDOW
#define NEW_WIDGET_WINDOW

#include "graphics/userinterface/element.hpp"
#include <memory>
#include <vector>
#include <string>

namespace BlueBear {
  namespace Graphics {
    namespace Vector {
      class Renderer;
    }
    namespace UserInterface {
      namespace Widgets {

        class Window : public Element {
          std::string windowTitle;
          double textSpan = 0;

        protected:
          Window( const std::string& id, const std::vector< std::string >& classes, const std::string& windowTitle );

        public:
          virtual void render( Graphics::Vector::Renderer& renderer ) override;
          void calculate() override;

          static std::shared_ptr< Window > create( const std::string& id, const std::vector< std::string >& classes, const std::string& windowTitle );
        };

      }
    }
  }
}


#endif