#pragma once

#include <memory>

namespace carpeDM
{
class IDatamaster;
std::unique_ptr<IDatamaster> MakeDatamaster( const char* device );
} // namespace carpeDM