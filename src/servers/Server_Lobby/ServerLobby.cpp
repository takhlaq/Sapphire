
#include <stdio.h>
#include <time.h>
#include <map>

#include <src/servers/Server_Common/Network/Hive.h>
#include <src/servers/Server_Common/Network/Acceptor.h>

#include <src/servers/Server_Common/Version.h>
#include <src/servers/Server_Common/Logging/Logger.h>
#include <src/servers/Server_Common/Config/XMLConfig.h>

//#include "LobbySession.h"

#include "ServerLobby.h"

#include "GameConnection.h"
#include "RestConnector.h"

#include <thread>
#include <boost/thread.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/uuid/sha1.hpp>

Core::Logger g_log;
Core::Network::RestConnector g_restConnector;

namespace Core {


   ServerLobby::ServerLobby( const std::string& configPath ) :
      m_configPath( configPath ),
      m_numConnections( 0 )
   {
      m_pConfig = boost::shared_ptr<XMLConfig>( new XMLConfig );
   }

   ServerLobby::~ServerLobby( void )
   {
   }

   LobbySessionPtr ServerLobby::getSession( char* sessionId )
   {
      return g_restConnector.getSession( sessionId );
   }

   XMLConfigPtr ServerLobby::getConfig() const
   {
      return m_pConfig;
   }

   bool ServerLobby::checkVersionString( std::string versionString )
   {
      return m_gameVersion.substr( 0, versionString.size() ) == versionString;
   }

   std::string ServerLobby::generateVersionString( std::string path )
   {
      
      auto f = fopen( ( path + "\\..\\..\\ffxiv.exe" ).c_str(), "rb" );
      fseek( f, 0, SEEK_END );
      auto size = ftell( f );
      std::vector<char> ffxiv_exe_bytes( size );
      rewind( f );
      fread( ffxiv_exe_bytes.data(), 1, size, f );
      fclose( f );

      boost::uuids::detail::sha1 sha1;
      sha1.process_bytes( ffxiv_exe_bytes.data(), size );

      unsigned hash[5] = { 0 };
      sha1.get_digest( hash );

      char ffxiv_exe_sha1[41] = { 0 };

      for (int i = 0; i < 5; i++)
      {
         std::sprintf( ffxiv_exe_sha1 + ( i << 3 ), "%08x", hash[i] );
      }

      f = fopen( ( path + "\\..\\..\\ffxiv_dx11.exe" ).c_str(), "rb" );
      fseek( f, 0, SEEK_END );
      size = ftell( f );
      std::vector<char> ffxiv_dx11_exe_bytes( size );
      rewind( f );
      fread( ffxiv_dx11_exe_bytes.data(), 1, size, f );
      fclose( f );

      sha1.reset();
      sha1.process_bytes( ffxiv_dx11_exe_bytes.data(), size );

      sha1.get_digest( hash );

      char ffxiv_dx11_exe_sha1[41] = { 0 };

      for (int i = 0; i < 5; i++)
      {
         std::sprintf( ffxiv_dx11_exe_sha1 + ( i << 3 ), "%08x", hash[i] );
      }

      std::ifstream ex1stream( path + "\\..\\ex1\\ex1.ver" );
      std::string ex1ver( ( std::istreambuf_iterator<char>( ex1stream ) ),
         std::istreambuf_iterator<char>() );

      std::ifstream ex2stream( path + "\\..\\ex2\\ex2.ver" );
      std::string ex2ver( ( std::istreambuf_iterator<char>( ex2stream ) ),
         std::istreambuf_iterator<char>() );

      ex1ver = ex1ver.substr( 0, ex1ver.size() - 5 );
      ex2ver = ex2ver.substr( 0, ex2ver.size() - 5 );

      return "ffxiv.exe/" + std::to_string( ffxiv_exe_bytes.size() ) + "/" + ffxiv_exe_sha1 + ",ffxiv_dx11.exe/" + std::to_string( ffxiv_dx11_exe_bytes.size() ) + "/" + ffxiv_dx11_exe_sha1 + "+" + ex1ver + "+" + ex2ver;
   }

   void ServerLobby::run( int32_t argc, char* argv[] )
   {
      g_log.setLogPath( "log\\SapphireLobby" );
      g_log.init();

      g_log.info( "===========================================================" );
      g_log.info( "Sapphire Server Project " );
      g_log.info( "Version: " + Version::VERSION );
      g_log.info( "Git Hash: " + Version::GIT_HASH );
      g_log.info( "Compiled: " __DATE__ " " __TIME__ );
      g_log.info( "===========================================================" );

      if( !loadSettings( argc, argv ) )
      {
         g_log.fatal( "Error loading settings! " );
         return;
      }

      m_gameVersion = generateVersionString( m_pConfig->getValue< std::string >( "Settings.General.DataPath", "" ) );
      g_log.info( "Game version: " + m_gameVersion );

      Network::HivePtr hive( new Network::Hive() );
      Network::addServerToHive< Network::GameConnection >( m_ip, m_port, hive );

      g_log.info( "Lobbyserver ready for connections on " + m_ip + ":" + std::to_string(m_port) );

      boost::thread_group worker_threads;
      worker_threads.create_thread( boost::bind( &Network::Hive::Run, hive.get() ) );
      worker_threads.join_all();

   }

   bool ServerLobby::loadSettings( int32_t argc, char* argv[] )
   {
      g_log.info( "Loading config " + m_configPath );

      if( !m_pConfig->loadConfig( m_configPath ) )
      {
         g_log.fatal( "Error loading config " + m_configPath );
         return false;
      }
      std::vector<std::string> args( argv + 1, argv + argc );
      for( size_t i = 0; i + 1 < args.size(); i += 2 )
      {
         std::string arg( "" );
         std::string val( "" );

         try
         {
            arg = boost::to_lower_copy( std::string( args[i] ) );
            val = std::string( args[i + 1] );

            // trim '-' from start of arg
            arg = arg.erase( 0, arg.find_first_not_of( '-' ) );

            if( arg == "ip" )
            {
               // todo: ip addr in config
               m_pConfig->setValue< std::string >( "Settings.General.ListenIp", val );
            }
            else if( arg == "p" || arg == "port" )
            {
               m_pConfig->setValue< std::string >( "Settings.General.ListenPort", val );
            }
            else if( arg == "ap" || arg == "auth" || arg == "authport" )
            {
               m_pConfig->setValue< std::string>( "Settings.General.AuthPort", val );
            }
            else if( arg == "worldip" || arg == "worldip" )
            {
               m_pConfig->setValue < std::string >( "Settings.General.WorldIp", val );
            }
            else if( arg == "worldport" )
            {
               m_pConfig->setValue< std::string >( "Settings.General.WorldPort", val );
            }
         }
         catch( ... )
         {
            g_log.error( "Error parsing argument: " + arg + " " + "value: " + val + "\n" );
            g_log.error( "Usage: <arg> <val> \n" );
         }
      }

      m_port = m_pConfig->getValue< uint16_t >( "Settings.General.ListenPort", 54994 );
      m_ip = m_pConfig->getValue< std::string >( "Settings.General.ListenIp", "0.0.0.0" );

      g_restConnector.restHost = m_pConfig->getValue< std::string >( "Settings.General.RestHost" );
      g_restConnector.serverSecret = m_pConfig->getValue< std::string >( "Settings.General.ServerSecret" );

      return true;
   }
}
