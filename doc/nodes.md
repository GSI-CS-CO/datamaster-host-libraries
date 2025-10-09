# New Node Types
## "Variable" Node
A variable node is meant to be used to: 
* Prepare memory for the values of the given variable
* Update a variable
* Initialize a variable
* Offer an interface to set values for other nodes

A variable can either be private or public.
When it is public, the value can be set via the ECA or host and will be updated as sooon as the scheduler reaches
a corresponding node.

## "Branch" Node
A joint node is meant to be used to branch the task graph.
This will be usually done by evaluating a function or variable.
A function is a mathematical equation defined in a simple way, which is able to use variables.

## "Execute Task" Node
A function node allows to switch to another subgraph and execute it.
This may be done synchronously or asynchronously.
The timestamp (and potentially more) of the function result can be stored in a variable on the caller side.
This will allow to call and wait for multiple tasks in parallel.

## "Send Timing Message" Node
A timing message node is meant to schedule the sending of a timing telegram over the timing network.

## "Increment Time" Node
A node that defines time frames is needed and for this reason we define the "increment time"-node (former Block node). All following messages will belong to this block.
If there is no time block, the initial time offset is 0 and the following time block defines the frame with its time offset.

e.g.: 

Timing Message A -> Timing Message B -> Block A [toffs="200000"] is a valid graph and describes the duration of the graph by Block A.

The block node (and potentially also the others) are able to define "dependencies".
The dependencies will be evaluated by the scheduler and the executaion of the graph will be paused until the dependencies are met.
For this the block containing dependencies will be put on a wait list and rescheduled on update of the dependant values.

## Other command nodes:
Potential other command nodes are 