/** \file Main file that starts agents and environments
    \author Todd Hester
 */

#include "../include/common/Random.h"
#include "../include/common/core.hh"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <iomanip>
#include <ctime>
#include <sstream> // stringstream

// include input and output archivers
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

// include this header to serialize vectors
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>

#include <boost/serialization/utility.hpp>

/** Plotting tools **/
#include "../include/gnuplot-iostream.h"

#include <iomanip>
//////////////////
// Environments //
//////////////////
#include "../include/envs/BlockRoom.hh"
#include "../include/envs/InfiniteBlocks.hh"



////////////
// Agents //
////////////
// #include "../include/agents/QLearner.hh"
#include "../include/agents/ModelBasedAgent.hh"
/*
#include "../include/agents/DiscretizationAgent.hh"
#include "../include/agents/SavedPolicy.hh"
#include "../include/agents/Dyna.hh"
#include "../include/agents/Sarsa.hh"
*/

// Tutors
#include "../include/tutors/s_dep_tutor.hpp"
#include "../include/tutors/no_tutor.hpp"

#include <vector>
#include <sstream>
#include <iostream>

#include <getopt.h>
#include <stdlib.h>

unsigned NUMEPISODES = 100; //10; //200; //500; //200;
const unsigned NUMTRIALS = 30; //30; //30; //5; //30; //30; //50

unsigned MAXSTEPS = 100; // per episode
bool PRINTS = false;
bool PRETRAIN = false;


void displayHelp(){
	cout << "\n Call experiment --agent type --env type [options]\n";
	cout << "Agent types: qlearner sarsa modelbased rmax texplore dyna savedpolicy\n";
	cout << "Env types: taxi tworooms fourrooms energy fuelworld mcar cartpole car2to7 car7to2 carrandom stocks lightworld\n";
	cout << "Tutor types : s_dep_tutor, no_tutor\n";

	cout << "\n Agent Options:\n";
	cout << "--gamma value (discount factor between 0 and 1)\n";
	cout << "--epsilon value (epsilon for epsilon-greedy exploration)\n";
	cout << "--alpha value (learning rate alpha)\n";
	cout << "--initialvalue value (initial q values)\n";
	cout << "--actrate value (action selection rate (Hz))\n";
	cout << "--lambda value (lambda for eligibility traces)\n";
	cout << "--m value (parameter for R-Max)\n";
	cout << "--k value (For Dyna: # of model based updates to do between each real world update)\n";
	cout << "--history value (# steps of history to use for planning with delay)\n";
	cout << "--filename file (file to load saved policy from for savedpolicy agent)\n";
	cout << "--model type (tabular,tree,m5tree)\n";
	cout << "--planner type (vi,pi,sweeping,uct,parallel-uct,delayed-uct,delayed-parallel-uct)\n";
	cout << "--explore type (unknown,greedy,epsilongreedy,variancenovelty)\n";
	cout << "--combo type (average,best,separate)\n";
	cout << "--nmodels value (# of models)\n";
	cout << "--nstates value (optionally discretize domain into value # of states on each feature)\n";
	cout << "--reltrans (learn relative transitions)\n";
	cout << "--abstrans (learn absolute transitions)\n";
	cout << "--v value (For TEXPLORE: b/v coefficient for rewarding state-actions where models disagree)\n";
	cout << "--n value (For TEXPLORE: n coefficient for rewarding state-actions which are novel)\n";

	cout << "\n Env Options:\n";
	cout << "--deterministic (deterministic version of domain)\n";
	cout << "--stochastic (stochastic version of domain)\n";
	cout << "--delay value (# steps of action delay (for mcar and tworooms)\n";
	cout << "--lag (turn on brake lag for car driving domain)\n";
	cout << "--highvar (have variation fuel costs in Fuel World)\n";
	cout << "--nsectors value (# sectors for stocks domain)\n";
	cout << "--nstocks value (# stocks for stocks domain)\n";

	cout << "\n--prints (turn on debug printing of actions/rewards)\n";
	cout << "--nepisodes value (# of episodes to run (1000 default)\n";
	cout << "--seed value (integer seed for random number generator)\n";
	cout << "--pretrain pre train the model\n";

	cout << "\n For more info, see: http://www.ros.org/wiki/rl_experiment\n";

	exit(-1);

}

experience generateExp(Environment* virtualEnv, int act){
	std::vector<float> sample = virtualEnv->generateSample();
	float virtualReward;

	experience exp;
	exp.s = sample;

	exp.act = act;

	virtualReward = virtualEnv->apply(act).reward;

	sample = virtualEnv->sensation();
	exp.next = sample;

	exp.reward = virtualReward;

	exp.terminal = virtualEnv->terminal();

	virtualEnv->reset();
	return exp;
}

