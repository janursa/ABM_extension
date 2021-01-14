#include <iostream>
#include <fstream>
#include "CppyABM/include/ABM/bases.h"
#include "CppyABM/include/ABM/mesh.h"
struct Domain;
struct Tissue;
struct Cell;
//! Domain class to coordinate the simulation
struct Domain: public Env<Domain,Cell,Tissue> {
	Domain():Env<Domain,Cell,Tissue>(){
	}
	virtual shared_ptr<Cell> generate_agent(std::string agent_name);
	virtual shared_ptr<Tissue> generate_patch();
	virtual void update();
	void damage();
	void setup();
	void step();
	void episode();
	void output();
	std::map<std::string,std::vector<int>> data= {{"cell_count",{}}};
	int tick=0;
};
struct Tissue: public Patch<Domain,Cell,Tissue> {
	Tissue(shared_ptr<Domain> env):Patch<Domain,Cell,Tissue>(env){
		this->setup();
	}
	void setup(){
	}
	bool damage_center = false;
	double ECM = 100;

};
struct Cell: public Agent<Domain,Cell,Tissue> {
	Cell(shared_ptr<Domain> env,std::string agent_name):Agent<Domain,Cell,Tissue>(env,agent_name){
		this->setup();
	}
	void update(){
		this->clock++;
	}
	virtual void step();
	virtual void setup(){
		this->cycle_t = 12;
	}
	int cycle_t;
	int clock = 0;
};
inline shared_ptr<Cell> Domain::generate_agent(std::string agent_name){
		auto agent_obj = make_shared<Cell>(this->shared_from_this(),agent_name);
		this->agents.push_back(agent_obj);
		return agent_obj;
	}
inline shared_ptr<Tissue> Domain::generate_patch(){
		auto patch_obj = make_shared<Tissue>(this->shared_from_this());
		return patch_obj;
	}
inline void Domain::setup(){
	auto mesh = space::grid2(1.5, 1.5, 0.015, true);
	this->setup_domain(mesh);
	std::map<std::string,unsigned> settings = {{"cell",2000}};
	this->setup_agents(settings);
	this->damage();
	this->update();
	}
inline void Domain::damage(){
		for (auto &[index,patch]:this->patches){
			auto x = patch->coords[0];
			auto y = patch->coords[1];
			if ((x >= 0.25 and x <=1.25) and (y>=0.25 and y<=1.25)){
				patch->damage_center = true;
				patch->ECM = 0;
				if (patch->empty() == false){
					patch->get_agent()->disappear = true;
				}
					
			}
		}
	}
inline void Domain::update(){
		Env<Domain,Cell,Tissue>::update();
		for (auto &agent: this->agents){
			agent->update();
		}
		int cell_count = this->agents.size();
		this->data["cell_count"].push_back(cell_count);		

	}
inline void Domain::step(){
		for (auto &cell:this->agents){
			cell->step();
		}
		this->update();
		this->tick ++;
	}	
inline void Cell::step(){
	auto dest_patch = this->patch->empty_neighbor(true);
	this->order_move(nullptr,true,false);
	auto neighbor_cell_count = this->patch->find_neighbor_agents().size();
	if (this->patch->damage_center and this->clock >= this->cycle_t){
		if (neighbor_cell_count <= 6){
			this->order_hatch(nullptr,false,true,false);
			this->clock = 0 ;
		}
	}
	if (this->patch->ECM < 100) {
		this->patch->ECM += 1;	
	}
		
	if (neighbor_cell_count >7){
		this->disappear = true;
	}
}
inline void Domain::episode(){
	for (unsigned i = 0; i < 336; i++){
			cout<<"iteration "<<i<<" agents "<<this->agents.size()<<endl;
			this->step();
			if (i%20 == 0) this->output();
		}	
}
inline void Domain::output(){
	// plot agents on the domain
	ofstream file1;
	file1.open("cells.csv");
	file1 << "x,y,type,size\n";
	for (auto &agent:this->agents){
		file1<<agent->patch->coords[0]<<","<<agent->patch->coords[1]<<","<< agent->class_name<<", "<<10<<std::endl;
	}
	file1.close();
	//plot ECM density on the domain
	ofstream file2;
	file2.open("ECM.csv");
	file2<<"x,y,type,size\n";
	for (auto &[index,patch]:this->patches){
		file2<<patch->coords[0]<<","<<patch->coords[1]<<","<< patch->ECM<<", "<<10<<std::endl;
	}
	file2.close();
	// cell counts
	ofstream file3;
	file3.open("cell_count.csv");
	file3<<"cell_count\n";
	for (unsigned i=0;i<this->data["cell_count"].size(); i++){
		file3<<i<<","<<this->data["cell_count"][i]<<std::endl;
	}
	file3.close();

}
