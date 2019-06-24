#include "tools/utility.hpp"
#include "eventmanager.hpp"
#include "log.hpp"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <string>
#include <cstring>
#include <jsoncpp/json/json.h>
#include <string>
#include <iterator>
#include <fstream>
#include <vector>
#include <sstream>
#include <limits>

#if defined(_WIN32) || defined(FS_EXPERIMENTAL)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace BlueBear {
	namespace Tools {
		/**
		 * Dump the Lua stack out to terminal
		 */
		void Utility::stackDump( lua_State* L ) {
			  int i;
			  int top = lua_gettop(L);
			  for (i = 1; i <= top; i++) {  /* repeat for each level */
					Utility::stackDumpAt( L, -i );
					printf("  ");  /* put a separator */
			  }

			  printf("\n");  /* end the listing */
		}

		void Utility::stackDumpAt( lua_State* L, int pos ) {
			int t = lua_type(L, pos);
			switch (t) {

				case LUA_TSTRING:  /* strings */
				printf("`%s'", lua_tostring(L, pos));
				break;

				case LUA_TBOOLEAN:  /* booleans */
				printf(lua_toboolean(L, pos) ? "true" : "false");
				break;

				case LUA_TNUMBER:  /* numbers */
				printf("%g", lua_tonumber(L, pos));
				break;

				case LUA_TNIL: /* nils */
				printf("nil");
				break;

				case LUA_TTABLE: /* table */
				printf("table");
				break;

				case LUA_TFUNCTION: /* function */
				printf("function");
				break;

				default:  /* other values */
				printf("other type:%s", lua_typename(L, t));
				break;

			}
		}

		/**
		 * Gets a collection of subdirectories for the given directory
		 */
		std::vector< std::string > Utility::getSubdirectoryList( const char* rootSubDirectory ) {
			std::vector< std::string > result;
			for( const auto& entry : fs::directory_iterator( rootSubDirectory ) ) {
				result.emplace_back( entry.path().filename().string() );
			}

			return result;
		}

		/**
		 * Don't use this in your mods.
		 */
		int Utility::lua_getPointer( lua_State* L ) {
			lua_pushstring( L, Tools::Utility::pointerToString( lua_topointer( L, -1 ) ).c_str() );

			return 1;
		}

		void Utility::clearLuaStack( lua_State* L ) {
			lua_settop( L, 0 );
		}

		void Utility::getTableValue( lua_State* L, const char* key ) {
			// Push the desired key onto the stack
			lua_pushstring( L, key );

			// Push table[key] onto the stack
			lua_gettable( L, -2 );
		}

		void Utility::setTableIntValue( lua_State* L, const char* key, int value ) {
			lua_pushstring( L, key );
			lua_pushnumber( L, (double) value );
			lua_settable( L, -3 );
		}

		void Utility::setTableStringValue( lua_State* L, const char* key, const char* value ) {
			lua_pushstring( L, key );
			lua_pushstring( L, value );
			lua_settable( L, -3 );
		}

		void Utility::setTableFunctionValue( lua_State* L, const char* key, lua_CFunction value ) {
			lua_pushstring( L, key );
			lua_pushcfunction( L, value );
			lua_settable( L, -3 );
		}

		/**
		 * Tokenise a std::string based on a char value
		 */
		std::vector<std::string> Utility::split(const std::string &text, char sep) {
		  std::vector<std::string> tokens;
		  std::size_t start = 0, end = 0;
		  while ((end = text.find(sep, start)) != std::string::npos) {
		    tokens.push_back(text.substr(start, end - start));
		    start = end + 1;
		  }
		  tokens.push_back(text.substr(start));
		  return tokens;
		}

		/**
		 * C++ doesn't have one of these built into the damn string class.
		 */
		std::string Utility::join( const std::vector< std::string >& strings, const std::string& token ) {
			std::string result;

			unsigned int size = strings.size();
			for( unsigned int i = 0; i != size; i++ ) {
				result += strings[ i ];
				if ( i != size - 1 ) {
					result += token;
				}
			}

			return result;
		}

		/**
		 * Traverse and retrieve a value stored in a Lua table as a tree structure.
		 * Leaves the found value on the top of the stack; leaves nil if the value was not found.
		 * Always pops the original table.
		 */
		void Utility::getTableTreeValue( lua_State* L, const std::string& treeValue ) {
			// start with <table>

			// Get tokens
			auto treeTokens = Utility::split( treeValue, '.' );

			for( const auto& token : treeTokens ) {
				// <subtable> <table> if subtable found
				Utility::getTableValue( L, token.c_str() );

				// If this is not a table, we cannot continue
				if( !lua_istable( L, -1 ) ) {
					// Pop what's on top and whatever's underneath
					// EMPTY
					lua_pop( L, 2 );

					// nil
					lua_pushnil( L );

					return;
				}

				// Pop the table under this table
				// <subtable>
				lua_remove( L, -2 );
			}

			// <desired table>
		}

		/**
		 * Determines if Json::Value is an RLE object. An RLE object is an object, contains a numeric "run" field, and a defined "value" field.
		 */
		bool Utility::isRLEObject( Json::Value& value ) {
			return value.isObject() &&
				value.get( "run", Json::Value::null ).isInt() &&
				!( value.get( "value", Json::Value::null ).isNull() );
		}

		std::string Utility::stringLtrim( std::string& s ) {
			if( s.size() == 0 ) {
				return "";
			}

			return s.substr( s.find_first_not_of( " \t\f\v\n\r" ) );
		}

		std::string Utility::stringRtrim( std::string& s ) {
			if( s.size() == 0 ) {
				return "";
			}

			return s.erase( s.find_last_not_of( " \t\f\v\n\r" ) + 1 );
		}

		std::string Utility::stringTrim( std::string s ) {
			std::string rTrimmed = Utility::stringRtrim( s );

			return Utility::stringLtrim( rTrimmed );
		}

		std::string Utility::pointerToString( const void* pointer ) {
			std::stringstream ss;
			ss << pointer;
			return ss.str();
		}

		void* Utility::stringToPointer( const std::string& str ) {
			std::stringstream ss;
			ss << str;

			uintptr_t ptr = 0x0;
			ss >> std::hex >> ptr;
			return reinterpret_cast< void* >( ptr );
		}

		/**
		 * Shitty decode method for the UTF-8 strings JsonCpp produces
		 */
		std::string Utility::decodeUTF8( const std::string& encoded ) {
			std::stringstream stringBuilder;

			for( int i = 0; i < encoded.length(); i++ ) {
				char c = encoded[ i ];
				unsigned char uc = ( unsigned char ) c;
				int ic = ( int )uc;

				if( ic == 0xC2 ) {
					// Drop 0xC2 and use the next byte directly.
					i++;
					stringBuilder << encoded[ i ];
				} else if( ic == 0xC3 ) {
					// Drop 0xC3, add 0x40 to the next byte, and use that.
					i++;
					stringBuilder << ( char )( encoded[ i ] + 0x40 );
				} else {
					// Use the byte as-is.
					stringBuilder << c;
				}
			}

			return stringBuilder.str();
		}

		/**
		 * THE AGONY
		 */
		std::string Utility::xmlToString( tinyxml2::XMLElement* element ) {
			tinyxml2::XMLPrinter printer;
			element->Accept( &printer );
			return printer.CStr();
		}

		tinyxml2::XMLElement* Utility::getRootNode( tinyxml2::XMLDocument& document, const std::string& xmlString ) {
			document.Parse( xmlString.c_str() );

			if( document.ErrorID() ) {
				Log::getInstance().error( "Utility::getRootNode", "Could not parse XML string!" );
				return nullptr;
			}

			tinyxml2::XMLElement* element = document.RootElement();
			if( !element ) {
				Log::getInstance().error( "Utility::getRootNode", "No root element!" );
				return nullptr;
			}

			return element;
		}

		std::string Utility::generateIndentation( unsigned int amount ) {
			std::string indentation;

			for( unsigned int i = 0; i != amount; i++ ) {
				indentation += "\t";
			}

			return indentation;
		}

		std::string Utility::sanitizeXML( const std::string& input ) {
			std::string result;
			result.reserve( input.size() );

			for( const char c : input ) {
				switch( c ) {
					case '<':
						result.append( "&lt;" );
						break;
					case '>':
						result.append( "&gt;" );
						break;
					case '\"':
						result.append( "&quot;" );
						break;
					case '\'':
						result.append( "&apos;" );
						break;
					case '&':
						result.append( "&amp;" );
						break;
					default:
						result.append( &c, 1 );
				}
			}

			return result;
		}

		Json::Value Utility::stringToJson( const std::string& string ) {
			Json::Value root;
			Json::Reader reader;

			if( !reader.parse( string.c_str(), root ) ) {
				throw InvalidJSONException();
			}

			return root;
		}

		std::string Utility::jsonToString( const Json::Value& json ) {
			Json::StreamWriterBuilder builder;
			return Json::writeString( builder, json );
		}

		std::pair< std::string, std::reference_wrapper< const Json::Value > > Utility::jsonIteratorToPair( Json::Value::const_iterator it ) {
			return std::make_pair( it.key().asString(), std::ref( *it ) );
		}

		Json::Value Utility::fileToJson( const std::string& path ) {
			std::ifstream file( path );
			Json::Value json;
			Json::Reader reader;

			if( reader.parse( file, json ) ) {
				return json;
			} else {
				Log::getInstance().error( "Utility::fileToJson", "Failed to open JSON file: " + path );
				return {};
			}
		}

		/**
		 * Shitty, over-complicated floating point comparison that is completely unnecessary for what a game needs
		 * Original code CC-BY 3.0 from https://floating-point-gui.de/errors/comparison/
		 * Changes made by ne0ndrag0n
		 */
		bool compareFloat( float a, float b, float epsilon = 0.0001f ) {
			float absA = std::abs( a );
			float absB = std::abs( b );
			float diff = std::abs( a - b );

			if( a == b ) {
				return true;
			} else if( a == 0.0f || b == 0.0f || diff < std::numeric_limits< float >::min() ) {
				return diff < ( epsilon * std::numeric_limits< float >::min() );
			} else {
				return diff / std::min( ( absA + absB ), std::numeric_limits< float >::max() ) < epsilon;
			}
		}

		bool Utility::equalEpsilon( float a, float b ) {
			return compareFloat( a, b );
		}

		bool Utility::equalEpsilon( const glm::vec2& a, const glm::vec2& b ) {
			return compareFloat( a.x, b.x ) && compareFloat( a.y, b.y );
		}

		bool Utility::equalEpsilon( const glm::vec3& a, const glm::vec3& b ) {
			return compareFloat( a.x, b.x ) && compareFloat( a.y, b.y ) && compareFloat( a.z, b.z );
		}

		bool Utility::equalEpsilon( const glm::vec4& a, const glm::vec4& b ) {
			return compareFloat( a.x, b.x ) && compareFloat( a.y, b.y ) && compareFloat( a.z, b.z ) && compareFloat( a.w, b.w );
		}

		glm::ivec2 Utility::normalize( const glm::ivec2& candidate ) {
			return glm::ivec2{
				candidate.x == 0 ? 0 : ( candidate.x < 0 ? -1 : 1 ),
				candidate.y == 0 ? 0 : ( candidate.y < 0 ? -1 : 1 ),
			};
		}

		int Utility::distance( const glm::ivec2& start, const glm::ivec2& end ) {
			return glm::distance( glm::vec2{ start.x, start.y }, glm::vec2{ end.x, end.y } );
		}

		float Utility::distance( const glm::vec3& start, const glm::vec3& end ) {
			return glm::distance( start, end );
		}

		float Utility::cross( const glm::vec2& a, const glm::vec2& b ) {
			return a.x * b.y - a.y * b.x;
		}

	}
}
