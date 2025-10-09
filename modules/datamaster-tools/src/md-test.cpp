#include "DatamasterEtherboneConnection.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>

#include <etherbone.h>

using namespace carpeDM;

int main( int argc, char* argv[] )
{
  DatamasterEtherboneConnection dm( "dev/ttyUSB0" );

  if ( dm.isConnected() )
  {
    auto meta = dm.getMetaInformation();
    std::cout << "Connected to Datamaster with " << meta.numberOfRamDevices << " RAM devices, "
              << meta.numberOfDiagDevices << " DIAG devices and " << meta.numberOfClusterTimeModules
              << " Cluster Time Modules." << std::endl;
  }
  else
  {
    std::cerr << "Could not connect to Datamaster." << std::endl;
    return EXIT_FAILURE;
  }

  auto buildInfo = dm.getBuildInfo();
  std::cout << buildInfo << std::endl;

  /*
  auto ramDevice    = dm.getRAMDevice( 0 );
  auto firstAddress = ramDevice->sdb_component.addr_first;
  auto lastAddress  = ramDevice->sdb_component.addr_last;
  auto deviceSize   = lastAddress - firstAddress + 1;
  std::cout << "First RAM device address range: 0x" << std::hex << firstAddress << " - 0x" << lastAddress << std::dec
            << " (" << deviceSize << " bytes)" << std::endl;

  std::vector<uint32_t> buffer( deviceSize / 4, 0 );

  eb_cycle_t  cycle;
  eb_status_t status = EB_OK;
  status             = eb_cycle_open( dm.getEbDevice(), nullptr, nullptr, &cycle );

  if ( status != EB_OK )
  {
    std::cerr << "Could not open Etherbone cycle: " << eb_status( status ) << std::endl;
    return EXIT_FAILURE;
  }

  for ( int32_t i = 0; i < deviceSize / 4; i++ )
  {
    eb_cycle_read(
        cycle, firstAddress + i * 4, EB_DATA32 | EB_BIG_ENDIAN, reinterpret_cast<eb_data_t*>( &buffer.data()[i] ) );
  }

  status = eb_cycle_close( cycle );
  if ( status != EB_OK )
  {
    std::cerr << "Could not close Etherbone cycle: " << eb_status( status ) << std::endl;
    return EXIT_FAILURE;
  }

  for ( int32_t i = 0; i < deviceSize / 4; i++ )
  {
    std::cout << "0x" << std::hex << std::setfill( '0' ) << std::setw( 8 ) << ( firstAddress + ( i * 4 ) ) << ": ";
    std::cout << std::hex << std::setfill( '0' ) << std::setw( 8 ) << buffer[i] << "\n";
  }
  std::cout << std::endl;
  */

  return EXIT_SUCCESS;
}