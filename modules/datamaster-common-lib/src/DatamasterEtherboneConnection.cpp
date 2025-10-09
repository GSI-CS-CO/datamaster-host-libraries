
#include "DatamasterEtherboneConnection.h"

#include <algorithm>
#include <iostream>

using carpeDM::DatamasterEtherboneConnection;
using carpeDM::DatamasterMetaInformation;

namespace
{
constexpr uint64_t SDB_VENDOR_GSI                 = 0x651ULL;
constexpr uint32_t SDB_DEVICE_LM32_RAM            = 0x54111351;
constexpr uint32_t SDB_DEVICE_DIAG                = 0x18060200;
constexpr uint32_t SDB_DEVICE_CLUSTER_TIME_MODULE = 0x10041233;

constexpr uint32_t BUILDID_OFFSET = 0x100;
constexpr uint32_t BUILDID_SIZE   = 0x400;

void RetrieveDevices( eb_device_t device, uint64_t vendorId, uint32_t deviceId, std::vector<sdb_device>& devices )
{
  int32_t numberOfDevices    = 0;
  auto    findIdentityResult = eb_sdb_find_by_identity( device, vendorId, deviceId, nullptr, &numberOfDevices );
  if ( findIdentityResult != EB_OK )
  {
    devices.clear();
    throw std::runtime_error( "Etherbone eb_sdb_find_by_identity returned " +
                              std::string( eb_status( findIdentityResult ) ) + "\n" );
  }

  devices.resize( std::max( 0, numberOfDevices ) );
  findIdentityResult = eb_sdb_find_by_identity( device, vendorId, deviceId, &devices[0], &numberOfDevices );

  if ( findIdentityResult != EB_OK )
  {
    devices.clear();
    throw std::runtime_error( "Etherbone eb_sdb_find_by_identity returned " +
                              std::string( eb_status( findIdentityResult ) ) + "\n" );
  }
}
} // namespace

DatamasterEtherboneConnection::DatamasterEtherboneConnection( std::string device )
    : m_socketConnectResult( EB_NULL )
    , m_deviceConnectResult( EB_NULL )
    , m_deviceAddress( std::move( device ) )
{
  m_socketConnectResult = eb_socket_open( EB_ABI_CODE, 0, EB_DATAX | EB_ADDRX, &m_socket );
  if ( m_socketConnectResult != EB_OK )
  {
    throw std::runtime_error( "Etherbone eb_socket_open returned " + std::string( eb_status( m_socketConnectResult ) ) +
                              "\n" );
  }
  m_deviceConnectResult = eb_device_open( m_socket, m_deviceAddress.c_str(), EB_DATAX | EB_ADDRX, 3, &m_device );
  if ( m_deviceConnectResult != EB_OK )
  {
    throw std::runtime_error( "Etherbone eb_device_open returned " + std::string( eb_status( m_deviceConnectResult ) ) +
                              "\n" );
  }

  RetrieveDevices( m_device, SDB_VENDOR_GSI, SDB_DEVICE_LM32_RAM, m_ramDevices );
  RetrieveDevices( m_device, SDB_VENDOR_GSI, SDB_DEVICE_DIAG, m_diagDevices );
  RetrieveDevices( m_device, SDB_VENDOR_GSI, SDB_DEVICE_CLUSTER_TIME_MODULE, m_clusterTimeModules );

  m_metaInformation.numberOfRamDevices         = m_ramDevices.size();
  m_metaInformation.numberOfDiagDevices        = m_diagDevices.size();
  m_metaInformation.numberOfClusterTimeModules = m_clusterTimeModules.size();
}

DatamasterEtherboneConnection::~DatamasterEtherboneConnection()
{
  if ( m_device != EB_NULL )
  {
    eb_device_close( m_device );
  }
  if ( m_socket != EB_NULL )
  {
    eb_socket_close( m_socket );
  }
}

DatamasterMetaInformation DatamasterEtherboneConnection::getMetaInformation() const
{
  return m_metaInformation;
}

std::string DatamasterEtherboneConnection::getBuildInfo() const
{
  std::vector<int32_t> buildInfo( BUILDID_SIZE / 4, 0 );
  eb_cycle_t           cycle;
  eb_status_t          status = EB_OK;
  status                      = eb_cycle_open( getEbDevice(), nullptr, nullptr, &cycle );

  const auto ramDevice      = getRAMDevice( 0 );
  auto       firstAddress   = ramDevice->sdb_component.addr_first;
  auto       buildInfoStart = firstAddress + BUILDID_OFFSET;

  if ( status != EB_OK )
  {
    return "";
  }

  for ( size_t i = 0; i < buildInfo.size(); i++ )
  {
    eb_cycle_read( cycle, buildInfoStart + i * 4, EB_DATA32 | EB_BIG_ENDIAN, (eb_data_t*)( &buildInfo[i] ) );
  }

  status = eb_cycle_close( cycle );

  std::transform( buildInfo.begin(),
                  buildInfo.end(),
                  buildInfo.begin(),
                  []( auto val )
                  {
                    return __builtin_bswap32( val );
                  } );

  if ( status != EB_OK )
  {
    return "";
  }

  return std::string( reinterpret_cast<char*>( &buildInfo[0] ) );
}

bool DatamasterEtherboneConnection::isConnected() const
{
  return m_socketConnectResult == EB_OK && m_deviceConnectResult == EB_OK;
}

const sdb_device* const DatamasterEtherboneConnection::getRAMDevice( uint32_t index ) const
{
  if ( index < m_ramDevices.size() )
  {
    return &m_ramDevices[index];
  }
  return nullptr;
}

const sdb_device* const DatamasterEtherboneConnection::getDiagDevice( uint32_t index ) const
{
  if ( index < m_diagDevices.size() )
  {
    return &m_diagDevices[index];
  }
  return nullptr;
}

const sdb_device* const DatamasterEtherboneConnection::getClusterTimeModule( uint32_t index ) const
{
  if ( index < m_clusterTimeModules.size() )
  {
    return &m_clusterTimeModules[index];
  }
  return nullptr;
}

eb_device_t DatamasterEtherboneConnection::getEbDevice() const
{
  return m_device;
}