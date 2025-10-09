#include <argp.h>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

#include <DatamasterFactory.h>
#include <DotParser.h>
#include <IDatamaster.h>
#include <ScheduleGraph.h>

using namespace carpeDM;

struct argp_option options[] = { { "file", 'f', "FILE", 0, "Input DOT file to validate" }, { 0 } };

namespace
{

enum class Command
{
  status,
  dump,
  clear,
  add,
  overwrite,
  remove,
  keep,
  rawvisited,
  chkrem
};

struct Arguments
{
  std::string            etherboneDevice;
  std::optional<Command> command = std::nullopt;
  std::string            inputDotFile;
  bool                   noVerify       = false;
  std::string            outputDotFile  = "download.dot";
  bool                   printMetaNodes = false;
  bool                   verbose        = false;
  bool                   debug          = false;
  bool                   force          = false;
};

struct argp_option OPTIONS[] = {
  { "dotFile", 'i', "DOTFILE", 0, "Input DOT file to use" },
  { "command", 'c', "COMMAND", 0, "Command to execute" },
  { "no-verify", 'n', 0, 0, "Do not verify the graph before applying it" },
  { "output", 'o', "OUTPUT", 0, "Output DOT file (default: download.dot)" },
  { "meta-nodes", 's', 0, 0, "Include meta nodes in output" },
  { "verbose", 'v', 0, 0, "Enable verbose output" },
  { "debug", 'd', 0, 0, "Enable debug output" },
  { "force", 'f', 0, 0, "Force, overrides the safety check for clear, remove, overwrite and keep (use with caution)" },
  { 0 }
};

bool CommandRequiresDotFile( Command command )
{
  switch ( command )
  {
  case Command::add:
    return true;
  default:
    return false;
  }
}

error_t ParseCommand( const char* const cmd, std::optional<Command>& command, struct argp_state* state )
{
  auto potentialCommand = magic_enum::enum_cast<Command>( cmd, magic_enum::case_insensitive );
  if ( potentialCommand.has_value() )
  {
    command = potentialCommand.value();
    return 0;
  }
  else
  {
    command = std::nullopt;
    argp_error( state, "Error: Unknown command %s", cmd );
    return EINVAL;
  }
}

error_t ParsePositionalArgument( int key, const char* const value, Arguments& arguments, struct argp_state* state )
{
  error_t result = 0;
  switch ( key )
  {
  case 0:
    arguments.etherboneDevice = value;
    break;
  case 1:
    result = ParseCommand( value, arguments.command, state );
    if ( result != 0 )
    {
      argp_error( state, "Error: Unknown command '%s'", value );
    }
    break;
  }
  return result;
}

error_t VerifyArguments( Arguments& arguments, struct argp_state* state )
{
  error_t result = 0;
  if ( arguments.etherboneDevice.empty() )
  {
    argp_error( state, "Error: <etherbone-device> is required.\n" );
    result = EINVAL;
  }

  const auto potentialCommand = arguments.command;
  if ( !potentialCommand.has_value() )
  {
    argp_error( state, "Error: <command> is required.\n" );
    result = EINVAL;
  }
  auto command = potentialCommand.value();

  if ( arguments.inputDotFile.empty() && CommandRequiresDotFile( command ) )
  {
    argp_error( state, "Error: --dotFile is required for the selected command.\n" );
    result = EINVAL;
  }

  return result;
}

error_t ParseArgument( int key, char* arg, struct argp_state* state )
{
  Arguments* arguments = static_cast<Arguments*>( state->input );
  error_t    result    = 0;
  switch ( key )
  {
  case ARGP_KEY_ARG:
    result = ParsePositionalArgument( state->arg_num, arg, *arguments, state );
    break;
  case 'i':
    arguments->inputDotFile = arg;
    break;
  case 'c':
    result = ParseCommand( arg, arguments->command, state );
    break;
  case 'n':
    arguments->noVerify = true;
    break;
  case 'o':
    arguments->outputDotFile = arg;
    break;
  case 's':
    arguments->printMetaNodes = true;
    break;
  case 'v':
    arguments->verbose = true;
    break;
  case 'd':
    arguments->debug = true;
    break;
  case 'f':
    arguments->force = true;
    break;
  case ARGP_KEY_END:
    result = VerifyArguments( *arguments, state );
    break;

  default:
    break;
  }

  return result;
}

struct argp argp_parameters = { OPTIONS, &ParseArgument, "<etherbone-device> <command> [<dotfile>] [<options>]",
                                nullptr, nullptr,        nullptr,
                                nullptr };

std::variant<ScheduleGraph, ConversionError, ParsingError> ParseInputDotFile( const std::string& filename )
{
  DotGraphParser parser;

  // @TODO(mdennst): Move this logic somewhere else
  FILE* file = fopen( filename.c_str(), "r" ); // Test if file can be opened

  if ( file == nullptr )
  {
    std::cerr << "Could not open file " << filename << std::endl;
    return ParsingError{ "Could not open file." };
  }

  fseek( file, 0, SEEK_END );
  size_t fileSize = ftell( file );
  fseek( file, 0, SEEK_SET );
  std::string fileBuffer( fileSize, '\0' );

  fread( fileBuffer.data(), 1, fileSize, file );
  fclose( file );
  ///

  auto potentialGraph = parser.parseGraph( std::string_view( fileBuffer ) );

  if ( std::holds_alternative<ParsingError>( potentialGraph ) )
  {
    auto error = std::get<ParsingError>( potentialGraph );
    return error;
  }

  auto graph                  = std::get<DotGraph>( potentialGraph );
  auto potentialScheduleGraph = ScheduleGraph::fromDotGraph( graph );
  if ( std::holds_alternative<ConversionError>( potentialScheduleGraph ) )
  {
    return std::get<ConversionError>( potentialScheduleGraph );
  }
  return std::get<ScheduleGraph>( potentialScheduleGraph );
}

void ExecuteAddCommand( const Arguments& arguments )
{
  auto datamaster = MakeDatamaster( arguments.etherboneDevice.c_str() );

  if ( !datamaster )
  {
    std::cerr << "Could not create datamaster for device " << arguments.etherboneDevice << std::endl;
    return;
  }
  auto potentialScheduleGraph = ParseInputDotFile( arguments.inputDotFile );
  std::visit(
      [&]( auto&& arg )
      {
        using T = std::decay_t<decltype( arg )>;
        if constexpr ( std::is_same_v<T, ScheduleGraph> )
        {
          datamaster->addGraph( arg );
        }
        else if constexpr ( std::is_same_v<T, ConversionError> )
        {
          std::cerr << "Failed to convert input DOT file:\n" << arg.message << std::endl;
        }
        else if constexpr ( std::is_same_v<T, ParsingError> )
        {
          std::cerr << "Failed to parse input DOT file:\n" << arg.message << std::endl;
        }
      },
      potentialScheduleGraph );
}
} // namespace

int main( int argc, char** argv )
{
  Arguments arguments;
  argp_parse( &argp_parameters, argc, argv, 0, 0, &arguments );
}