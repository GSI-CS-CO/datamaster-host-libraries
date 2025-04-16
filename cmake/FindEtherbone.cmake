
set(Etherbone_VERSION ${PC_Etherbone_VERSION})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Etherbone
  REQUIRED_VARS
    Etherbone_LIBRARY
    Etherbone_INCLUDE_DIR
  VERSION_VAR Etherbone_VERSION
)