from device_config import get_dut
import datetime

from test.features.steps.dm_sched import DMSchedAdapter

import structlog
log = structlog.get_logger()

import os

from labgrid import Environment

os.environ["LG_COORDINATOR"] = "tcp://192.168.1.102:20408"

def before_all(context):
    import os
    log.info(f"Starting tests at {datetime.datetime.now()}")
    log.info(os.getcwd())

    env = Environment(f"./test/environment.yaml") # type: ignore
    #target = env.get_target("datamaster")
    #if target is None:
    #    raise RuntimeError("No target found with role 'datamaster' in the environment configuration.")
    #log.info(f"Using target: {target}")
    #target.get_resource("NetworkDriver")


    dut_host = os.getenv('DUT_HOST', '192.168.1.101')
    dut_device = os.getenv('DUT_DEVICE', 'dev/wbm0')
    context.dut = get_dut(dut_host, dut_device)

    log.info(f"Using DUT: {context.dut}")
    log.info(f"Uploading test data to {context.dut.get_test_data_path()}")
    context.dut.upload_file(
        local_path=os.path.abspath(f"test/features/data/schedules/*"),
        remote_path=f"{context.dut.get_test_data_path()}/schedules/"
    )

    log.info(f"Uploading binary files to {context.dut.get_test_data_path()}")
    context.dut.upload_file(
        local_path=os.path.abspath(f"out/*"),
        remote_path=f"{context.dut.get_test_data_path()}/"
    )

    log.info(f"Clearing existing schedules on the DUT")
    adapter = DMSchedAdapter(
        dut=context.dut
    )
    adapter.clear_schedules(flags=DMSchedAdapter.CommandFlags.FORCE)
    

def after_scenario(context, step):
    DMSchedAdapter(
        dut=context.dut
    ).clear_schedules(flags=DMSchedAdapter.CommandFlags.FORCE)