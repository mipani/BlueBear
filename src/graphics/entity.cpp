#include "graphics/entity.hpp"
#include "graphics/model.hpp"
#include "graphics/drawable.hpp"
#include <GL/glew.h>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace BlueBear {
  namespace Graphics {

    Instance::Instance( const Model& model, GLuint shaderProgram ) : shaderProgram( shaderProgram ) {
      prepareInstanceRecursive( model );
    }

    void Instance::prepareInstanceRecursive( const Model& model ) {
      // Copy the list of drawables
      drawables = model.drawables;

      for( auto& pair : model.children ) {
        auto& child = *( pair.second );

        // Hand down the same transform as the parent to this model
        children.emplace( pair.first, std::make_shared< Instance >( child, shaderProgram ) );
      }
    }

    std::shared_ptr< Instance > Instance::findChildByName( std::string name ) {
      return children[ name ];
    }

    void Instance::drawEntity() {
      // Pass an identity matrix to mix-in
      glm::mat4 identity;
      drawEntity( identity );
    }

    void Instance::drawEntity( glm::mat4& parent ) {
      glm::mat4 nextParent = transform.sendToShader( shaderProgram, parent, dirty );

      for( auto& pair : drawables ) {
        auto& drawable = pair.second;

        drawable.render( shaderProgram );
      }

      for( auto& pair : children ) {
        // Apply the same transform of the parent
        pair.second->drawEntity( nextParent );
      }
    }

    void Instance::markDirty() {
      dirty = true;

      for( auto& pair : children ) {
        pair.second->markDirty();
      }
    }

    glm::vec3 Instance::getPosition() {
      return transform.position;
    }

    void Instance::setPosition( const glm::vec3& position ) {
      transform.position = position;
      markDirty();
    }

    glm::vec3 Instance::getScale() {
      return transform.scale;
    }

    void Instance::setScale( const glm::vec3& scale ) {
      transform.scale = scale;
      markDirty();
    }

    glm::vec3 Instance::getRotationAxes() {
      return transform.rotationAxes;
    }

    void Instance::setRotationAxes( const glm::vec3& rotationAxes ) {
      transform.rotationAxes = rotationAxes;
      markDirty();
    }

    GLfloat Instance::getRotationAngle() {
      return transform.rotationAngle;
    }

    void Instance::setRotationAngle( GLfloat rotationAngle ) {
      transform.rotationAngle = rotationAngle;
      markDirty();
    }

  }
}