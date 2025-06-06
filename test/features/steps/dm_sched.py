from behave import given, then, when # type: ignore
from test.lib.TestContext import TestContext
from test.lib.DMSchedAdapter import DMSchedAdapter
from test.lib.scheduleCompareAdapter import scheduleCompareAdapter, CompareFlags

@given(u'we add schedules')
def step_add_schedule(context: TestContext):
    if context.table is None:
        raise ValueError("No schedules provided in the table.")
    if not hasattr(context, 'dut') or context.dut is None:
        raise ValueError("Device Under Test (DUT) is not set in the context.")
    dm_sched_adapter = DMSchedAdapter(context.dut)

    for schedule in context.table:
        name = schedule['name']
        dm_sched_adapter.add_schedule(name)

@then(u'the device should have the schedule {schedule_name}')
def step_verify_schedule(context, schedule_name):
    if not hasattr(context, 'dut') or context.dut is None:
        raise ValueError("Device Under Test (DUT) is not set in the context.")

    dm_sched_adapter = DMSchedAdapter(context.dut)
    result_file = dm_sched_adapter.get_schedule(schedule_name)
    comparator = scheduleCompareAdapter(context.dut)
    comparator.compare(
        result_file,
        f"/tmp/test/schedules/{schedule_name}.dot",
        flags=CompareFlags.UNDEFINED_AS_EMPTY
    )

@when(u'we run the schedule "{pattern_name}"')
def step_impl(context, pattern_name):
    if not hasattr(context, 'dut') or context.dut is None:
        raise ValueError("Device Under Test (DUT) is not set in the context.")
    dm_sched_adapter = DMSchedAdapter(context.dut)
    downloaded_file = dm_sched_adapter.start_pattern(pattern_name)

@when(u'we only keep the schedule {schedule_name}')
def step_keep_single_edge_schedule(context: TestContext, schedule_name: str):
    if not hasattr(context, 'dut') or context.dut is None:
        raise ValueError("Device Under Test (DUT) is not set in the context.")
    dm_sched_adapter = DMSchedAdapter(context.dut)
    dm_sched_adapter.keep_only_schedule(schedule_name)