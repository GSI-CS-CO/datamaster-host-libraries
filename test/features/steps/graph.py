from behave import given, then, when  # type: ignore
import pydot
from test.lib.TestContext import TestContext

@given(u'we have a graph called "{graph_name}"')
def step_create_graph(context: TestContext, graph_name):
    new_graph = pydot.Dot(graph_name, graph_type='digraph')
    context.add_graph(graph_name, new_graph)