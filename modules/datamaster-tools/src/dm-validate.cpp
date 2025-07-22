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

  GlobalRefTable rt;
  RefLocation    rl;
  AllocTable     at( rl, rt );
  at.addMemory( 0, 0x0, 0x0, 0x0, 0x0, 0x100000, 0x100000, 0x10000 ); // Example memory pool, adjust as needed
  at.addMemory( 1, 0x0, 0x0, 0x0, 0x0, 0x100000, 0x100000, 0x10000 ); // Example memory pool for CPU 1
  at.addMemory( 2, 0x0, 0x0, 0x0, 0x0, 0x100000, 0x100000, 0x10000 ); // Example memory pool for CPU 2
  at.addMemory( 3, 0x0, 0x0, 0x0, 0x0, 0x100000, 0x100000, 0x10000 ); // Example memory pool for CPU 3

  Graph g;
  dm.parseDot( inputDotContent, g );
  InitializeGraphNodes( g );

  BOOST_FOREACH ( vertex_t v, vertices( g ) )
  {
    const auto allocState = at.allocate( g[v].np->getCpu(), g[v].np->getHash(), v, g, true );
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
  }
  std::cout << "Validation successful." << std::endl;
}