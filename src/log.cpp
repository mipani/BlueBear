#include "log.hpp"
#include "configmanager.hpp"
#include "eventmanager.hpp"
#include <string>
#include <map>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>

namespace BlueBear {

  Log::Log() {
    // The ConfigManager can now never ever log anything
    // The EventManager also can now never log anything
    minimumReportableLevel = ( LogLevel )ConfigManager::getInstance().getIntValue( "min_log_level" );
    mode = ( LogMode )ConfigManager::getInstance().getIntValue( "logger_mode" );
    logFile.open( ConfigManager::getInstance().getValue( "logfile_path" ), std::ios_base::app );
    logFile << std::endl;
  }

  std::map< Log::LogLevel, std::string > Log::Colors = {
    { LogLevel::LEVEL_DEBUG, std::string( Log::ANSI_GREEN ) },
    { LogLevel::LEVEL_INFO, std::string( Log::ANSI_BLUE ) },
    { LogLevel::LEVEL_WARN, std::string( Log::ANSI_YELLOW ) },
    { LogLevel::LEVEL_ERROR, std::string( Log::ANSI_RED ) }
  };

  std::map< Log::LogLevel, std::string > Log::StringTypes = {
    { LogLevel::LEVEL_DEBUG, "d" },
    { LogLevel::LEVEL_INFO, "i" },
    { LogLevel::LEVEL_WARN, "w" },
    { LogLevel::LEVEL_ERROR, "e" }
  };

  void Log::out( const LogMessage& message ) {
    std::unique_lock< std::mutex > lock( mutex );

    messages.push_back( message );

    if( message.level >= minimumReportableLevel ) {
      // sucks
      if( mode == LogMode::CONSOLE || mode == LogMode::BOTH ) {
        outToConsole( messageToString( message, true ) );
      }
      if( mode == LogMode::FILE || mode == LogMode::BOTH ) {
        outToFile( messageToString( message, false ) );
      }

      // SUCKS
      eventManager.MESSAGE_LOGGED.trigger( messageToString( message, false ) );
    }
  }

  void Log::outToConsole( const std::string& text ) {
    std::cout << text << std::endl;
  }

  void Log::outToFile( const std::string& text ) {
    logFile << text << std::endl;
  }

  std::string Log::messageToString( const LogMessage& message, bool accent ) {
    auto time = std::time( nullptr );
    auto localtime = *std::localtime( &time );
    std::stringstream stream;

    if( accent ) {
      #ifdef _WIN32
        // TODO: MS-DOS console colors
        stream << "(" << Log::StringTypes[ message.level ] << ") " << std::put_time( &localtime, "%Y-%m-%d %H:%M:%S: " ) << "[" << message.tag << "] " << message.message;
      #else
        stream << Log::Colors[ message.level ] << "(" << Log::StringTypes[ message.level ] << ") " << std::put_time( &localtime, "%Y-%m-%d %H:%M:%S: " ) << Log::ANSI_RESET << "[" << message.tag << "] " << message.message;
      #endif
    } else {
      stream << "(" << Log::StringTypes[ message.level ] << ") " << std::put_time( &localtime, "%Y-%m-%d %H:%M:%S: " ) << "[" << message.tag << "] " << message.message;
    }

    return stream.str();
  }

  void Log::debug( const std::string& tag, const std::string& message ) {
    out( LogMessage { tag, message, LogLevel::LEVEL_DEBUG } );
  }

  void Log::info( const std::string& tag, const std::string& message ) {
    out( LogMessage { tag, message, LogLevel::LEVEL_INFO } );
  }

  void Log::warn( const std::string& tag, const std::string& message ) {
    out( LogMessage { tag, message, LogLevel::LEVEL_WARN } );
  }

  void Log::error( const std::string& tag, const std::string& message ) {
    out( LogMessage { tag, message, LogLevel::LEVEL_ERROR } );
  }
}
