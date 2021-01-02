## Inheritage-based develomment
The development method in CppyABM is based on inheritage. Three based classes of Env, Agent, and Patch are available for extension. Agent and Patch require an input argument of Env for initialization. Also, Agent requires an identity variable termed \ref Agent::class_name to enable multi-agent modeling. There are few conventions that need further explanation to define an environment. Since Python requires object ownership, any object shared between C++ and Python ends, i.e. Patch and Agent objects, needs to be stored in Python end and referenced as a pointer in C++ end. To satisfy this requirement, two template functions of \ref Env::generate_agent and \ref Env::generate_patch are provided to manage the generation and storage of agents and patches. These functions need to be customized for each model. A repository variable needs to be defined in Python end to store created objects. Simultaneously, the objects need to be added to Env::agents and Env::patches which are the standard containers. The examples is given in \ref Domain. Using these functions, the defined agents and patches can be accessed in both C++ and Python without further measure.
## Bind tools from Cpp to Python
In order to bind or extend a type or function from Cpp to Python, a series of tools are defined 
## Usefull build-in functions
## Examples
Find the examples [here](examples.md)