void save_results(std::vector<float> &model_acc,
		std::vector<float> &reward_acc,
		std::vector<float> &accumulated_rewards,
		std::vector<float> &accumulated_tutor_rewards,
		std::vector<float> &accumulated_rewards_pick_red,
		std::vector<float> &accumulated_rewards_pick_blue,
		std::vector<float> &var_prop,
		std::vector<float> &sync_prop,
		std::vector<float> &nov_prop,
		std::vector<float> &reward_prop,
		boost::filesystem::path rootPath){

	ofstream ofs;

	const char* pointer = reinterpret_cast<const char*>(&model_acc[0]);
	size_t bytes = model_acc.size() * sizeof(model_acc[0]);
	ofs.open(rootPath.string()+"/model_acc",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&reward_acc[0]);
	bytes = reward_acc.size() * sizeof(reward_acc[0]);
	ofs.open(rootPath.string()+"/reward_acc",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&accumulated_rewards[0]);
	bytes = accumulated_rewards.size() * sizeof(accumulated_rewards[0]);
	ofs.open(rootPath.string()+"/accumulated_rewards",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&accumulated_tutor_rewards[0]);
	bytes = accumulated_tutor_rewards.size() * sizeof(accumulated_tutor_rewards[0]);
	ofs.open(rootPath.string()+"/accumulated_tutor_rewards",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&accumulated_rewards_pick_red[0]);
	bytes = accumulated_rewards_pick_red.size() * sizeof(accumulated_rewards_pick_red[0]);
	ofs.open(rootPath.string()+"/accumulated_rewards_pick_red",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&accumulated_rewards_pick_blue[0]);
	bytes = accumulated_rewards_pick_blue.size() * sizeof(accumulated_rewards_pick_blue[0]);
	ofs.open(rootPath.string()+"/accumulated_rewards_pick_blue",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&var_prop[0]);
	bytes = var_prop.size() * sizeof(var_prop[0]);
	ofs.open(rootPath.string()+"/var_prop",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&sync_prop[0]);
	bytes = sync_prop.size() * sizeof(sync_prop[0]);
	ofs.open(rootPath.string()+"/sync_prop",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&nov_prop[0]);
	bytes = nov_prop.size() * sizeof(nov_prop[0]);
	ofs.open(rootPath.string()+"/nov_prop",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

	pointer = reinterpret_cast<const char*>(&reward_prop[0]);
	bytes = reward_prop.size() * sizeof(reward_prop[0]);
	ofs.open(rootPath.string()+"/reward_prop",ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();

}

std::pair<float,float> evaluation(Environment* env, Agent* agent, int numactions, Random* rng,
		std::vector<float> minValues, std::vector<float> maxValues, int rRange){

	int K = 1000;
	float reward_error = 0;
	float model_error = 0.;

	for (int testStep=0;testStep<K;testStep++){
		int act = rng->uniformDiscrete(0,numactions);
		experience exp = generateExp(env, act);

		std::tuple<std::vector<float>,float,float> prediction = agent->pred(exp.s, exp.act);
		std::vector<float> predNextState = std::get<0>(prediction);
		float predReward = std::get<1>(prediction);

		float me = env->getEuclidianDistance(predNextState, exp.next, minValues, maxValues);
		model_error += me;

		float re = fabs(predReward-exp.reward)/rRange;
		reward_error += re;

	}

	reward_error /= K;
	model_error /= K;
	return std::make_pair(reward_error, model_error);
}


int main(int argc, char **argv) {

	// default params for env and agent
	char* agentType = "texplore";
	char* envType = "infiniteBlocks";
	float discountfactor = 0.9;
	float epsilon = 0.05;
	float alpha = 0.5;
	float initialvalue = 0.0;
	float actrate = 10.0;
	float lambda = 0.1;
	int M = 0;
	int modelType = C45TREE;
	int predType = BEST;
	int plannerType = ET_UCT_ACTUAL;
	int nmodels = 5;
	bool reltrans = true;
	bool deptrans = false;
	float featPct = 0.2;
	int nstates = 0;
	int k = 1000;
	char *filename = NULL;
	bool stochastic = false;
	int nstocks = 3;
	int nsectors = 3;
	int delay = 0;
	bool lag = false;
	bool highvar = false;
	int history = 0;
	int seed = 1;
	float v = 0.;
	int pretrain_steps = 0;
	int taskTrain = ALL;
	float nTrain = 0.;
	float tTrain = 0.;
	float rTrain = 0.;
	int stepsTrain = 500;
	bool resetQ = false;
	int taskEval = MATCHING;
	float nEval = 0.;
	float tEval = 0.;
	float rEval = 0.;
	int stepsEval = 1000;
	int batchFreq = 1;
	int roomsize = 5;

	// parse other arguments
	char ch;
	const char* optflags = "geairlmopcn:";
	int option_index = 0;
	static struct option long_options[] = {
			{"gamma", 1, 0, 'g'},
			{"discountfactor", 1, 0, 'g'},
			{"epsilon", 1, 0, 'e'},
			{"alpha", 1, 0, 'a'},
			{"initialvalue", 1, 0, 'i'},
			{"actrate", 1, 0, 'r'},
			{"lambda", 1, 0, 'l'},
			{"m", 1, 0, 'm'},
			{"model", 1, 0, 'o'},
			{"planner", 1, 0, 'p'},
			{"combo", 1, 0, 'c'},
			{"nmodels", 1, 0, '#'},
			{"reltrans", 0, 0, 't'},
			{"abstrans", 0, 0, '0'},
			{"seed", 1, 0, 's'},
			{"agent", 1, 0, 'q'},
			{"prints", 0, 0, 'd'},
			{"nstates", 1, 0, 'w'},
			{"k", 1, 0, 'k'},
			{"filename", 1, 0, 'f'},
			{"history", 1, 0, 'y'},

			{"env", 1, 0, 1},
			{"deterministic", 0, 0, 2},
			{"stochastic", 0, 0, 3},
			{"delay", 1, 0, 4},
			{"nsectors", 1, 0, 5},
			{"nstocks", 1, 0, 6},
			{"lag", 0, 0, 7},
			{"nolag", 0, 0, 8},
			{"highvar", 0, 0, 11},
			{"nepisodes", 1, 0, 12},
			{"v", 1, 0, 13},
			{"pretrain", 1, 0, 14},
			{"taskTrain",1,0,15},
			{"nTrain",1,0,16},
			{"tTrain", 1, 0, 17},
			{"rTrain",1 ,0, 18},
			{"stepsTrain", 1, 0, 19},
			{"resetQ", 0, 0, 20},
			{"taskEval", 1, 0, 21},
			{"nEval", 1, 0, 22},
			{"tEval", 1, 0, 23},
			{"rEval", 1, 0, 24},
			{"stepsEval", 1, 0, 25},
			{0, 0, 0, 0}
	};

	while(-1 != (ch = getopt_long_only(argc, argv, optflags, long_options, &option_index))) {
		switch(ch) {

		case 'g':
			discountfactor = std::atof(optarg);
			cout << "discountfactor: " << discountfactor << endl;
			break;

		case 'e':
			epsilon = std::atof(optarg);
			cout << "epsilon: " << epsilon << endl;
			break;

		case 'y':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0){
				history = std::atoi(optarg);
				cout << "history: " << history << endl;
			} else {
				cout << "--history is not a valid option for agent: " << agentType << endl;
				exit(-1);
			}
			break;
		}

		case 'k':
		{
			if (strcmp(agentType, "dyna") == 0){
				k = std::atoi(optarg);
				cout << "k: " << k << endl;
			} else {
				cout << "--k is only a valid option for the Dyna agent" << endl;
				exit(-1);
			}
			break;
		}

		case 'f':
			filename = optarg;
			cout << "policy filename: " <<  filename << endl;
			break;

		case 'a':
		{
			if (strcmp(agentType, "qlearner") == 0 || strcmp(agentType, "dyna") == 0 || strcmp(agentType, "sarsa") == 0){
				alpha = std::atof(optarg);
				cout << "alpha: " << alpha << endl;
			} else {
				cout << "--alpha option is only valid for Q-Learning, Dyna, and Sarsa" << endl;
				exit(-1);
			}
			break;
		}

		case 'i':
		{
			if (strcmp(agentType, "qlearner") == 0 || strcmp(agentType, "dyna") == 0 || strcmp(agentType, "sarsa") == 0){
				initialvalue = std::atof(optarg);
				cout << "initialvalue: " << initialvalue << endl;
			} else {
				cout << "--initialvalue option is only valid for Q-Learning, Dyna, and Sarsa" << endl;
				exit(-1);
			}
			break;
		}

		case 'r':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0 || strcmp(agentType, "rmax") == 0){
				actrate = std::atof(optarg);
				cout << "actrate: " << actrate << endl;
			} else {
				cout << "Model-free methods do not require an action rate" << endl;
				exit(-1);
			}
			break;
		}

		case 'l':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0 || strcmp(agentType, "rmax") == 0 || strcmp(agentType, "sarsa") == 0){
				lambda = std::atof(optarg);
				cout << "lambda: " << lambda << endl;
			} else {
				cout << "--lambda option is invalid for this agent: " << agentType << endl;
				exit(-1);
			}
			break;
		}

		case 'm':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0 || strcmp(agentType, "rmax") == 0){
				M = std::atoi(optarg);
				cout << "M: " << M << endl;
			} else {
				cout << "--M option only useful for model-based agents, not " << agentType << endl;
				exit(-1);
			}
			break;
		}

		case 'o':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0 || strcmp(agentType, "rmax") == 0){
				if (strcmp(optarg, "tabular") == 0) modelType = RMAX;
				else if (strcmp(optarg, "tree") == 0) modelType = C45TREE;
				else if (strcmp(optarg, "texplore") == 0) modelType = C45TREE;
				else if (strcmp(optarg, "c45tree") == 0) modelType = C45TREE;
				else if (strcmp(optarg, "m5tree") == 0) modelType = M5ALLMULTI;
				else if (strcmp(optarg, "stump") == 0) modelType = STUMP;
				else if (strcmp(optarg, "lst") == 0) modelType = LSTMULTI;
				if (strcmp(agentType, "rmax") == 0 && modelType != RMAX){
					cout << "R-Max should use tabular model" << endl;
					exit(-1);
				}
			} else {
				cout << "Model-free methods do not need a model, --model option does nothing for this agent type" << endl;
				exit(-1);
			}
			cout << "model: " << modelNames[modelType] << endl;
			break;
		}

		case 'p':
		{
			if (strcmp(optarg, "vi") == 0) plannerType = VALUE_ITERATION;
			else if (strcmp(optarg, "valueiteration") == 0) plannerType = VALUE_ITERATION;
			else if (strcmp(optarg, "policyiteration") == 0) plannerType = POLICY_ITERATION;
			else if (strcmp(optarg, "pi") == 0) plannerType = POLICY_ITERATION;
			else if (strcmp(optarg, "sweeping") == 0) plannerType = PRI_SWEEPING;
			else if (strcmp(optarg, "prioritizedsweeping") == 0) plannerType = PRI_SWEEPING;
			else if (strcmp(optarg, "uct") == 0) plannerType = ET_UCT_ACTUAL;
			else if (strcmp(optarg, "paralleluct") == 0) plannerType = PAR_ETUCT_ACTUAL;
			else if (strcmp(optarg, "realtimeuct") == 0) plannerType = PAR_ETUCT_ACTUAL;
			else if (strcmp(optarg, "realtime-uct") == 0) plannerType = PAR_ETUCT_ACTUAL;
			else if (strcmp(optarg, "parallel-uct") == 0) plannerType = PAR_ETUCT_ACTUAL;
			else if (strcmp(optarg, "delayeduct") == 0) plannerType = POMDP_ETUCT;
			else if (strcmp(optarg, "delayed-uct") == 0) plannerType = POMDP_ETUCT;
			else if (strcmp(optarg, "delayedparalleluct") == 0) plannerType = POMDP_PAR_ETUCT;
			else if (strcmp(optarg, "delayed-parallel-uct") == 0) plannerType = POMDP_PAR_ETUCT;
			if (strcmp(agentType, "texplore") != 0 && strcmp(agentType, "modelbased") != 0 && strcmp(agentType, "rmax") != 0){
				cout << "Model-free methods do not require planners, --planner option does nothing with this agent" << endl;
				exit(-1);
			}
			if (strcmp(agentType, "rmax") == 0 && plannerType != VALUE_ITERATION){
				cout << "Typical implementation of R-Max would use value iteration, but another planner type is ok" << endl;
			}
			cout << "planner: " << plannerNames[plannerType] << endl;
			break;
		}

		case 'c':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0){
				if (strcmp(optarg, "average") == 0) predType = AVERAGE;
				else if (strcmp(optarg, "weighted") == 0) predType = WEIGHTAVG;
				else if (strcmp(optarg, "best") == 0) predType = BEST;
				else if (strcmp(optarg, "separate") == 0) predType = SEPARATE;
				cout << "predType: " << comboNames[predType] << endl;
			} else {
				cout << "--combo is an invalid option for agent: " << agentType << endl;
				exit(-1);
			}
			break;
		}

		case '#':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0){
				nmodels = std::atoi(optarg);
				cout << "nmodels: " << nmodels << endl;
			} else {
				cout << "--nmodels is an invalid option for agent: " << agentType << endl;
				exit(-1);
			}
			if (nmodels < 1){
				cout << "nmodels must be > 0" << endl;
				exit(-1);
			}
			break;
		}

		case 't':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0){
				reltrans = true;
				cout << "reltrans: " << reltrans << endl;
			} else {
				cout << "--reltrans is an invalid option for agent: " << agentType << endl;
				exit(-1);
			}
			break;
		}

		case '0':
		{
			if (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0){
				reltrans = false;
				cout << "reltrans: " << reltrans << endl;
			} else {
				cout << "--abstrans is an invalid option for agent: " << agentType << endl;
				exit(-1);
			}
			break;
		}

		case 's':
			seed = std::atoi(optarg);
			cout << "seed: " << seed << endl;
			break;

		case 'q':
			// already processed this one
			cout << "agent: " << agentType << endl;
			break;

		case 'd':
			PRINTS = true;
			break;

		case 'w':
			nstates = std::atoi(optarg);
			cout << "nstates for discretization: " << nstates << endl;
			break;

		case 2:
			stochastic = false;
			cout << "stochastic: " << stochastic << endl;
			break;

		case 11:
		{
			if (strcmp(envType, "fuelworld") == 0){
				highvar = true;
				cout << "fuel world fuel cost variation: " << highvar << endl;
			} else {
				cout << "--highvar is only a valid option for the fuelworld domain." << endl;
				exit(-1);
			}
			break;
		}

		case 3:
			stochastic = true;
			cout << "stochastic: " << stochastic << endl;
			break;

		case 4:
		{
			if (strcmp(envType, "mcar") == 0 || strcmp(envType, "tworooms") == 0){
				delay = std::atoi(optarg);
				cout << "delay steps: " << delay << endl;
			} else {
				cout << "--delay option is only valid for the mcar and tworooms domains" << endl;
				exit(-1);
			}
			break;
		}

		case 5:
		{
			if (strcmp(envType, "stocks") == 0){
				nsectors = std::atoi(optarg);
				cout << "nsectors: " << nsectors << endl;
			} else {
				cout << "--nsectors option is only valid for the stocks domain" << endl;
				exit(-1);
			}
			break;
		}

		case 6:
		{
			if (strcmp(envType, "stocks") == 0){
				nstocks = std::atoi(optarg);
				cout << "nstocks: " << nstocks << endl;
			} else {
				cout << "--nstocks option is only valid for the stocks domain" << endl;
				exit(-1);
			}
			break;
		}

		case 7:
		{
			if (strcmp(envType, "car2to7") == 0 || strcmp(envType, "car7to2") == 0 || strcmp(envType, "carrandom") == 0){
				lag = true;
				cout << "lag: " << lag << endl;
			} else {
				cout << "--lag option is only valid for car velocity tasks" << endl;
				exit(-1);
			}
			break;
		}

		case 8:
		{
			if (strcmp(envType, "car2to7") == 0 || strcmp(envType, "car7to2") == 0 || strcmp(envType, "carrandom") == 0){
				lag = false;
				cout << "lag: " << lag << endl;
			} else {
				cout << "--nolag option is only valid for car velocity tasks" << endl;
				exit(-1);
			}
			break;
		}

		case 1:
			// already processed this one
			cout << "env: " << envType << endl;
			break;

		case 12:
			NUMEPISODES = std::atoi(optarg);
			cout << "Num Episodes: " << NUMEPISODES << endl;
			break;
		case 13:
			v = std::atof(optarg);
			break;
		case 14:
			PRETRAIN = true;
			pretrain_steps = std::atof(optarg);
			break;
		case 15:
			taskTrain = std::atoi(optarg);
			break;
		case 16:
			nTrain = std::atof(optarg);
			break;
		case 17:
			tTrain = std::atof(optarg);
			break;
		case 18:
			rTrain = std::atof(optarg);
			break;
		case 19:
			stepsTrain = std::atof(optarg);
			break;
		case 20:
			resetQ = true;
			break;
		case 21:
			taskEval = std::atof(optarg);
			break;
		case 22:
			nEval = std::atof(optarg);
			break;
		case 23:
			tEval = std::atof(optarg);
			break;
		case 24:
			rEval = std::atof(optarg);
			break;
		case 25:
			stepsEval = std::atof(optarg);
			break;
		default:
			displayHelp();
			break;
		}
	}
	cout << "planner :" << plannerType << endl;
	cout << "model : " << modelType << endl;
	cout << "prediction " << predType << endl;

	Random rng(1 + seed);

	std::vector<int> statesPerDim;

	// Construct environment here.
	Environment* e;
	if (strcmp(envType, "infiniteBlocks") == 0){
		if (PRINTS) cout << "Environment: infiniteBlocks \n";
		e = new InfiniteBlocks(rng, roomsize, stochastic, rTrain, taskTrain);
	}

	else {
		std::cerr << "Invalid env type" << endl;
		exit(-1);
	}

	const int numactions = e->getNumActions(); // Most agents will need this?
	const int num_tutor_actions = e->getNumTutorActions();

	std::map<int,std::string> action_names = e->get_action_names();

	std::vector<float> minValues;
	std::vector<float> maxValues;
	e->getMinMaxFeatures(&minValues, &maxValues);
	bool episodic = e->isEpisodic();

	cout << "Environment is ";
	if (!episodic) cout << "NOT ";
	cout << "episodic." << endl;

	// lets just check this for now
	for (unsigned i = 0; i < minValues.size(); i++){
		if (PRINTS) cout << "Feat " << i << " min: " << minValues[i]
																  << " max: " << maxValues[i] << endl;
	}

	// get max/min reward for the domain
	float rMax = 0.0;
	float rMin = -1.0;

	e->getMinMaxReward(&rMin, &rMax);
	float rRange = rMax - rMin;
	if (PRINTS) cout << "Min Reward: " << rMin
			<< ", Max Reward: " << rMax << endl;

	if (statesPerDim.size() == 0){
		cout << "set statesPerDim to " << nstates << " for all dim" << endl;
		statesPerDim.resize(minValues.size(), nstates);
	}

	// Construct tutor here.
	Tutor* tutor = new s_dep_tutor(num_tutor_actions);

	tutor->setTrueEnv(e);

	// agent->evaluate_model();
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(1);
	oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
	oss << "_taskTrain_" << taskTrain;
	oss << "_nTrain_"<<nTrain;
	oss << "_tTrain_"<<tTrain;
	oss << "_rTrain_"<<rTrain;
	oss << "_stepsTrain_"<<stepsTrain;
	oss << "_resetQ_"<<resetQ;
	oss << "_taskEval_"<<taskEval;
	oss << "_nEval_"<<nEval;
	oss << "_tEval_"<<tEval;
	oss << "_rEval_"<<rEval;
	oss << "_stepsEval_"<<stepsEval;

	std::string name = oss.str();


	int eval_freq = 25;
	int maxsteps = stepsEval+stepsTrain;

