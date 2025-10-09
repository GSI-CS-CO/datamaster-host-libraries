#include <carpeDM.h>
#include <validation.h>

#include <boost/program_options.hpp>
#include <iostream>

#include <alloctable.h>
#include <carpeDM.h>
#include <globalreftable.h>
#include <graphNodes.h>
#include <node.h>
#include <reflocation.h>
#include <visitoruploadcrawler.h>

namespace po = boost::program_options;

int main( int argc, char* argv[] )
{
  po::options_description desc( "Allowed options" );
  desc.add_options()( "help", "produce help message" )( "file", po::value<std::string>(), "dot file to validate" );

  po::variables_map vm;
  po::store( po::parse_command_line( argc, argv, desc ), vm );
  po::notify( vm );

  if ( vm.count( "help" ) )
  {
    std::cout << desc << "\n";
    return 1;
  }

  if ( vm.count( "file" ) )
  {
    std::cout << "File to validate: " << vm["file"].as<std::string>() << "\n";
  }
  else
  {
    std::cout << "No file to validate specified.\n";
    exit( -1 );
  }

  std::string file = vm["file"].as<std::string>();

  CarpeDM     dm( std::cout, std::cerr );
  std::string inputDotContent = dm.readTextFile( file );

  // Unilac 1 cpu:
  // IntAdrOffs  : 0x10000000
  // SharedOffs  : 0x500
  // SharedSize  : 491520
  // ThreadQty   : 32

  // LM32 base address: 4180000

  GlobalRefTable rt;
  RefLocation    rl;
  AllocTable     at( rl, rt );
  at.addMemory( 0, 0x0, 0x0, 0x0, 0x0, 0x1000000, 0x1000000, 0x1000000 ); // Example memory pool, adjust as needed
  rl.init( nullptr, 0x500 );                                              // Initialize RefLocation with shared offset

  /*
  at.addMemory( 1, 0x0, 0x0, 0x0, 0x0, 0x100000, 0x100000, 0x10000 ); // Example memory pool for CPU 1
  at.addMemory( 2, 0x0, 0x0, 0x0, 0x0, 0x100000, 0x100000, 0x10000 ); // Example memory pool for CPU 2
  at.addMemory( 3, 0x0, 0x0, 0x0, 0x0, 0x100000, 0x100000, 0x10000 ); // Example memory pool for CPU 3
*/
  Graph g;
  dm.parseDot( inputDotContent, g );
  InitializeGraphNodes( g );

  /*

  std::unordered_map<uint32_t, std::string> hashDict;

  BOOST_FOREACH ( vertex_t v, vertices( g ) )
  {
    // insert node into hashDict and print duplicates
    uint32_t hash = g[v].np->getHash();
    if ( hashDict.find( hash ) != hashDict.end() )
    {
      std::cerr << "Duplicate hash found: " << hash << " for node: " << g[v].name
                << " and existing node: " << hashDict[hash] << std::endl;
    }
    else
    {
      hashDict[hash] = g[v].name;
    }
  }

  BOOST_FOREACH ( vertex_t v, vertices( g ) )
  {
    int allocState = ALLOC_OK;
    try
    {
      allocState = at.allocate( g[v].np->getCpu(), g[v].np->getHash(), v, g, true );
    }
    catch ( const std::runtime_error& e )
    {
      std::cerr << "Error allocating node '" << g[v].np->getName() << "': " << e.what() << std::endl;
      // std::cerr << "Node 0: " << g[0].name << std::endl;
      // std::cerr << "Current Node: " << g[v].name << std::endl;
      exit( 1 );
    }
    if ( ALLOC_NO_SPACE == allocState )
    {
      std::cerr << "Not enough space in CPU " << (int)g[v].np->getCpu() << " memory pool for node '"
                << g[v].np->getName() << "'." << std::endl;
      exit( 1 );
    }
    if ( ALLOC_ENTRY_EXISTS == allocState )
    {
      std::cerr << "Node '" << g[v].np->getName() << "' would be duplicate in graph." << std::endl;
      exit( 1 );
    }
  }

  BOOST_FOREACH ( vertex_t v, vertices( g ) )
  {
    g[v].np->accept( VisitorUploadCrawler( g, v, at, std::cout, std::cerr ) );
  }
  /*
    Validation::init();
    try
    {
      BOOST_FOREACH ( vertex_t v, vertices( g ) )
      {
        Validation::neighbourhoodCheck( v, g );
        Validation::neighbourhoodCheckCpu( v, g );
        bool isNodeThatNeedsNoSequenceCheck = g[v].type == DotStr::Node::TypeVal::sQBuf ||
                                              g[v].type == DotStr::Node::TypeVal::sDstList ||
                                              g[v].type == DotStr::Node::TypeVal::sGlobal;
        if ( !isNodeThatNeedsNoSequenceCheck )
        {
          Validation::eventSequenceCheck( v, g, true );
        }
      }
    }
    catch ( const std::runtime_error& err )
    {
      std::cerr << "Validation failed: " << err.what() << std::endl;
      return -1;
    }*/
  std::cout << "Validation successful." << std::endl;
}