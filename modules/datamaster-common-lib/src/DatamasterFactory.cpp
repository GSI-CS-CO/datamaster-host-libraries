#include "DatamasterFactory.h"
#include "IDatamaster.h"

namespace carpeDM
{
std::unique_ptr<IDatamaster> MakeDatamaster( const char* device )
{
  return nullptr;
}
} // namespace carpeDM