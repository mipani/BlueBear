#include "lotentity.hpp"
#include "utility.hpp"
#include "bbtypes.hpp"
#include "json.hpp"
#include <iostream>
#include <cstring>
#include <string>

extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

namespace BlueBear {

	/**
	 * Every BlueBear::LotEntity is tied to its Lua instance in the _lotinsts table
	 */
	LotEntity::LotEntity( lua_State* L, const char* classID, const char* instance ) {

		// Store pointer to Luasphere on this object
		this->L = L;

		// Store classID in this->classID
		this->classID = classID;

		// Get fresh start with the Lua stack
		Utility::clearLuaStack( L );

		// Push bluebear onto Lua API stack
		lua_getglobal( L, "bluebear" );

		// Get new_instance_from_file method. This method will not only create a new instance, but also deserialise
		// the object provided, creating an instance that should be identical to what was saved previously
		Utility::getTableValue( L, "new_instance_from_file" );

		// Push identifier and instance string
		lua_pushstring( L, classID );
		lua_pushstring( L, instance );

		// Call new_instance_from_file
		if( lua_pcall( L, 2, 1, 0 ) == 0 ) {
			this->ok = true;

			// This will return a reference to the entry in _bblib - Pop and use this to store a reference to this function in this->luaVMInstance
			this->luaVMInstance = luaL_ref( L, LUA_REGISTRYINDEX );
		} else {
			std::cout << lua_tostring( L, -1 ) << std::endl;
		}
	}

	void LotEntity::execute( unsigned int worldTicks ) {

		if( this->ok == false ) {
			return;
		}

		unsigned int nextTickSchedule;

		// Clear the API stack of the Luasphere
		Utility::clearLuaStack( this->L );

		// Push this object's table onto the API stack
		lua_rawgeti( this->L, LUA_REGISTRYINDEX, this->luaVMInstance );

		// First, we need to push a reference to the _sys table
		Utility::getTableValue( this->L, "_sys" );

		// Next, push the value of _sched within _sys
		Utility::getTableValue( this->L, "_sched" );

		// Extract and pop int (and _sys table) from top of stack
		nextTickSchedule = lua_tonumber( this->L, -1 );
		lua_pop( this->L, 2 );

		// Execute only if the amount of ticks is just right (worldTicks >= nextTickSchedule)
		if( worldTicks >= nextTickSchedule ) {

			std::cout << "Running new iteration for luaVMInstance " << this->luaVMInstance << ", current worldTicks is " << worldTicks << " and this object's nextTickSchedule is " << nextTickSchedule << "\n";

			// Push the object's "main" method
			Utility::getTableValue( this->L, "main" );

			// Re-push table onto stack as argument
			lua_pushvalue( this->L, -2 );

			// Run function
			if( lua_pcall( this->L, 1, 1, 0 ) != 0 ) {
				std::cerr << lua_tostring( this->L, -1 ) << "\n";
				this->ok = false;
			}

			// This function returns a tick amount. The next execution is current world ticks + this amount
			// Set this object's _sys._sched to worldTicks + nextTickSchedule
			nextTickSchedule = lua_tonumber( this->L, -1 );
			lua_pop( this->L, 1 );

			// The function and its arguments should be popped, leaving the object itself
			// Get the _sys table
			Utility::getTableValue( this->L, "_sys" );

			// Set the _sched value
			Utility::setTableIntValue( this->L, "_sched", worldTicks + nextTickSchedule );
		}
	}

	int LotEntity::lua_getLotEntityObject( lua_State* L ) {
		BlueBear::LotEntity* lotEntity = ( BlueBear::LotEntity* )lua_touserdata( L, lua_upvalueindex( 1 ) );


	}
}
