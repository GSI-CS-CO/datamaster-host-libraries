
class RootGraph():
    def __init__(self, name):
        self.name = name
    
    def __str__(self) -> str:
        return f"digraph {self.name} {{}}"
