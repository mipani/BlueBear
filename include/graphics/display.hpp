#ifndef BBSCREEN
#define BBSCREEN

/**
 * Abstracted class representing the display device that is available to BlueBear.
 */
#include "containers/collection3d.hpp"
#include "containers/conccollection3d.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include <vector>
#include <memory>
#include <list>
#include <glm/glm.hpp>
#include <string>
#include <map>
#include "graphics/texturecache.hpp"
#include "graphics/imagecache.hpp"
#include "scripting/tile.hpp"
#include "scripting/wallcell.hpp"
#include "graphics/imagebuilder/imagesource.hpp"
#include "graphics/shader.hpp"
#include "graphics/model.hpp"
#include "graphics/camera.hpp"
#include "threading/lockable.hpp"

namespace BlueBear {
  namespace Scripting {
    class Lot;
  }

  namespace Threading {
    class CommandBus;
  }

  namespace Graphics {
    class Instance;
    class WallInstance;
    class XWallInstance;
    class YWallInstance;
    class DWallInstance;
    class RWallInstance;
    class WallCellBundler;

    class Display {
    public:
      static const std::string WALLPANEL_MODEL_XY_PATH;
      static const std::string WALLPANEL_MODEL_DR_PATH;
      static const std::string FLOOR_MODEL_PATH;

      // RAII style
      Display( Threading::CommandBus& commandBus );
      ~Display();

      void openDisplay();
      void start();
      void loadInfrastructure( unsigned int currentRotation, Containers::ConcCollection3D< Threading::Lockable< Scripting::Tile > >& floorMap, Containers::ConcCollection3D< Threading::Lockable< Scripting::WallCell > >& wallMap );

      // ---------- STATES ----------
      class State {
        public:
          Display& instance;
          State( Display& instance );
          virtual void execute() = 0;
          virtual void handleEvent( sf::Event& event ) = 0;
      };

      class IdleState : public State {
        public:
          IdleState( Display& instance );
          void execute();
          void handleEvent( sf::Event& event );
      };

      class TitleState : public State {
        public:
          TitleState( Display& instance );
          void execute();
          void handleEvent( sf::Event& event );
      };

      class MainGameState : public State {
          struct {
            std::string ISOMETRIC;
            std::string FIRST_PERSON;
          } strings;
          unsigned int currentRotation;
          Shader defaultShader;
          Camera camera;
          Model floorModel;
          TextureCache texCache;
          ImageCache imageCache;
          struct {
            sfg::Desktop desktop;
            std::shared_ptr< sfg::Window > window;
            std::shared_ptr< sfg::Box > box;
            std::shared_ptr< sfg::Label > statusLabel;
            sf::Clock clock;
          } gui;
          // These are from the lot!
          Containers::ConcCollection3D< Threading::Lockable< Scripting::Tile > >& floorMap;
          Containers::ConcCollection3D< Threading::Lockable< Scripting::WallCell > >& wallMap;
          // These are ours!
          std::unique_ptr< Containers::Collection3D< std::shared_ptr< Instance > > > floorInstanceCollection;
          std::unique_ptr< Containers::Collection3D< std::shared_ptr< WallCellBundler > > > wallInstanceCollection;
          void processOsd();
          void loadInfrastructure();
          void createFloorInstances();
          void createWallInstances();
          void setupDefaultWindows();
        public:
          void execute();
          void handleEvent( sf::Event& event );
          MainGameState( Display& instance, unsigned int currentRotation, Containers::ConcCollection3D< Threading::Lockable< Scripting::Tile > >& floorMap, Containers::ConcCollection3D< Threading::Lockable< Scripting::WallCell > >& wallMap );
          ~MainGameState();
      };
      // ----------------------------

      class Command {
        public:
          virtual void execute( Display& instance ) = 0;
      };

      class NewEntityCommand : public Command {
        glm::vec3 position;
        public:
          void execute( Display& instance );
      };

      class SendInfrastructureCommand : public Command {
        unsigned int rotation;
        Containers::ConcCollection3D< Threading::Lockable< Scripting::Tile > >& floorMap;
        Containers::ConcCollection3D< Threading::Lockable< Scripting::WallCell > >& wallMap;
        public:
          SendInfrastructureCommand( Scripting::Lot& lot );
          void execute( Display& instance );
      };

      using CommandList = std::list< std::unique_ptr< Command > >;

      private:
        using ViewportDimension = int;
        ViewportDimension x;
        ViewportDimension y;
        sf::RenderWindow mainWindow;
        Threading::CommandBus& commandBus;
        sfg::SFGUI sfgui;

        std::unique_ptr< State > currentState;

        struct {
          sf::Font osdFont;
          sf::Font uiFont;
        } fonts;

        void main();
    };

  }
}

#endif
