/**
 * @file dm-reduce.cpp
 * @brief Main file for the dm-reduce tool.
 *
 * This tool is meant to reduce a given dot graph by removing as many nodes as possible while keeping a given set of
 * nodes intact.
 */

#include <boost/program_options.hpp>
#include <iostream>

#include <alloctable.h>
#include <carpeDM.h>
#include <globalreftable.h>
#include <graphNodes.h>
#include <node.h>
#include <reflocation.h>
#include <visitoruploadcrawler.h>

boost::program_options::variables_map handleOptions( int argc, char* argv[] )
{
  namespace po = boost::program_options;

  po::options_description desc( "Allowed options" );
  auto                    add_option = desc.add_options();
  add_option( "help,h", "produce help message" );
  add_option( "input,i", po::value<std::string>()->required(), "input dot file" );
  add_option( "output,o", po::value<std::string>()->required(), "output dot file" );
  add_option( "keep,k", po::value<std::vector<std::string>>()->multitoken()->required(), "nodes to keep (by name)" );
  add_option( "verbose,v", "enable verbose output" );

  po::variables_map vm;
  try
  {
    po::store( po::parse_command_line( argc, argv, desc ), vm );
    po::notify( vm );
  }
  catch ( const po::error& e )
  {
    std::cerr << "Error parsing command line: " << e.what() << std::endl;
    std::cerr << desc << std::endl;
    exit( 1 );
  }

  if ( vm.count( "help" ) )
  {
    std::cout << desc << std::endl;
    exit( 0 );
  }

  return vm;
}

void FilterGraphByRules( Graph& g, const std::vector<std::string>& nodesToKeep )
{
  BOOST_FOREACH ( edge_t e, boost::edges( g ) )
  {
    vertex_t source = boost::source( e, g );
    vertex_t target = boost::target( e, g );

    if ( g[e].type == DotStr::Edge::TypeVal::sDefDst )
    {
      if ( std::find( nodesToKeep.begin(), nodesToKeep.end(), g[target].name ) == nodesToKeep.end() )
      {
        auto target_edges = boost::out_edges( target, g ); // Get all edges of the target node
        // If the target node is not in the keep list, remove the edge
        boost::remove_edge( e, g );
        boost::remove_vertex( target, g );
      }
    }
  }
}

int main( int argc, char* argv[] )
{
  auto vm = handleOptions( argc, argv );

  std::string              inputFile   = vm["input"].as<std::string>();
  std::string              outputFile  = vm["output"].as<std::string>();
  std::vector<std::string> nodesToKeep = vm["keep"].as<std::vector<std::string>>();
  bool                     verbose     = vm.count( "verbose" ) > 0;

  CarpeDM     dm( std::cout, std::cerr );
  std::string inputDotContent = dm.readTextFile( inputFile );

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

  // Filter Graph by Rules.....

  dm.writeDotFile( outputFile, g, true );
}