#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main( void )
{
  char     line[512];
  uint64_t td, ev, pa;

  while ( fgets( line, sizeof( line ), stdin ) )
  {
    // Parse using sscanf — only match if all three are found
    if ( sscanf( line, "tDeadline: 0x%" SCNx64 " EvtID: 0x%" SCNx64 " Param: 0x%" SCNx64, &td, &ev, &pa ) == 3 )
    {
      // Write as raw binary (little-endian or native endianness)
      fwrite( &td, sizeof( td ), 1, stdout );
      fwrite( &ev, sizeof( ev ), 1, stdout );
      fwrite( &pa, sizeof( pa ), 1, stdout );
    }
  }
  return 0;
}
