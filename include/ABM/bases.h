
#pragma once
#include <map>
#include <algorithm>
#include <random>
// #include <nlohmann/json.hpp>
// using json = nlohmann::json
#include "common.h"
#include "mesh.h"
#include "tools.h"
using std::shared_ptr;
using std::vector;
struct baseEnv;
struct Agent;
struct Patch;
using namespace std;
//!  Base class
/*!
  All classes inherit this
*/
struct Base{
	string class_name;
    virtual void set_data(string tag, double value) {throw undefined_member("Set data is used before implementation");};
    virtual double get_data(string tag) {throw undefined_member("Get data is used before implementation");};

};
//!   Base clase for patches
/*!
  
*/
struct Patch: public Base{
	Patch(shared_ptr<baseEnv> env){
		this->env = env;
	}
	virtual ~Patch(){}
	unsigned index;
	unsigned layer_index;
	vector<double> coords;
	vector<unsigned> neighbors_indices;
	vector<shared_ptr<Patch>> neighbors;
	/** main funcs **/
	virtual void step(){
		throw undefined_member("Step function is not defined inside Patch");
	};
	/** Auxs funcs **/
	void set_agent(shared_ptr<Agent> agent){
		this->agent = agent;
		this->empty = false;
		this->agent_count ++;
		if (this->agent_count>1){
			cout<<"Patch holds more than an agent"<<endl;
		}
	}
	void remove_agent(){
		this->agent = nullptr;
		this->empty = true;
		this->agent_count = 0;
	}
	shared_ptr<Patch> empty_neighbor(bool quiet = false);
	vector<shared_ptr<Agent>> find_neighbor_agents(bool include_self = true){
		vector<shared_ptr<Agent>> neighbor_agents;
		if (!this->empty & include_self) neighbor_agents.push_back(this->agent);
		for (auto const &patch:this->neighbors){
			if (!patch->empty) neighbor_agents.push_back(patch->agent);
		}
		return neighbor_agents;
	}
	/** connectors **/
	unsigned agent_count = 0; //!< For debugging 
	std::shared_ptr<Agent> agent;
	std::shared_ptr<baseEnv> env;
	/** patch data **/
	bool empty = true;
	bool on_border = false;

};

//!   Base clase for Agents
/*!
  
*/
struct Agent: public Base,enable_shared_from_this<Agent>{
	Agent(shared_ptr<baseEnv> env , string class_name){
		this->env = env;
		this->class_name = class_name;
	}
	virtual ~Agent(){};
	/** Major functions **/ 
	virtual void step(){
		throw undefined_member("Agent step function is not defined");
	};
    /** Flags **/
    std::pair <bool,std::string> switch_info = std::make_pair(false,"");
    HATCH_CONFIG _hatch = HATCH_CONFIG();
    MOVE_CONFIG _move = MOVE_CONFIG();
    SWITCH_CONFIG _switch = SWITCH_CONFIG();
    /** pure virtuals **/

    /** Auxillary funcs **/
    virtual void inherit(shared_ptr<Agent> father){cout<<"Inherit is not defined"<<endl;};
    virtual void update(){};
    void set_patch(shared_ptr<Patch> patch){ this->patch = patch;}
    void move(shared_ptr<Patch> dest, bool quiet = false);
    std::shared_ptr<Agent> get_ptr();
    void order_hatch(shared_ptr<Patch> patch = nullptr, 
            bool inherit = false, bool quiet = false, bool reset = false);
    void order_move(shared_ptr<Patch> patch, bool quiet, bool reset);
    void order_switch(string to){
    	this->_switch = SWITCH_CONFIG(true,to);
    };

    void reset_hatch(){ this->_hatch = HATCH_CONFIG();}
    void reset_move(){ this->_move = MOVE_CONFIG();}
    void reset_switch(){ this->_switch = SWITCH_CONFIG();}

    /** connectors **/
	std::shared_ptr<Patch> patch;
	std::shared_ptr<baseEnv> env;

