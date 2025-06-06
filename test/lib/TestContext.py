from behave import runner
from test.features.device_config import DUT

class TestContext(runner.Context):
    """
    Custom context for Behave tests.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.dut : (DUT|None) = None  # Device Under Test