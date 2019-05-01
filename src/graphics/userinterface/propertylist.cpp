#include "graphics/userinterface/propertylist.hpp"
#include "graphics/userinterface/element.hpp"
#include <glm/glm.hpp>
#include <string>

namespace BlueBear {
  namespace Graphics {
    namespace UserInterface {

      // Note: if you add a new type here, style.hpp may need to know how to interpolate it for animations

      const PropertyList _default( {
        { "antialias", true },
        { "left", 0 },
        { "top", 0 },
        { "width", ( int ) Requisition::AUTO },
        { "height", ( int ) Requisition::AUTO },
        { "padding", 0 },
        { "placement", Placement::FLOW },
        { "gravity", Gravity::LEFT },
        { "layout-weight", 0 },
        { "vertical-orientation", Orientation::TOP },
        { "horizontal-orientation", Orientation::LEFT },
        { "background-color", glm::uvec4{ 255, 255, 255, 255 } },
        { "color", glm::uvec4{ 0, 0, 0, 255 } },
        { "drop-shadow-left", false },
        { "drop-shadow-top", false },
        { "drop-shadow-right", false },
        { "drop-shadow-bottom", false },
        { "fade-in-color", glm::uvec4{ 255, 255, 255, 255 } },
        { "fade-out-color", glm::uvec4{ 0, 0, 0, 255 } },
        { "cursor-color", glm::uvec4{ 0, 0, 0, 255 } },
        { "font", std::string{ "roboto" } },
        { "font-color", glm::uvec4{ 255, 255, 255, 255 } },
        { "font-hint-color", glm::uvec4{ 128, 128, 128, 255 } },
        { "font-size", 16.0 },
        { "text-alignment", Orientation::MIDDLE },
        { "close-event", false },
        { "fade", false },
        { "draggable", false },
        { "local-z-order", 1 },
        { "scrollbar-x", true },
        { "scrollbar-y", true },
        { "tab-title", std::string{ "" } },
        { "tab-index", 0 },
        { "tab-active-accent-color", glm::uvec4{ 0, 0, 0, 255 } },
        { "tab-inactive-accent-color", glm::uvec4{ 128, 128, 128, 255 } }
      } );

      const PropertyList& PropertyList::rootPropertyList = _default;

    }
  }
}
