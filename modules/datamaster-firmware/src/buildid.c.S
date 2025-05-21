#define BUILDID __attribute__((section(".buildid")))
const char BUILDID build_id_rom[] = "\
UserLM32\n\
Stack Status:                                                       \n\
Project     : @TARGET@\n\
Version     : @CMAKE_PROJECT_VERSION@\n\
Platform    : @CBR_PF@\n\
Build Date  : @CBR_DATE@\n\
Prepared by : @USER@ @CBR_USR@ <@CBR_MAIL@>\n\
Prepared on : @CBR_HOST@\n\
OS Version  : @CBR_OS@ @CBR_KRNL@\n\
GCC Version : @CBR_GCC@\n\
IntAdrOffs  : @CONFIG_RAMADDR@\n\
SharedOffs  : @SHARED_START@\n\
SharedSize  : @SHARED_SIZE@\n\
ThreadQty   : @THR_QTY@\n\
FW-ID ROM will contain:\n\n\
@CBR_GIT1@\n\
@CBR_GIT2@\n\
@CBR_GIT3@\n\
@CBR_GIT4@\n\
@CBR_GIT5@\n\
From branch: @GIT_BRANCH@\n\
";