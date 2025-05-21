#ifndef CONFIG_TARGET_SHARED_MMAP_H
#define CONFIG_TARGET_SHARED_MMAP_H
//Location of Buildid and Shared Section in LM32 Memory, to be used by host

#cmakedefine INT_BASE_ADR		@CONFIG_RAMADDR@
#cmakedefine RAM_SIZE		@CONFIG_RAMSIZE@
#cmakedefine SHARED_SIZE		@CONFIG_SHAREDSIZE@
#cmakedefine BUILDID_OFFS		@CONFIG_BOOTSIZE@
#cmakedefine SHARED_OFFS		(@CONFIG_BOOTSIZE@ + @CONFIG_BUILDIDSIZE@)
#endif
