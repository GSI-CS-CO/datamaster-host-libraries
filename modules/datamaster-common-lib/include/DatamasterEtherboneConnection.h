#include "IDatamasterEtherboneConnection.h"

#include <etherbone.h>

#include <string>
#include <vector>

namespace carpeDM
{
class DatamasterEtherboneConnection : public IDatamasterEtherboneConnection
{
public:
  /**
   * Connect to a Datamaster device via the given socket address.
   * @param device The device to connect to e.g. dev/ttyUSB0
   * @throws std::runtime_error on connection errors
   */
  DatamasterEtherboneConnection( std::string device );

  virtual ~DatamasterEtherboneConnection();

  DatamasterMetaInformation getMetaInformation() const override;
  std::string               getBuildInfo() const override;
  bool                      isConnected() const override;

  const sdb_device* const getRAMDevice( uint32_t index ) const override;
  const sdb_device* const getDiagDevice( uint32_t index ) const override;
  const sdb_device* const getClusterTimeModule( uint32_t index ) const override;

  virtual eb_device_t getEbDevice() const override;

private:
  eb_status_t m_socketConnectResult;
  eb_status_t m_deviceConnectResult;
  std::string m_deviceAddress;

  std::vector<sdb_device> m_ramDevices;
  std::vector<sdb_device> m_diagDevices;
  std::vector<sdb_device> m_clusterTimeModules;

  eb_socket_t m_socket = EB_NULL;
  eb_device_t m_device = EB_NULL;

  DatamasterMetaInformation m_metaInformation;
};
} // namespace carpeDM