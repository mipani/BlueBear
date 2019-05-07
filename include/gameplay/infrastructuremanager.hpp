#ifndef INFRASTRUCTUREMANAGER
#define INFRASTRUCTUREMANAGER

#include "state/substate.hpp"
#include "models/infrastructure.hpp"
#include "models/wallsegment.hpp"
#include "tools/sector_discovery.hpp"
#include <jsoncpp/json/json.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace BlueBear::State { class HouseholdGameplayState; }
namespace BlueBear::Graphics::Utilities{ class ShaderManager; }
namespace BlueBear::Graphics::SceneGraph::Light { class SectorIlluminator; }
namespace BlueBear::Gameplay {

	class InfrastructureManager : public State::Substate {
		Models::Infrastructure model;
		std::shared_ptr< Graphics::SceneGraph::Light::SectorIlluminator > sectorLights;

	public:
		InfrastructureManager( State::State& state );

		void loadInfrastructure( const Json::Value& infrastructure );

		void generateRooms();

		bool update() override;
	};

}

#endif