#include "LM32MemoryRegion.h"

#include <etherbone.h>

using carpeDM::LM32MemoryRegion;

LM32MemoryRegion::LM32MemoryRegion( uint64_t baseAddress, uint64_t size, IDatamasterEtherboneConnection& connection )
    : m_baseAddress( baseAddress )
    , m_size( size )
    , m_connection( connection ){};

/*
void LM32MemoryRegion::read( uint64_t address, std::span<uint8_t> memory ) const
{
eb_cycle_t  cycle;
eb_status_t status = EB_OK;
status             = eb_cycle_open( m_connection.getEbDevice(), nullptr, nullptr, &cycle );

const auto ramDevice      = m_connection.getRAMDevice( 0 );
auto       firstAddress   = ramDevice->sdb_component.addr_first;
auto       buildInfoStart = m_baseAddress + address;

if ( status != EB_OK )
{
return "";
}

for ( size_t i = 0; i < memory.size(); i++ )
{
eb_cycle_read( cycle, buildInfoStart + i * 4, EB_DATA32 | EB_BIG_ENDIAN, (eb_data_t*)( &memory[i] ) );
}

status = eb_cycle_close( cycle );

std::transform( memory.begin(),
              memory.end(),
              memory.begin(),
              []( auto val )
              {
                return __builtin_bswap32( val );
              } );

if ( status != EB_OK )
{
return "";
}

return std::string( reinterpret_cast<char*>( &buildInfo[0] ) );
};
*/
void LM32MemoryRegion::write( uint64_t address, const std::span<uint8_t>& memory )
{
}