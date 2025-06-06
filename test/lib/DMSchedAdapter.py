from enum import Enum
from test.features.device_config import DUT, CommandResult

from structlog import get_logger
logger = get_logger()

POTENTIAL_ERRORS = [
    "FAILED",
    "Failed",
    "Unknown command",
    "No such file or directory",
    "No such file",
    "No such device"
]


class DMSchedAdapter:
    class CommandFlags(Enum):
        NONE = 0
        FORCE = 1

    @staticmethod
    def __check_command_result(command_result: CommandResult):
        """
        Checks the output of a command for potential errors.
        :param output: The output string to check.
        :raises RuntimeError: If any potential error is found in the output.
        """
        if command_result.exit_code != 0:
            logger.error(f"Command failed with exit code {command_result.exit_code}")
            logger.error(f"Command output: {command_result.output}")
            raise RuntimeError(f"Command failed with exit code {command_result.exit_code}")

        for error in POTENTIAL_ERRORS:
            if error in command_result.output:
                logger.error(f"Command failed with error: {error}")
                raise RuntimeError(f"Command failed with error: {error}")

    def __init__(self, dut: DUT):
        self.dut = dut

    def __get_upload_path(self, name: str) -> str:
        """
        Returns the path where the schedule file will be uploaded on the DUT.
        :param name: The name of the schedule file (without extension).
        :return: The full path to the upload location.
        """
        return f"{self.dut.get_test_data_path()}/schedules/{name}.dot"

    def add_schedule(self, name: str):
        """
        Uploads a schedule file to the DUT and adds it to the datamaster.
        :param name: The name of the schedule file (without extension).
        """
        """
        self.dut.upload_file(
            local_path=os.path.abspath(f"test/features/data/schedules/{name}.dot"),
            remote_path=f"{self.dut.get_test_data_path()}/schedules/"
        )
        """

        output = self.dut.run_command(
            f"dm-sched {self.dut.get_datamaster()} add {self.__get_upload_path(name)}"
        )

        DMSchedAdapter.__check_command_result(output)

    def get_schedule(self, name: str):
        """
        Downloads a schedule file from the DUT.
        :param name: The name of the schedule file (without extension).
        :return: The path to the downloaded schedule file.
        """

        output_file = f"{self.dut.get_test_data_path()}/results/{name}.dot"

        output = self.dut.run_command(
            f"dm-sched {self.dut.get_datamaster()} status -o {output_file}"
        )

        DMSchedAdapter.__check_command_result(output)

        return output_file
    
    def start_pattern(self, pattern_name: str):
        """
        Starts a pattern on the DUT.
        :param pattern_name: The name of the pattern to start.
        :return: The output of the command.
        """
        output = self.dut.run_command(
            f"dm-cmd {self.dut.get_datamaster()} startpattern {pattern_name}"
        )
        DMSchedAdapter.__check_command_result(output)
        return output
    
    def abort_schedules(self):
        """
        Aborts all running schedules on the DUT.
        """
        output = self.dut.run_command(
            f"dm-cmd {self.dut.get_datamaster()} abort"
        )
        DMSchedAdapter.__check_command_result(output)
    
    def clear_schedules(self, *, flags: CommandFlags = CommandFlags.NONE):
        command = f"dm-sched {self.dut.get_datamaster()} clear"
        if flags == DMSchedAdapter.CommandFlags.FORCE:
            command += " -f"
        output = self.dut.run_command(command)
        DMSchedAdapter.__check_command_result(output)

    def keep_only_schedule(self, schedule_name: str):
        """
        Keeps only the specified schedule on the DUT, removing all others.
        :param schedule_name: The name of the schedule to keep.
        """
        output = self.dut.run_command(
            f"dm-sched {self.dut.get_datamaster()} keep {self.__get_upload_path(schedule_name)}"
        )
        DMSchedAdapter.__check_command_result(output)