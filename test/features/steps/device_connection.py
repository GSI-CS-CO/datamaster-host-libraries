from behave import given # type: ignore
from test.lib.TestContext import TestContext


@given('we have a device connection established')
def step_impl(context: TestContext):
    assert context.dut != None, "Device Under Test (DUT) is not set in the context."