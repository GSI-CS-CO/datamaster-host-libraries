#include "log.h"

extern "C" {
DATAMASTER_COMMON_EXPORT log_level_t GLOBAL_LEVEL = ERROR;

DATAMASTER_COMMON_EXPORT const char* const log_lvl_str[] = {
	"NOTHING:",
    "CRITICAL:",
    "ERROR:",
    "WARNING:",
    "INFO:",
    "VERBOSE:",
    "DEBUG LVL 0:",
    "DEBUG LVL 1:",
    "DEBUG LVL 2:",
    "DEBUG LVL 3:",
};
}