//	float total_reward = 0;
//	float total_tutor_reward = 0.;


	for (unsigned j = 0; j < NUMTRIALS; ++j) {

		boost::filesystem::path rootPath ( "./resultats_python/" + name +"/trial_"+std::to_string(j));
		boost::system::error_code returnedError;

		boost::filesystem::create_directories( rootPath, returnedError );

		std::vector<float> model_acc((maxsteps+pretrain_steps)/eval_freq+1, 0.);
		std::vector<float> model_acc_train((maxsteps+pretrain_steps)/eval_freq+1, 0.);

		std::vector<float> reward_acc((maxsteps+pretrain_steps)/eval_freq+1, 0.);
		std::vector<float> reward_acc_train((maxsteps+pretrain_steps)/eval_freq+1, 0.);

		std::vector<float> accumulated_rewards((maxsteps+pretrain_steps)/eval_freq+1, 0.);
		std::vector<float> accumulated_tutor_rewards((maxsteps+pretrain_steps)/eval_freq+1, 0.);
		std::vector<float> accumulated_rewards_pick_red((maxsteps+pretrain_steps)/eval_freq+1, 0.);

		std::vector<float> accumulated_rewards_pick_blue((maxsteps+pretrain_steps)/eval_freq+1, 0.);


		std::vector<float> var_prop((maxsteps+pretrain_steps)/eval_freq+1, 0);
		std::vector<float> nov_prop((maxsteps+pretrain_steps)/eval_freq+1, 0);
		std::vector<float> reward_prop((maxsteps+pretrain_steps)/eval_freq+1, 0);
		std::vector<float> sync_prop((maxsteps+pretrain_steps)/eval_freq+1, 0);

		int trial_step = 0;
		float trial_reward = 0.;
		float trial_tutor_reward = 0.;
		float trial_reward_pick_red = 0.;
		float trial_reward_pick_blue = 0.;
		float trial_tutor_reward_2 = 0.;

		float avg_var_prop = 0.;
		float avg_nov_prop = 0.;
		float avg_reward_prop = 0.;
		float avg_sync_prop = 0.;

		// Construct agent here.
		Agent* agent;

		if (strcmp(agentType, "modelbased") == 0 || strcmp(agentType, "rmax") || strcmp(agentType, "texplore")){
			if (PRINTS) cout << "Agent: Model Based" << endl;
			agent = new ModelBasedAgent(numactions,
					discountfactor,
					rMax, rRange,
					modelType,
					predType,
					nmodels,
					plannerType,
					epsilon, // epsilon
					lambda,
					(1.0/actrate), //0.1, //0.1, //0.01, // max time
					M,
					minValues, maxValues,
					statesPerDim,//0,
					history, v, nTrain, tTrain,
					deptrans, reltrans, featPct, stochastic, episodic, batchFreq,
					rng);
			agent->setTrueEnv(e);
			agent->setRewarding(true);
		}

		else {
			std::cerr << "ERROR: Invalid agent type" << endl;
			exit(-1);
		}

		int virtualSeed = 12;
		e->setTutor(true);
		Random virtualRng(virtualSeed);
		Environment* virtualInfinite = new InfiniteBlocks(virtualRng, roomsize, stochastic, rTrain, taskTrain);
		virtualInfinite->setDebug(false);
		virtualInfinite->setVerbose(false);

		if (PRETRAIN && pretrain_steps>0){

			std::cout << "Pretraining :" <<std::endl;

			//std::vector<std::tuple<std::vector<float>, int, std::vector<float>, float>> training_samples;
			float count_r = 0;
			float virtualReward;
			int virtualAct;
			std::vector<float> virtualState;
			std::vector<experience> experiences;

			float model_error_train_r = 0;

			//std::vector<int> numex_succes(numactions,0);
			//int num_rew = 0;

//			std::vector<float> count_acts_train(numactions,0);
			for (int trainStep=0;trainStep<pretrain_steps;trainStep++){

				int virtualAct = rng.uniformDiscrete(0,numactions-1);
				experience exp = generateExp(virtualInfinite, virtualAct);

				count_r += exp.reward;

				experiences.push_back(exp);

				if (trainStep % 1000 == 0 && trainStep !=0){

					std::vector<experience> trainexp(experiences.end()-1000,experiences.end());
					bool modelChanged = agent->train_only_many(trainexp);

					std::cout << "Trial " << j << ",eval at step "<< trainStep << std::endl;

					std::pair<float,float> errors = evaluation(virtualInfinite, agent, numactions, &rng, minValues, maxValues, rRange);

					reward_acc[trainStep/eval_freq] += errors.first;
					model_acc[trainStep/eval_freq] += errors.second;
					
					accumulated_rewards[trainStep/eval_freq] += trial_reward;
					accumulated_rewards_pick_red[trainStep/eval_freq] += trial_reward_pick_red;

					accumulated_rewards_pick_blue[trainStep/eval_freq] += trial_reward_pick_blue;

					accumulated_tutor_rewards[trainStep/eval_freq] += trial_tutor_reward;

					save_results(model_acc,
							reward_acc,
							accumulated_rewards,
							accumulated_tutor_rewards,
							accumulated_rewards_pick_red,
							accumulated_rewards_pick_blue,
							var_prop,
							sync_prop,
							nov_prop,
							reward_prop,
							rootPath);

				}

			}

			trial_step += pretrain_steps;
		}


		// STEP BY STEP DOMAIN
		if (!episodic){

			tutor_feedback t_feedback(0.,0., 0);
			occ_info_t info(0,0,0,0,0);

			int a = 0;


			//////////////////////////////////
			// non-episodic
			//////////////////////////////////
			for (unsigned step = 0; step < maxsteps; ++step){

				std::vector<float> es = e->sensation();

				// first step
				if (step == 0){

					// first action
					int a = agent->first_action(es, &avg_var_prop, &avg_nov_prop, &avg_reward_prop, &avg_sync_prop);
					occ_info_t info = e->apply(a);
					t_feedback = e->tutorAction();
					e->apply_tutor(t_feedback.action);
					agent->setRewarding(false);
				}
				else {
					// next action
					a = agent->next_action(info.reward, es, &avg_var_prop, &avg_nov_prop, &avg_reward_prop, &avg_sync_prop);
					info = e->apply(a);
					t_feedback = e->tutorAction();
					e->apply_tutor(t_feedback.action);
				}

				if (step==stepsTrain) {
					//agent->setRewarding(true);
					agent->setTutorBonus(tEval);
					agent->setNovelty(nEval);
					e->setTask(taskEval);
					e->setReward(rEval);
					if (resetQ){
						agent->forget();
					}
				}

				trial_reward += info.reward;
				trial_tutor_reward += info.tutor_reward;
				trial_reward_pick_red += info.reward_pick_red;
				trial_reward_pick_blue += info.reward_pick_blue;
//				trial_tutor_reward_2 += t_feedback.reward;

				if (step % eval_freq == 0){

					std::cout << "Trial " << j << ",eval at step "<< trial_step+step << std::endl;

					std::pair<float,float> errors = evaluation(virtualInfinite, agent, numactions, &rng, minValues, maxValues, rRange);

					reward_acc[(trial_step+step)/eval_freq] += errors.first;
					model_acc[(trial_step+step)/eval_freq] += errors.second;

					std::cout<< "error reward : "<<errors.first<<", error model : "<<errors.second<<std::endl;

					accumulated_rewards[(trial_step+step)/eval_freq] += trial_reward;
					accumulated_tutor_rewards[(trial_step+step)/eval_freq] += trial_tutor_reward;
					accumulated_rewards_pick_red[(trial_step+step)/eval_freq] += trial_reward_pick_red;
					accumulated_rewards_pick_blue[(trial_step+step)/eval_freq] += trial_reward_pick_blue;

					var_prop[(trial_step+step)/eval_freq] += avg_var_prop;
					nov_prop[(trial_step+step)/eval_freq] += avg_nov_prop;
					reward_prop[(trial_step+step)/eval_freq] += avg_reward_prop;
					sync_prop[(trial_step+step)/eval_freq] += avg_sync_prop;

				}

				if (step % 200 == 0 && step != 0){

					save_results(model_acc,
							reward_acc,
							accumulated_rewards,
							accumulated_tutor_rewards,
							accumulated_rewards_pick_red,
							accumulated_rewards_pick_blue,
							var_prop,
							sync_prop,
							nov_prop,
							reward_prop,
							rootPath);


				}

			}

		}
		else {
			/*


			//////////////////////////////////
			// episodic
			//////////////////////////////////
			for (unsigned i = 0; i < NUMEPISODES; ++i) {

				// performance tracking

				float episode_reward = 0;
				float episode_tutor_reward =0.;
				float episode_tutor_reward_2 = 0.;
				int episode_step = 0;
				tutor_feedback t_feedback(0.,0., 0);

				// first action
				std::vector<float> es = e->sensation();


				int a = agent->first_action(es, &avg_var_prop, &avg_nov_prop, &avg_reward_prop, &avg_sync_prop);
//				std::cout << "Total variance bonus received at step 0 : " << avg_var_prop << std::endl;
//				std::cout << "Total novelty bonus received at step 0 : " << avg_nov_prop << std::endl;
//				std::cout << "Total enironment reward received at step 0 : " << avg_reward_prop << std::endl;
//				std::cout << "Total synchronisation bonus received at step 0 : " << avg_sync_prop << std::endl;
				occ_info_t info = e->apply(a);

				if (with_tutor){
					t_feedback = e->tutorAction();
					e->apply_tutor(t_feedback.action);
				}


				// update performance
				episode_reward += info.reward;
				episode_tutor_reward += t_feedback.tutor_reward;
				episode_tutor_reward_2 += t_feedback.reward;


				while (!e->terminal() && episode_step < maxsteps) {

					if ((trial_step+episode_step) % eval_freq == 0){
						std::cout << "Trial " << j << ",eval at step "<< trial_step+episode_step << std::endl;
						int K = 100;
						float model_error_test_r = 0;
						std::vector<float> model_error_acts(numactions,0);
						std::vector<float> model_error_comp(minValues.size(),0);

						std::vector<float> virtualState;
						float virtualReward;
						int virtualAct;
						for (int testStep=0;testStep<K;testStep++){
							int nb_act = rng.uniformDiscrete(0,(nbRedBlocks+nbBlueBlocks)*6-1);
							for (int i=0; i<nb_act; i++){
								virtualState = virtualInfinite->sensation();
								if (!virtualInfinite->terminal()){
									virtualAct = virtualInfinite->trueBestAction();
									virtualReward = virtualInfinite->apply(virtualAct).reward;
								}
								else{
									virtualInfinite->reset();
								}
							}
							for (int sample_act_test = 0; sample_act_test<numactions; sample_act_test++){
								std::vector<float> sample_test = virtualInfinite->sensation();
								std::tuple<std::vector<float>,float,float> prediction = agent->pred(sample_test, sample_act_test);
								std::vector<float> predNextState = std::get<0>(prediction);
								float predReward = std::get<1>(prediction);

								float reward = virtualInfinite->apply(sample_act_test).reward;
								std::vector<float> new_state = virtualInfinite->sensation();

								float error_test = e->getEuclidianDistance(predNextState, new_state, minValues, maxValues);
								model_error_acts[sample_act_test] += error_test;

								for (int i=0;i<minValues.size();i++){
									model_error_comp[i] += (predNextState[i]-new_state[i])/(maxValues[i]-minValues[i]);
								}

								float error_test_r = fabs(predReward-reward)/rRange;
								model_error_test_r += error_test_r;
							}
						}

						for (int i=0;i<numactions;i++){
							model_error_acts[i]/=K;
							model_acc[i][(trial_step+episode_step)/eval_freq] += model_error_acts[i];
						}

						model_error_test_r /= (K*numactions);
						reward_model_acc[(trial_step+episode_step)/eval_freq] += model_error_test_r;

						for (int i=0;i<minValues.size();i++){
							model_error_comp[i] /= (K*numactions);
							comp_acc[i][(trial_step+episode_step)/eval_freq] += model_error_comp[i];
						}

						accu_rewards[(trial_step+episode_step)/eval_freq] += trial_reward+episode_reward;
						accu_tutor_rewards_2[(trial_step+episode_step)/eval_freq] += trial_tutor_reward_2+episode_tutor_reward_2;
						accu_tutor_rewards[(trial_step+episode_step)/eval_freq] += trial_tutor_reward+episode_tutor_reward;
						step_reached[(trial_step+episode_step)/eval_freq]++;

						var_prop[(trial_step+episode_step)/eval_freq] += avg_var_prop/eval_freq;
						nov_prop[(trial_step+episode_step)/eval_freq] += avg_nov_prop/eval_freq;
						reward_prop[(trial_step+episode_step)/eval_freq] += avg_reward_prop/eval_freq;
						sync_prop[(trial_step+episode_step)/eval_freq] += avg_sync_prop/eval_freq;

						avg_var_prop = 0.;
						avg_nov_prop = 0.;
						avg_reward_prop = 0.;
						avg_sync_prop = 0.;

					}

					++episode_step;
					// perform an action
					es = e->sensation();

					a = agent->next_action(info.reward, es, &avg_var_prop, &avg_nov_prop, &avg_reward_prop, &avg_sync_prop);
//					std::cout << a << std::endl;
//					std::cout << "Total variance bonus received at step " << episode_step << " : " << avg_var_prop << std::endl;
//					std::cout << "Total novelty bonus received at step " << episode_step << " : " << avg_nov_prop << std::endl;
//
//					std::cout << "Total enironment reward received at step " << episode_step << " : " << avg_reward_prop << std::endl;
//					std::cout << "Total synchronisation bonus received at step " << episode_step << " : " << avg_sync_prop << std::endl;
					info = e->apply(a);

					t_feedback = e->tutorAction();
					if (with_tutor){
						e->apply_tutor(t_feedback.action);
					}

//					act_count[a].first++;
//					if (info.success){
//						act_count[a].second++;
//					}


					// update performance
					episode_reward += info.reward;
					episode_tutor_reward += t_feedback.tutor_reward;
					episode_tutor_reward_2 += t_feedback.reward;
				}

				// terminal/last state
				if (e->terminal()){
					agent->last_action(info.reward);
				}else{
					agent->next_action(info.reward, e->sensation(), &avg_var_prop, &avg_nov_prop, &avg_reward_prop, &avg_sync_prop);
				}

				e->reset();

				trial_reward += episode_reward;
				trial_tutor_reward += episode_tutor_reward;
				trial_tutor_reward_2 += episode_tutor_reward_2;

				trial_step += episode_step;
			}
			*/
		}

		delete agent;
	}

}



 // end main

