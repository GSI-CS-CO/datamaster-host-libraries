from behave import runner
from test.features.device_config import DUT
import pydot

class TestContext(runner.Context):
    """
    Custom context for Behave tests.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.dut : (DUT|None) = None  # Device Under Test
        self.graphs : dict[str, pydot.Dot] = {}  # Store graphs by name

    def add_graph(self, name: str, graph: pydot.Dot):
        """
        Add a graph to the context.
        :param name: The name of the graph.
        :param graph: The pydot graph object.
        """
        if not isinstance(graph, pydot.Dot):
            raise TypeError("graph must be an instance of pydot.Dot")
        
        if name in self.graphs:
            raise ValueError(f"Graph with name '{name}' already exists in the context.")

        self.graphs[name] = graph