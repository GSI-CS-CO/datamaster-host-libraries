#include <cstdlib>
#include <iostream>

#include <boost/program_options.hpp>

#include <DotParser.h>
#include <ScheduleGraph.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <argp.h>

using namespace carpeDM;

namespace
{
struct Arguments
{
  char* filename = nullptr;
};

error_t ParseArgument( int key, char* arg, struct argp_state* state )
{
  Arguments* arguments = static_cast<Arguments*>( state->input );
  switch ( key )
  {
  case 'f':
    arguments->filename = arg;
    break;
  case ARGP_KEY_ARG:
    if ( state->arg_num > 0 )
    {
      // Too many arguments
      argp_error( state, "Error: Too many arguments.\n" );
      return EINVAL;
    }
    arguments->filename = arg;
    break;
  case ARGP_KEY_END:
    if ( arguments->filename == nullptr )
    {
      argp_error( state, "Error: --file is required.\n" );
    }
    break;
  }

  return 0;
}
} // namespace

int main( int argc, char* argv[] )
{
  Arguments          arguments;
  struct argp_option options[] = { { "file", 'f', "FILE", 0, "Input DOT file to validate" }, { 0 } };
  struct argp        argp      = { options, &ParseArgument, nullptr, nullptr, nullptr, nullptr, nullptr };

  argp_parse( &argp, argc, argv, 0, 0, &arguments );

  if ( nullptr == arguments.filename )
  {

    return EXIT_SUCCESS;
  }

  FILE* file = fopen( arguments.filename, "r" ); // Test if file can be opened

  if ( file == nullptr )
  {
    std::cerr << "Error: Could not open file " << arguments.filename << std::endl;
    return EXIT_FAILURE;
  }

  struct stat fileStat;
  if ( fstat( fileno( file ), &fileStat ) != 0 ) // Get file size
  {
    std::cerr << "Error: Could not stat file " << arguments.filename << std::endl;
    fclose( file );
    return EXIT_FAILURE;
  }

  size_t fileSize = fileStat.st_size;
  if ( fileSize == 0 )
  {
    std::cerr << "Error: File " << arguments.filename << " is empty" << std::endl;
    fclose( file );
    return EXIT_FAILURE;
  }

  const off_t fileOffset = 0;
  char*       fileBuffer =
      static_cast<char*>( mmap( nullptr, fileSize, PROT_READ, MAP_PRIVATE, fileno( file ), fileOffset ) );
  if ( fileBuffer == MAP_FAILED )
  {
    std::cerr << "Error: Could not mmap file " << arguments.filename << std::endl;
    fclose( file );
    return EXIT_FAILURE;
  }

  DotGraphParser parser;
  auto           graph = parser.parseGraph( std::string_view( fileBuffer, fileSize ) );
  if ( std::holds_alternative<ParsingError>( graph ) )
  {
    std::cerr << "Parsing Error: " << std::get<ParsingError>( graph ).message << std::endl;
    munmap( fileBuffer, fileSize );
    fclose( file );
    return EXIT_FAILURE;
  }

  auto scheduleGraph = ScheduleGraph::fromDotGraph( std::get<DotGraph>( graph ) );
  if ( std::holds_alternative<ConversionError>( scheduleGraph ) )
  {
    std::cerr << "Conversion Error: " << std::get<ConversionError>( scheduleGraph ).message << std::endl;
    munmap( fileBuffer, fileSize );
    fclose( file );
    return EXIT_FAILURE;
  }

  std::cout << "Successfully parsed and converted graph '" << std::get<ScheduleGraph>( scheduleGraph ).getName()
            << "' from file " << arguments.filename << std::endl;

  munmap( fileBuffer, fileSize );
  fclose( file );

  return EXIT_SUCCESS;
}