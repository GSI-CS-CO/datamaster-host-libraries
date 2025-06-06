import pexpect
from abc import ABC, abstractmethod
from dataclasses import dataclass

from pexpect import pxssh
from typing import IO,Tuple

from structlog import get_logger
logger = get_logger()

@dataclass
class CommandResult:
    output: str
    exit_code: int

    def __str__(self):
        return f"CommandResult(output={self.output}, exit_code={self.exit_code})"

    def __repr__(self):
        return self.__str__()

def get_dut(host, device):
    """
    Returns a pexpect spawn object for the device under test (DUT).

    :param host: The host address of the DUT.
    :param device: The device path for the DUT.
    :return: A pexpect spawn object connected to the DUT.
    """

    if host == 'localhost':
        # Use LocalDut for local devices
        assert False, "LocalDut is not implemented. Please use SSHDut for remote devices."
    else:
        return SSHDut(host, device)

class DUT(ABC):
    @abstractmethod
    def upload_file(self, *, local_path: str, remote_path: str):
        """
        Uploads a file to the DUT.
        :param local_path: The path to the file on the local machine.
        :param remote_path: The path on the DUT where the file should be uploaded.
        """
        pass

    @abstractmethod
    def download_file(self, *, remote_path: str, local_path: str):
        """
        Downloads a file from the DUT.

        :param remote_path: The path on the DUT to the file.
        :param local_path: The path where the file should be saved locally.
        """
        pass

    @abstractmethod
    def get_datamaster(self) -> str:
        """
        Returns the path to the datamaster on the DUT.
        
        :return: The path to the datamaster.
        """
        pass

    @abstractmethod
    def run_command(self, command: str) -> CommandResult:
        """
        Runs a command on the DUT and returns the output.

        :param command: The command to run on the DUT.
        :return: The output of the command.
        """
        pass

    @abstractmethod
    def get_test_data_path(self) -> str:
        """
        Returns the path to the test data directory on the DUT.

        :return: The path to the test data directory.
        """
        pass

class SSHDut(DUT):
    def __init__(self, host, device, *, base_path='/tmp/test'):
        self.host = host
        self.device = device
        self.base_path = base_path
        self.__connect()

        # ensure the base path exists on the DUT
        self.connection.sendline(f'mkdir -p {self.base_path}')
        self.connection.prompt()
        self.connection.sendline(f'rm -rf {self.base_path}/*')
        self.connection.prompt()
        self.connection.sendline(f'mkdir -p {self.base_path}/schedules')
        self.connection.prompt()
        self.connection.sendline(f'mkdir -p {self.base_path}/results')
        self.connection.prompt()

    def __connect(self):
        self.connection = pxssh.pxssh(timeout=2, encoding='utf-8')
        self.connection.login(self.host, 'root', 'geheim')
        self.connection.sendline("uptime")
        self.connection.prompt()

    def upload_file(self, *, local_path: str, remote_path: str):
        logger.info(f"Uploading {local_path} to {remote_path}")
        scp_command = f'scp -O {local_path} root@{self.host}:{remote_path} && echo "SUCCESS"'
        logger.info(f"Running SCP command: {scp_command}")

        console = pexpect.spawn("bash", encoding='utf-8', timeout=2)
        console.sendline(scp_command)
        console.expect("SUCCESS")
        i = console.expect(["password:", pexpect.EOF, "SUCCESS"], timeout=10)
        if i == 0:
            console.sendline('geheim')
        elif i == 1:
            raise Exception("Failed to upload file: SCP command did not complete successfully.")
        elif i == 2:
            logger.info("File upload completed successfully.")

    def download_file(self, *, remote_path: str, local_path: str):
        scp_command = f'scp -O root@{self.host}:{remote_path} {local_path} && echo "SUCCESS"'
        logger.info(f"Downloading {remote_path} to {local_path}")
        logger.info(f"Running SCP command: {scp_command}")

        console = pexpect.spawn("bash", encoding='utf-8', timeout=2)
        console.sendline(scp_command)
        console.expect("SUCCESS")

        i = console.expect(["password:", pexpect.EOF, "SUCCESS"], timeout=10)
        if i == 0:
            console.sendline('geheim')
        elif i == 1:
            raise Exception("Failed to download file: SCP command did not complete successfully.")
        elif i == 2:
            logger.info("File download completed successfully.")
        assert console.expect(pexpect.EOF) == 0, "SCP command did not complete successfully."

    def run_command(self, command: str) -> CommandResult:
        logger.info(f"Running command on DUT: {command}")
        self.connection.sendline(command)
        # ensure the command is executed successfully
        self.connection.prompt()
        output: str = self.connection.before # type: ignore
        logger.info(f"Command output: {output.strip()}")
        self.connection.sendline("echo $?")
        self.connection.prompt()

        exit_code: str = self.connection.before.replace("echo $?", "").strip()  # type: ignore
        return CommandResult(output=output.strip(), exit_code=int(exit_code.strip()))

    def get_datamaster(self) -> str:
        return f"{self.device}"
    
    def get_test_data_path(self) -> str:
        return f"{self.base_path}"

