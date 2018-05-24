#ifndef UTILITY
#define UTILITY

#include "exceptions/genexc.hpp"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <cstdint>
#include <jsoncpp/json/json.h>
#include <tinyxml2.h>
#include <cstddef>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <glm/glm.hpp>

namespace BlueBear {
	namespace Tools {
		class Utility {

			static constexpr const char* UNKNOWN_STRING = "unknown";
			static constexpr const char* DIRECTORY_STRING = "directory";
			static constexpr const char* FILE_STRING = "file";

			public:
				enum class FilesystemType : int { UNKNOWN, DIRECTORY, FILE };
				struct DirectoryEntry {
					FilesystemType type;
					std::string name;
				};

				EXCEPTION_TYPE( InvalidJSONException, "Invalid JSON string!" );

				static void stackDump( lua_State* L );

				static void stackDumpAt( lua_State* L, int pos );

				static std::vector< std::string > getSubdirectoryList( const char* rootSubDirectory );

				static std::vector< DirectoryEntry > getFileList( const std::string& parent );

				static int lua_getFileList( lua_State* L );

				static int lua_getPointer( lua_State* L );

				static void clearLuaStack( lua_State* L );

				static void getTableValue( lua_State* L, const char* key );

				static void setTableIntValue( lua_State* L, const char* key, int value );

				static void setTableStringValue( lua_State* L, const char* key, const char* value );

				static void setTableFunctionValue( lua_State* L, const char* key, lua_CFunction value );

				static std::vector<std::string> split(const std::string &text, char sep);

				static std::string join( const std::vector< std::string >& strings, const std::string& token );

				static void getTableTreeValue( lua_State* L, const std::string& treeValue );

				static bool isRLEObject( Json::Value& value );

				static std::string stringLtrim( std::string& s );

				static std::string stringRtrim( std::string& s );

				static std::string stringTrim( std::string s );

				static std::string pointerToString( const void* pointer );

				static void* stringToPointer( const std::string& str );

				static constexpr unsigned int hash( const char* str, int h = 0 ) {
					if( !str ) {
						str = "";
					}

					return !str[ h ] ? 5381 : ( hash( str, h+1 ) * 33 ) ^ str[ h ];
				};

				static std::string decodeUTF8( const std::string& encoded );

				/**
				 * C++ std::string is too fucking stupid to know what a null string is
				 */
				static inline const char* sanitizeCString( const char* string ) {
					return !string ? "" : string;
				};

				static std::string xmlToString( tinyxml2::XMLElement* element );

				static tinyxml2::XMLElement* getRootNode( tinyxml2::XMLDocument& document, const std::string& xmlString );

				// released under boost licence
				template < class T >
				static inline void hashCombine( std::size_t& seed, const T& v ) {
    			std::hash< T > hasher;
    			seed ^= hasher( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
				}

				static std::string generateIndentation( unsigned int amount );

				template< class T >
				static inline std::vector< T > concatArrays( const std::vector< T >& left, const std::vector< T >& right ) {
					std::vector< T > result = left;

					for( auto& r : right ) {
						result.push_back( r );
					}

					return result;
				};

				static constexpr const char* safeString( const char* cString ) {
					return cString ? cString : "";
				};

				static bool intersect( const glm::ivec2& pointer, const glm::ivec4& target ) {
					return pointer.x >= target[ 0 ] &&
								 pointer.y >= target[ 1 ] &&
								 pointer.x <= target[ 2 ] &&
								 pointer.y <= target[ 3 ];
				};

				template < typename Numeric > static constexpr Numeric interpolateLinear( Numeric v0, Numeric v1, double alpha ) {
					return ( 1 - alpha ) * v0 + alpha * v1;
				};

				static std::string sanitizeXML( const std::string& input );

				static Json::Value stringToJson( const std::string& string );

				static std::string jsonToString( const Json::Value& json );

				static std::pair< std::string, std::reference_wrapper< const Json::Value > > jsonIteratorToPair( Json::Value::const_iterator it );

				static Json::Value fileToJson( const std::string& path );
		};
	}
}


#endif