	bool disappear = false;


};


//!   Base clase for environments
/*!
  
*/
struct baseEnv: public Base,enable_shared_from_this<baseEnv>{
	virtual ~baseEnv(){};
    AgentsBank agents;
    PatchesBank patches;
    vector<unsigned> patches_indices;
    //** Pure virtuals **/
    // template<class PATCH>
    virtual shared_ptr<Patch> generate_patch() {
    	throw undefined_member("Generate patch is not defined inside Env");
    };
	virtual	shared_ptr<Agent> generate_agent(string class_name) {
		throw undefined_member("Generate agent is not defined inside Env");
	};
	// virtual void update_repo(){throw undefined_member("Update repository is not implemented but called");};
	virtual void update_repo(){}
	//** main functions **/
    void setup_domain(vector<MESH_ITEM> mesh);
    void step_agents();
    void step_patches();
    
	virtual void update();
    //** Place agents **/
    virtual void setup_agents(map<string,unsigned> config);
    void place_agent(shared_ptr<Patch> patch,shared_ptr<Agent> agent);
    void place_agent_randomly(shared_ptr<Agent> agent);
    shared_ptr<Patch> find_empty_patch(); //!< Finds empty patches in the entire domain
    void connect_patch_agent(shared_ptr<Patch> patch,shared_ptr<Agent> agent);

    virtual void step() {
    	throw undefined_member("Step function is not defined inside Env");
    };
    //** aux functions **//
    std::shared_ptr<baseEnv> get_ptr(){
    	return this->shared_from_this();
    };
    map<string,unsigned> count_agents();

    /** Env data **/
    std::map<std::string,unsigned> agents_count;
    std::set<string> agent_classes;

};
template<class AGENT,class PATCH>
struct Env: public baseEnv,enable_shared_from_this<Env<AGENT,PATCH>>{
	virtual ~baseEnv(){};
    vector<shared_ptr<AGENT>> agents;
    map<unsigned,shared_ptr<PATCH>> patches;
    vector<unsigned> patches_indices;
    //** Pure virtuals **/
    virtual shared_ptr<PATCH> generate_patch() {
    	throw undefined_member("Generate patch is not defined inside Env");
    };
	virtual	shared_ptr<AGENT> generate_agent(string class_name) {
		throw undefined_member("Generate agent is not defined inside Env");
	};    
    //** Place agents **/
    void place_agent(shared_ptr<Patch> patch,shared_ptr<Agent> agent);
    void place_agent_randomly(shared_ptr<Agent> agent);
    shared_ptr<Patch> find_empty_patch(); //!< Finds empty patches in the entire domain
    void connect_patch_agent(shared_ptr<Patch> patch,shared_ptr<Agent> agent);
    
    // //** aux functions **//
    std::shared_ptr<baseEnv> get_ptr(){
    	return this->shared_from_this();
    };

};
inline void Agent::move(shared_ptr<Patch> dest, bool quiet){
        if (!dest->empty) {
            if (!quiet) throw patch_availibility("Given patch for move is not empty");
            else return;
        }
        this->patch->remove_agent(); // remove it from the current patch
        this->env->connect_patch_agent(dest,this->get_ptr());
    }
inline void Agent::order_hatch(shared_ptr<Patch> patch, bool inherit, bool quiet, bool reset)
    {
    this->_hatch =  HATCH_CONFIG(true,patch,inherit,quiet,reset);

}
inline void Agent::order_move(shared_ptr<Patch> patch = nullptr, 
             bool quiet = false, bool reset = false){
    this->_move =  MOVE_CONFIG(true,patch,quiet,reset);

}

inline std::shared_ptr<Agent> Agent::get_ptr(){
    return this->shared_from_this();

}

