import pandas as pd
import time
"""
@page purelypython
"""
"""@package Python example of CppyABM
We define two simple classes of Cell and Tissue as extensions of Agent and Patch, respectively,
to model cellular behavior and the properties of tissue, respectively.

"""
import sys, os,pathlib
from torch.distributions import Categorical

from cppyabm.binds import Env, Agent, Patch, grid2
current_file_path = pathlib.Path(__file__).parent.absolute()
sys.path.insert(1,os.path.join(current_file_path,'..','..'))
from foxyNN.tools import t_env, t_agent
from policies import cell_policy

class Tissue(Patch):
	"""This class extends Patch to simulate tissue properties.
	"""
	def __init__(self,env):
		Patch.__init__(self,env)
		self.damage_center = False # identifies the patch as part of the damaged area
		self.ECM = 100 # quantity of extracellular matrix
@t_agent
class Cell (Agent):
	"""
	This class extends Agent to simulate cells.
	"""
	def __init__(self,env,agent_name):
		Agent.__init__(self,env = env,class_name = agent_name)
		self.clock = 12 # internal clock used to keep the track of proliferation
		self.policy = cell_policy()
	def update(self):
		self.clock+=1
	def step(self):
		"""
		Simulates cellular reactions of migration, proliferation, tissue deposition, and death.
		"""
		neighbor_cell_count = len(self.patch.find_neighbor_agents())
		# migration
		self.order_move(quiet=True)
		# proliferation
		if self.patch.damage_center and self.clock >= 6:
			probs,state_value= self.policy.forward(neighbor_cell_count)
			m = Categorical(probs)
        	action = m.sample()
        	if action == 0:
				self.order_hatch(quiet=True)
				self.clock = 0 
			else:
				pass
			self.save_action(torch.log(probs[action]),state_value)	
		self.reward()
		# ECM synthesize
		# if self.patch.ECM < 100:
		# 	self.patch.ECM += 1	
		# apoptosis
		if neighbor_cell_count >7:
			self.disappear = True
	def reward(self):
		if self.env.clock == 50:
			reward = 100-abs(len(self.env.agents) - 80)
			self.save_reward(reward)
settings = {
    'lr':0.03,
    'episodes':20,
    'policy_t':'AC' # type of policty, e.g. actor critic
}
@t_env(settings=settings)
class Domain(Env):
	"""
	This class extends Env to simulate coordinate the simulation.
	"""
	def __init__(self):
		Env.__init__(self)
		self._repo = []
		self.clock = 0
		self.data = {'cell_count':[]}
	def generate_agent(self,agent_name):
		"""
		Extension of the original function to create agents
		"""
		agent_obj = Cell(self,agent_name)
		self._repo.append(agent_obj)
		self.agents.append(agent_obj)
		return agent_obj
	def generate_patch(self):
		"""
		Extension of the original function to create pacthes
		"""
		patch_obj = Tissue(self)
		self._repo.append(patch_obj)
		return patch_obj
	def damage(self):
		"""
		Create damage
		"""
		for [index,patch] in self.patches.items():
			x,y,z = patch.coords
			if (x >= 0.25 and x <=1.25) and (y>=0.25 and y<=1.25):
				patch.damage_center = True
				patch.ECM = 0
				if patch.empty == False:
					patch.agent.disappear = True
	def setup(self):
		"""
		Setup the simulation by creating mesh, patches, damage, and agents
		"""
		## create mesh
		length = .15 # mm	
		width = length #mm 	
		mesh = grid2(length=length, width=width, mesh_length=0.015, share = True)
		## create patch based on mesh
		self.setup_domain(mesh)
		## assiging 200 cells randomly in the domain
		self.setup_agents({'cell':20})
		## create the damage
		self.damage()
		self.update()
	def step(self):
		"""
		Simulate the progress of the model
		"""
		# execute cells
		for cell in self.agents:
			cell.step()
		self.update()
		self.clock +=1
	def reset(self):
		for agent in self.agents:
			agent.disappear =True
		self.update()
		self.setup_agents({'cell':20})
		self.update()
	def episode(self):
		"""
		Simulate the whole duration of the run
		"""
		self.reset()
		for i in range(50):
			print('Iteration {} cell count {}'.format(i,len(self.agents)))
			self.step()
			# if i%20 ==0:
			# 	self.output()
	def update(self):
		"""
		Update the model. A call to parent function is sent to take care of default functions.
		"""
		super().update()
		for agent in self.agents:
			agent.update()
		cell_count = self.count_agents()
		self.data['cell_count'].append(cell_count['cell'])

	def output(self):
		"""
		Output the results
		"""
		# plot agents on the domain
		file = open('cells.csv','w')
		file.write('x,y,type,size\n')
		for agent in self.agents:
			x,y,z = agent.patch.coords
			file.write("{},{},{},{}\n".format(x, y, agent.class_name, 10))
		file.close()
		#plot ECM density on the domain
		file = open('ECM.csv','w')
		file.write('x,y,type,size\n')
		for [index,patch] in self.patches.items():
			x,y,z = patch.coords
			file.write("{},{},{},{}\n".format(x, y, patch.ECM, 10))
		file.close()
		## cell counts
		df = pd.DataFrame.from_dict(self.data)
		df_agent_counts = df[['cell_count']]
		df_agent_counts.to_csv('cell_count.csv')


