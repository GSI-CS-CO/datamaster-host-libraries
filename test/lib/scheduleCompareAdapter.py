from pexpect import spawn, EOF
from typing import IO
from test.features.device_config import DUT

import logging
logger = logging.getLogger(__name__)

# enum for compare flags
class CompareFlags:
    NONE = 0
    VERBOSE = 1
    IGNORE_NAMES = 2
    CHECK = 4
    UNDEFINED_AS_EMPTY = 8
    TEST = 16


class scheduleCompareAdapter:
    def __init__(self, dut: DUT):
        """
        Initialize the scheduleCompareAdapter.
        This adapter is used to compare two schedules using an external binary.
        """
        self.__dut : DUT = dut

    def compare(self, schedule1, schedule2, flags: CompareFlags = CompareFlags.NONE) -> bool:
        """
        Compare two schedules and return a boolean indicating if they are the same.
        """

        logger.info(f"Comparing schedules: {schedule1} and {schedule2}")

        args = []
        if flags & CompareFlags.VERBOSE:
            args.append("-v")
        else:
            args.append("-s")

        if flags & CompareFlags.IGNORE_NAMES:
            args.append("-n")
        
        if flags & CompareFlags.CHECK:
            args.append("-c")
        
        if flags & CompareFlags.UNDEFINED_AS_EMPTY:
            args.append("-u")

        if flags & CompareFlags.TEST:
            args.append("-t")

        result = self.__dut.run_command("/tmp/test/scheduleCompare " + " ".join(args) + " " + schedule1 + " " + schedule2)

        logger.info(f"Comparison result: {result}")