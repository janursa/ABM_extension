#include <iostream>
#include "CPPYABM/include/ABM/bases.h"
#include "CPPYABM/include/ABM/mesh.h"
struct healingEnv;
struct myPatch;
struct Cell;

struct healingEnv: public baseEnv<healingEnv,Cell,myPatch> {
	healingEnv():baseEnv<healingEnv,Cell,myPatch>(){}
	virtual shared_ptr<Cell> generate_agent(std::string agent_name);
	virtual shared_ptr<myPatch> generate_patch();
	virtual void update();
	void damage();
	void setup();
	void run();
	unsigned clock=0;
};
struct myPatch: public basePatch<healingEnv,Cell,myPatch> {
	myPatch(shared_ptr<healingEnv> env):basePatch<healingEnv,Cell,myPatch>(env){}
	bool damage_center = false;
	double tissue = 100;
};
struct Cell: public baseAgent<healingEnv,Cell,myPatch> {
	Cell(shared_ptr<healingEnv> env,std::string agent_name):baseAgent<healingEnv,Cell,myPatch>(env,agent_name){
	}
	void update(){
		this->clock++;
	}
	void step();
	unsigned clock = 12;
};