inline void baseEnv::step_agents(){
    for (unsigned i = 0; i < this->agents.size(); i++){
        this->agents[i]->step();
    }
}
inline void baseEnv::step_patches(){

    for (unsigned i = 0; i < this->patches.size(); i++){
        this->patches[i]->step();
    }
}
inline void baseEnv::setup_agents(map<string,unsigned> config){
    for (auto const [agent_type,count]:config){
        for (unsigned i = 0; i < count; i++){
            auto agent = this->generate_agent(agent_type);
            this->place_agent_randomly(agent);
        }
        this->agent_classes.insert(agent_type);
    }
}
inline void baseEnv::update(){
    auto g = random_::randomly_seeded_MT();
    std::shuffle(this->agents.begin(),this->agents.end(),g);
    /** hatch **/
    unsigned agent_count = this->agents.size();
    for (unsigned  i = 0; i < agent_count; i++){
        if (this->agents[i]->_hatch._flag){
            auto inherit = this->agents[i]->_hatch._inherit;
            auto patch = this->agents[i]->_hatch._patch;

            if (patch == nullptr){ // find a random patch
                try{
                    patch = this->agents[i]->patch->empty_neighbor();
                }
                // failure: manage how to respond
                catch(patch_availibility&ee){
                    
                    if (!this->agents[i]->_hatch._quiet){ // only throw when it's forced
                        if (this->agents[i]->_hatch._reset){ // the try failed so reset the order
                            this->agents[i]->reset_hatch();
                        }
                        throw ee;
                    } 
                    else{ // go to the next agent
                        if (this->agents[i]->_hatch._reset){ // the try failed so reset the order
                            this->agents[i]->reset_hatch();
                        }
                        continue;
                    }
                }
                
            }
            // check if another agent hasn't already taken it 
            if (!patch->empty){ 
                // failure: manage how to respond
                
                if (this->agents[i]->_hatch._quiet){ // only throw when it's forced
                    if (this->agents[i]->_hatch._reset){ // the try failed so reset the order
                        this->agents[i]->reset_hatch();
                    }   
                    continue;
                }
                else{
                    if (this->agents[i]->_hatch._reset){ // the try failed so reset the order
                        this->agents[i]->reset_hatch();
                    }
                    throw patch_availibility("No patch for hatching. If you want to silent this error, pass argumen quiet as true");
                }
            }
            auto new_agent = this->generate_agent(this->agents[i]->class_name);
            if (this->agents[i]->_hatch._inherit){
                new_agent->inherit(this->agents[i]);
            }
            this->connect_patch_agent(patch,new_agent);
            this->agents[i]->reset_hatch();
        };
    }

    /** move **/
    for (unsigned  i = 0; i < agent_count; i++){
        if (!this->agents[i]->_move._flag) continue;
        auto patch = this->agents[i]->_move._patch;
        if (patch == nullptr){ // find a random patch
                try{
                    patch = this->agents[i]->patch->empty_neighbor();
                }
                // failure: manage how to respond
                catch(patch_availibility&ee){
                    
                    if (!this->agents[i]->_move._quiet){ // only throw when it's forced
                        if (this->agents[i]->_move._reset){ // the try failed so reset the order
                            this->agents[i]->reset_move();
                        }
                        throw ee;
                    } 
                    else{ // go to the next agent
                        if (this->agents[i]->_move._reset){ // the try failed so reset the order
                            this->agents[i]->reset_move();
                        }
                        continue;
                    }
                }
                
            }
            // check if another agent hasn't already taken it 
            if (!patch->empty){ 
                // failure: manage how to respond
                
                if (this->agents[i]->_move._quiet){ // only throw when it's forced
                    if (this->agents[i]->_move._reset){ // the try failed so reset the order
                        this->agents[i]->reset_move();
                    }   
                    continue;
                }
                else{
                    if (this->agents[i]->_move._reset){ // the try failed so reset the order
                        this->agents[i]->reset_move();
                    }
                    throw patch_availibility("No patch for moving. If you want to silent this error, pass argumen quiet as true");
                }
            }

            this->agents[i]->move(patch,this->agents[i]->_move._quiet);

            this->agents[i]->reset_move();

    }

    /** switch **/
    for (unsigned  i = 0; i < agent_count; i++){
        auto agent = this->agents[i];
        if (!agent->_switch._flag) continue;
        auto to = agent->_switch._to;
        auto new_agent = this->generate_agent(to);
        agent->patch->remove_agent(); // get the patch empty
        this->connect_patch_agent(agent->patch,new_agent);
        agent->disappear = true;
        agent->reset_switch();

    }
    /** process disappearing **/
    int jj = 0;
    while (true) {
        if (jj >= this->agents.size()) break;
        for (int ii = jj; ii < this->agents.size(); ii++) {
            if (this->agents[ii]->disappear == true) {
                this->agents[ii]->patch->remove_agent();
                this->agents.erase(this->agents.begin() + ii);
                break;
            }
            jj++;
        };
    }
    this->update_repo(); // to remove the agents from repo
    /** update Env data **/
    // update agent counts
    this->count_agents();


    
};
inline map<string,unsigned> baseEnv::count_agents(){
    map<string,unsigned> agents_count;
    for (unsigned i = 0; i < this->agents.size(); i++) {
        auto agent =  this->agents[i];
        agents_count[agent->class_name]++;
        
        agent_classes.insert(agent->class_name);
        
    }
    // to add those agents that were present at setup time but disappeared
    for (auto const & agent_class:this->agent_classes){
        if (agents_count.find(agent_class) == agents_count.end()){
            agents_count[agent_class] = 0;
        }
    }
    this->agents_count = agents_count;
    return agents_count;
}
inline void baseEnv::place_agent(shared_ptr<Patch> patch,shared_ptr<Agent> agent){
    if (!patch->empty) throw patch_availibility("Patch is not empty");
    connect_patch_agent(patch,agent);
}
inline void baseEnv::connect_patch_agent(shared_ptr<Patch> patch,shared_ptr<Agent> agent){
    patch->set_agent(agent);
    agent->set_patch(patch);
}
inline void baseEnv::place_agent_randomly(shared_ptr<Agent> agent){
    auto patch = this->find_empty_patch();
    connect_patch_agent(patch,agent);
}
inline shared_ptr<Patch> baseEnv::find_empty_patch(){
    /**
     * Finds the first empty patch. If none found, throws an exeption.
     */
    auto patches_indices_copy = this->patches_indices;
    auto patch_count = this->patches_indices.size();

    auto g = random_::randomly_seeded_MT();

    std::shuffle(patches_indices_copy.begin(), patches_indices_copy.end(), g);
    
    for (auto const&i:patches_indices_copy){
        auto potential_patch = this->patches.at(i);
        if (potential_patch->empty){
            return potential_patch;
        }
    }
    throw patch_availibility("All patches are occupied.");
}
 inline void baseEnv::setup_domain(vector<MESH_ITEM> mesh){
        /** create patches **/ 
        // step 1: create patches from info of meshes
        for (auto & mesh_item:mesh){
            auto patch = this->generate_patch(); // create patch
            patch->index = mesh_item.index;      // copy index
            patch->layer_index = mesh_item.layer_index;      // copy layer index
            patch->coords = mesh_item.coords;    // copy coords
            patch->neighbors_indices = mesh_item.neighbors_indices;  // copy neighbors indices
            try {
                patch->on_border = mesh_item.on_border;
            }
            catch (...){

            }
            patches[patch->index]= patch;
            this->patches_indices.push_back(patch->index);
        }
        // step 2: assign neighbor patches
        for (auto &[index,patch]:patches){
            vector<shared_ptr<Patch>> neighbors;
            auto neighbors_indices = patch->neighbors_indices;
            for (auto const &neighbor_index:neighbors_indices){
                auto neighbor_patch = patches.at(neighbor_index);
                neighbors.push_back(neighbor_patch);
            }
            patch->neighbors = neighbors;
        }
        // log_mesh(mesh,"mesh.txt");
    }


