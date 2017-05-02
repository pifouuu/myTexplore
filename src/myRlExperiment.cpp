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
const unsigned NUMTRIALS = 15; //30; //30; //5; //30; //30; //50

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


int main(int argc, char **argv) {

	// default params for env and agent
	char* agentType = NULL;
	char* envType = NULL;
	char* tutorType = NULL;
	float discountfactor = 0.9;
	float epsilon = 0.05;
	float alpha = 0.5;
	float initialvalue = 0.0;
	float actrate = 10.0;
	float lambda = 0.1;
	int M = 5;
	int modelType = C45TREE;
	int exploreType = GREEDY;
	int predType = BEST;
	int plannerType = PAR_ETUCT_ACTUAL;
	int nmodels = 1;
	bool reltrans = true;
	bool deptrans = false;
	float v = 0.;
	float n = 0.;
	float featPct = 0.2;
	int nstates = 0;
	int k = 1000;
	char *filename = NULL;
	bool stochastic = true;
	int nstocks = 3;
	int nsectors = 3;
	int delay = 0;
	bool lag = false;
	bool highvar = false;
	int history = 0;
	int seed = 1;
	int pretrain_steps = 0;
	float tutorBonus = 10;
	int finalReward = 100;
	int nbRedBlocks = 1;
	int nbBlueBlocks = 0;
	int maxsteps = 100;
	int batchFreq = 1;
	bool rewarding=true;
	int roomsize = 5;
	// change some of these parameters based on command line args

	// parse agent type
	bool gotAgent = false;
	for (int i = 1; i < argc-1; i++){
		if (strcmp(argv[i], "--agent") == 0){
			gotAgent = true;
			agentType = argv[i+1];
		}
	}
	if (!gotAgent) {
		cout << "--agent type  option is required" << endl;
		displayHelp();
	}

	// set some default options for rmax or texplore
	if (strcmp(agentType, "rmax") == 0){
		modelType = RMAX;
		exploreType = EXPLORE_UNKNOWN;
		predType = BEST;
		plannerType = VALUE_ITERATION;
		nmodels = 1;
		reltrans = false;
		M = 5;
		history = 0;
	} else if (strcmp(agentType, "texplore") == 0){
		modelType = C45TREE;
		exploreType = DIFF_AND_NOVEL_BONUS;
		v = 0;
		n = 0;
		predType = AVERAGE;
		plannerType = PAR_ETUCT_ACTUAL;
		nmodels = 5;
		reltrans = true;
		M = 0;
		history = 0;
	}

	// parse env type
	bool gotEnv = false;
	for (int i = 1; i < argc-1; i++){
		if (strcmp(argv[i], "--env") == 0){
			gotEnv = true;
			envType = argv[i+1];
		}
	}
	if (!gotEnv) {
		cout << "--env type  option is required" << endl;
		displayHelp();
	}

	// parse tutor type
	bool gotTutor = false;
	for (int i = 1; i < argc-1; i++){
		if (strcmp(argv[i], "--tutor") == 0){
			gotTutor = true;
			tutorType = argv[i+1];
		}
	}
	if (!gotTutor) {
		cout << "--tutor type  option is required" << endl;
		displayHelp();
	}

	// parse other arguments
	char ch;
	const char* optflags = "geairlmoxpcn:";
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
			{"explore", 1, 0, 'x'},
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
			{"b", 1, 0, 'b'},
			{"v", 1, 0, 'v'},
			{"n", 1, 0, 'n'},

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
			{"tutor", 1, 0, 13},
			{"pretrain", 1, 0, 14},
			{"tutorBonus",1,0,15},
			{"finalReward",1,0,16},
			{"nbred", 1, 0, 17},
			{"nbblue",1 ,0, 18},
			{"maxsteps", 1, 0, 19},
			{"batchFreq", 1, 0, 20},
			{"roomsize", 1, 0, 21},
			{0, 0, 0, 0}
	};

	bool epsilonChanged = false;
	bool actrateChanged = false;
	bool mChanged = false;
	bool bvnChanged = false;
	bool lambdaChanged = false;
	bool with_tutor = true;

	while(-1 != (ch = getopt_long_only(argc, argv, optflags, long_options, &option_index))) {
		switch(ch) {

		case 'g':
			discountfactor = std::atof(optarg);
			cout << "discountfactor: " << discountfactor << endl;
			break;

		case 'e':
			epsilonChanged = true;
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
			actrateChanged = true;
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
			lambdaChanged = true;
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
			mChanged = true;
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

		case 'x':
		{
			if (strcmp(optarg, "unknown") == 0) exploreType = EXPLORE_UNKNOWN;
			else if (strcmp(optarg, "greedy") == 0) exploreType = GREEDY;
			else if (strcmp(optarg, "epsilongreedy") == 0) exploreType = EPSILONGREEDY;
			else if (strcmp(optarg, "unvisitedstates") == 0) exploreType = UNVISITED_BONUS;
			else if (strcmp(optarg, "unvisitedactions") == 0) exploreType = UNVISITED_ACT_BONUS;
			else if (strcmp(optarg, "variancenovelty") == 0) exploreType = DIFF_AND_NOVEL_BONUS;
			//else if (strcmp(optarg, "noveltytutor") == 0) exploreType = DIFF_NOVEL_TUTOR;
			if (strcmp(agentType, "rmax") == 0 && exploreType != EXPLORE_UNKNOWN){
				cout << "R-Max should use \"--explore unknown\" exploration" << endl;
				exit(-1);
			}
			else if (strcmp(agentType, "texplore") != 0 && strcmp(agentType, "modelbased") != 0 && strcmp(agentType, "rmax") != 0 && (exploreType != GREEDY && exploreType != EPSILONGREEDY)) {
				cout << "Model free methods must use either greedy or epsilon-greedy exploration!" << endl;
				exploreType = EPSILONGREEDY;
				exit(-1);
			}
			cout << "explore: " << exploreNames[exploreType] << endl;
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

		case 'v':
		case 'b':
		{
			bvnChanged = true;
			if (strcmp(agentType, "texplore") == 0){
				v = std::atof(optarg);
				cout << "v coefficient (variance bonus): " << v << endl;
			}
			else {
				cout << "--v and --b are invalid options for agent: " << agentType << endl;
				exit(-1);
			}
			break;
		}

		case 'n':
		{
			bvnChanged = true;
			if (strcmp(agentType, "texplore") == 0){
				n = std::atof(optarg);
				cout << "n coefficient (novelty bonus): " << n << endl;
			}
			else {
				cout << "--n is an invalid option for agent: " << agentType << endl;
				exit(-1);
			}
			break;
		}

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
			// already processed this one
			cout << "tutor: " << tutorType << endl;
			if (strcmp(tutorType,"no_tutor") == 0) {with_tutor = false;}
			else {exploreType = DIFF_NOVEL_TUTOR;}
			break;
		case 14:
			PRETRAIN = true;
			pretrain_steps = std::atof(optarg);
			break;
		case 15:
			tutorBonus = std::atof(optarg);
			break;
		case 16:
			finalReward = std::atof(optarg);
			break;
		case 17:
			nbRedBlocks = std::atof(optarg);
			break;
		case 18:
			nbBlueBlocks = std::atof(optarg);
			break;
		case 19:
			maxsteps = std::atof(optarg);
			break;
		case 20:
			batchFreq = std::atof(optarg);
			break;
		case 21:
			roomsize = std::atof(optarg);
			break;
		case 'h':
		case '?':
		case 0:
		default:
			displayHelp();
			break;
		}
	}
	cout << "explore : " << exploreType << endl;
	cout << "planner :" << plannerType << endl;
	cout << "model : " << modelType << endl;
	cout << "prediction " << predType << endl;
	cout << "tutor" << tutorType << endl;
	// default back to greedy if no coefficients
	if (exploreType == DIFF_AND_NOVEL_BONUS && v == 0 && n == 0)
		exploreType = GREEDY;

	// check for conflicting options
	// changed epsilon but not doing epsilon greedy exploration
	/*if (epsilonChanged && exploreType != EPSILONGREEDY){
		cout << "No reason to change epsilon when not using epsilon-greedy exploration" << endl;
		exit(-1);
	}

	// set history value but not d19oing uct w/history planner
	if (history > 0 && (plannerType == VALUE_ITERATION || plannerType == POLICY_ITERATION || plannerType == PRI_SWEEPING)){
		cout << "No reason to set history higher than 0 if not using a UCT planner" << endl;
		exit(-1);
	}

	// set action rate but not doing real-time planner
	if (actrateChanged && (plannerType == VALUE_ITERATION || plannerType == POLICY_ITERATION || plannerType == PRI_SWEEPING)){
		cout << "No reason to set actrate if not using a UCT planner" << endl;
		exit(-1);
	}

	// set lambda but not doing uct (lambda)
	if (lambdaChanged && (strcmp(agentType, "texplore") == 0 || strcmp(agentType, "modelbased") == 0 || strcmp(agentType, "rmax") == 0) && (plannerType == VALUE_ITERATION || plannerType == POLICY_ITERATION || plannerType == PRI_SWEEPING)){
		cout << "No reason to set actrate if not using a UCT planner" << endl;
		exit(-1);
	}

	// set n/v/b but not doing that diff_novel exploration
	if (bvnChanged && exploreType != DIFF_AND_NOVEL_BONUS){
		cout << "No reason to set n or v if not doing variance & novelty exploration" << endl;
		exit(-1);
	}

	// set combo other than best but only doing 1 model
	if (predType != BEST && nmodels == 1){
		cout << "No reason to have model combo other than best with nmodels = 1" << endl;
		exit(-1);
	}

	// set M but not doing explore unknown
	if (mChanged && exploreType != EXPLORE_UNKNOWN){
		cout << "No reason to set M if not doing R-max style Explore Unknown exploration" << endl;
		exit(-1);
	}
*/
	if (PRINTS){
		if (stochastic)
			cout << "Stohastic\n";
		else
			cout << "Deterministic\n";
	}

	Random rng(1 + seed);

	std::vector<int> statesPerDim;

	// Construct environment here.
	Environment* e;
	if (strcmp(envType, "infiniteBlocks") == 0){
		if (PRINTS) cout << "Environment: infiniteBlocks \n";
		e = new InfiniteBlocks(rng, roomsize, with_tutor, stochastic, finalReward);
	}

	/*else if (strcmp(envType, "cartpole") == 0){
		if (PRINTS) cout << "Environment: Cart Pole\n";
		e = new CartPole(rng, stochastic);
	}

	else if (strcmp(envType, "mcar") == 0){
		if (PRINTS) cout << "Environment: Mountain Car\n";
		e = new MountainCar(rng, stochastic, false, delay);
	}

	// taxi
	else if (strcmp(envType, "taxi") == 0){
		if (PRINTS) cout << "Environment: Taxi\n";
		e = new Taxi(rng, stochastic);
	}

	// Light World
	else if (strcmp(envType, "lightworld") == 0){
		if (PRINTS) cout << "Environment: Light World\n";
		e = new LightWorld(rng, stochastic, 4);
	}

	// two rooms
	else if (strcmp(envType, "tworooms") == 0){
		if (PRINTS) cout << "Environment: TwoRooms\n";
		e = new TwoRooms(rng, stochastic, true, delay, false);
	}

	// car vel, 2 to 7
	else if (strcmp(envType, "car2to7") == 0){
		if (PRINTS) cout << "Environment: Car Velocity 2 to 7 m/s\n";
		e = new RobotCarVel(rng, false, true, false, lag);
		statesPerDim.resize(4,0);
		statesPerDim[0] = 12;
		statesPerDim[1] = 120;
		statesPerDim[2] = 4;
		statesPerDim[3] = 10;
		MAXSTEPS = 100;
	}
	// car vel, 7 to 2
	else if (strcmp(envType, "car7to2") == 0){
		if (PRINTS) cout << "Environment: Car Velocity 7 to 2 m/s\n";
		e = new RobotCarVel(rng, false, false, false, lag);
		statesPerDim.resize(4,0);
		statesPerDim[0] = 12;
		statesPerDim[1] = 120;
		statesPerDim[2] = 4;
		statesPerDim[3] = 10;
		MAXSTEPS = 100;
	}
	// car vel, random vels
	else if (strcmp(envType, "carrandom") == 0){
		if (PRINTS) cout << "Environment: Car Velocity Random Velocities\n";
		e = new RobotCarVel(rng, true, false, false, lag);
		statesPerDim.resize(4,0);
		statesPerDim[0] = 12;
		statesPerDim[1] = 48;
		statesPerDim[2] = 4;
		statesPerDim[3] = 10;
		MAXSTEPS = 100;
	}

	// four rooms
	else if (strcmp(envType, "fourrooms") == 0){
		if (PRINTS) cout << "Environment: FourRooms\n";
		e = new FourRooms(rng, stochastic, true, false);
	}

	// four rooms with energy level
	else if (strcmp(envType, "energy") == 0){
		if (PRINTS) cout << "Environment: EnergyRooms\n";
		e = new EnergyRooms(rng, stochastic, true, false);
	}

	// gridworld with fuel (fuel stations on top and bottom with random costs)
	else if (strcmp(envType, "fuelworld") == 0){
		if (PRINTS) cout << "Environment: FuelWorld\n";
		e = new FuelRooms(rng, highvar, stochastic);
	}

	// stocks
	else if (strcmp(envType, "stocks") == 0){
		if (PRINTS) cout << "Enironment: Stocks with " << nsectors
				<< " sectors and " << nstocks << " stocks\n";
		e = new Stocks(rng, stochastic, nsectors, nstocks);
	}*/

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

	// set rmax as a bonus for certain exploration types
	if (rMax <= 0.0 && (exploreType == TWO_MODE_PLUS_R ||
			exploreType == CONTINUOUS_BONUS_R ||
			exploreType == CONTINUOUS_BONUS ||
			exploreType == THRESHOLD_BONUS_R)){
		rMax = 1.0;
	}




	if (statesPerDim.size() == 0){
		cout << "set statesPerDim to " << nstates << " for all dim" << endl;
		statesPerDim.resize(minValues.size(), nstates);
	}

	// Construct tutor here.
	Tutor* tutor;

	if (!with_tutor){
		std::cout<<"Tutor : without a tutor."<<std::endl;
		tutor = new no_tutor(num_tutor_actions);
	}
	else if (strcmp(tutorType, "s_dep_tutor") == 0) {
		if (PRINTS) cout << "Tutor: State-dependent" << endl;
		tutor = new s_dep_tutor(num_tutor_actions);
	}
	else {
		std::cerr << "ERROR: Invalid tutor type" << endl;
		exit(-1);
	}
	tutor->setTrueEnv(e);

	// agent->evaluate_model();
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
	auto str = oss.str();

	std::string name = str;
	name += "_"+std::to_string(modelType)+"_";
	name += "_v_"+std::to_string(v);
	name += "_n_"+std::to_string(n);
	name += "_tb_"+std::to_string(tutorBonus);
	name += "_pretrain_"+std::to_string(pretrain_steps);
	name += "_fR_"+std::to_string(finalReward);
	name += "_nbR_"+std::to_string(nbRedBlocks)+"_nbB_"+std::to_string(nbBlueBlocks);
	name += "_nmodels_"+std::to_string(nmodels);
	name += "_batch_"+std::to_string(batchFreq);
	name += "_steps_"+std::to_string(maxsteps);
	name += "_size_"+std::to_string(roomsize);
//	name += "_explo";
	boost::filesystem::path rootPath ( "./resultats_5/" + name );
	boost::system::error_code returnedError;

	boost::filesystem::create_directories( rootPath, returnedError );

	int eval_freq = 25;

	//std::vector<act_info_t> act_to_occ;
//	std::vector<std::pair<float,float>> act_count(numactions);

	//std::map<int,std::vector<std::pair<float, float>>> plot_act_succes;
//	std::map<int,std::vector<std::pair<float, float>>> plot_act_try;
//	std::map<int, std::vector<std::pair<float,float>>> plot_act_acc;
//	std::list<std::pair<int,int>> plot_blocks_in;
//	std::list<std::pair<int,int>> plot_blocks_right;
	std::map<int, std::vector<float>> model_acc;
	std::map<int, std::vector<float>> model_acc_train;
	for (int act=0;act<numactions;act++){
		model_acc[act] = std::vector<float>((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);
		model_acc_train[act] = std::vector<float>((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);
	}

	std::vector<std::vector<float>> comp_acc(minValues.size());
	std::vector<std::vector<float>> comp_acc_train(minValues.size());
	for (int comp=0;comp<minValues.size();comp++){
		comp_acc[comp] = std::vector<float>((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);
		comp_acc_train[comp] = std::vector<float>((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);

	}

	std::vector<float> reward_model_acc((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);
	std::vector<float> reward_model_acc_train((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);

	std::vector<float> accu_rewards((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);
	std::vector<float> accu_tutor_rewards((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);
	std::vector<float> accu_tutor_rewards_2((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0.);

	std::vector<int> step_reached((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0);
	std::vector<int> eval_steps((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, -pretrain_steps);

	std::vector<float> var_prop((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0);
	std::vector<float> nov_prop((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0);
	std::vector<float> reward_prop((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0);
	std::vector<float> sync_prop((NUMEPISODES*maxsteps+pretrain_steps)/eval_freq+1, 0);

	for (int tmp = 1; tmp<eval_steps.size();tmp++){
		eval_steps[tmp]+=eval_freq*tmp;
	}



//	float total_reward = 0;
//	float total_tutor_reward = 0.;


	for (unsigned j = 0; j < NUMTRIALS; ++j) {

		int trial_step = 0;
		float trial_reward = 0.;
		float trial_tutor_reward = 0.;
		float trial_tutor_reward_2 = 0.;

		float avg_var_prop = 0.;
		float avg_nov_prop = 0.;
		float avg_reward_prop = 0.;
		float avg_sync_prop = 0.;

		// Construct agent here.
		Agent* agent;

		/*if (strcmp(agentType, "qlearner") == 0){
			if (PRINTS) cout << "Agent: QLearner" << endl;
			agent = new QLearner(numactions,
					discountfactor,
					initialvalue, //0.0, // initialvalue
					alpha, // alpha
					epsilon, // epsilon
					rng);
		}

		else if (strcmp(agentType, "dyna") == 0){
			if (PRINTS) cout << "Agent: Dyna" << endl;
			agent = new Dyna(numactions,
					discountfactor,
					initialvalue, //0.0, // initialvalue
					alpha, // alpha
					k, // k
					epsilon, // epsilon
					rng);
		}

		else if (strcmp(agentType, "sarsa") == 0){
			if (PRINTS) cout << "Agent: SARSA" << endl;
			agent = new Sarsa(numactions,
					discountfactor,
					initialvalue, //0.0, // initialvalue
					alpha, // alpha
					epsilon, // epsilon
					lambda,
					rng);
		}*/

		if (strcmp(agentType, "modelbased") == 0 || strcmp(agentType, "rmax") || strcmp(agentType, "texplore")){
			if (PRINTS) cout << "Agent: Model Based" << endl;
			agent = new ModelBasedAgent(numactions,
					discountfactor,
					rMax, rRange,
					modelType,
					exploreType,
					predType,
					nmodels,
					plannerType,
					epsilon, // epsilon
					lambda,
					(1.0/actrate), //0.1, //0.1, //0.01, // max time
					M,
					minValues, maxValues,
					statesPerDim,//0,
					history, v, n, tutorBonus,
					deptrans, reltrans, featPct, stochastic, episodic, rewarding, batchFreq,
					rng);
			agent->setTrueEnv(e);
		}

		/*else if (strcmp(agentType, "savedpolicy") == 0){
			if (PRINTS) cout << "Agent: Saved Policy" << endl;
			agent = new SavedPolicy(numactions,filename);
		}*/

		else {
			std::cerr << "ERROR: Invalid agent type" << endl;
			exit(-1);
		}

		// start discrete agent if we're discretizing (if nstates > 0 and not agent type 'c')
		/*int totalStates = 1;
		Agent* a2 = agent;
		// not for model based when doing continuous model
		if (nstates > 0 && (modelType != M5ALLMULTI || strcmp(agentType, "qlearner") == 0)){
			int totalStates = powf(nstates,minValues.size());
			if (PRINTS) cout << "Discretize with " << nstates << ", total: " << totalStates << endl;
			agent = new DiscretizationAgent(nstates, a2,
					minValues, maxValues, PRINTS);
		}
		else {
			totalStates = 1;
			for (unsigned i = 0; i < minValues.size(); i++){
				int range = 1+maxValues[i] - minValues[i];
				totalStates *= range;
			}
			if (PRINTS) cout << "No discretization, total: " << totalStates << endl;
		}*/

		// before we start, seed the agent with some experiences
		// agent->seedExp(e->getSeedings());

		struct act_info_t{
			std::list<int> occ_list;
			int success;
		};


		int virtualSeed = 12;
		Random virtualRng(virtualSeed);
		Environment* virtualInfinite = new InfiniteBlocks(virtualRng, roomsize, with_tutor, stochastic, finalReward);
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
				// 23 = nb blocks * 6 étapes par bloc -1
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



				experience exp;
				exp.s = virtualInfinite->sensation();

				virtualAct = rng.uniformDiscrete(0, virtualInfinite->getNumActions()-1);
				exp.act = virtualAct;

				virtualReward = virtualInfinite->apply(virtualAct).reward;
				exp.next = virtualInfinite->sensation();

				exp.reward = virtualReward;


				exp.terminal = virtualInfinite->terminal();
				count_r += exp.reward;

				experiences.push_back(exp);

//				if (exp.next!=exp.s) numex_succes[virtualAct]++;
//				if (exp.reward>0) num_rew++;

				virtualInfinite->reset();
				bool modelChanged = agent->train_only(exp);

				if (trainStep % 1000 == 0 && trainStep !=0){

					std::vector<experience> trainexp(experiences.end()-1000,experiences.end());
					bool modelChanged = agent->train_only_many(trainexp);

					std::vector<int> numex(numactions,0);
					std::vector<float> model_error_acts_train(numactions,0);
					std::vector<float> model_error_comp_train(minValues.size(),0);
					int K = 100;

					for (int evalTrain=0;evalTrain<numactions*K;evalTrain++){
						std::tuple<std::vector<float>,float,float> prediction;
						experience rand_exp = experiences[rng.uniformDiscrete(0, experiences.size()-1)];
						prediction = agent->pred(rand_exp.s, rand_exp.act);
						numex[rand_exp.act]++;
						std::vector<float> predNextState = std::get<0>(prediction);
						float predReward = std::get<1>(prediction);

						float error_train = e->getEuclidianDistance(predNextState, rand_exp.next, minValues, maxValues);
						if (error_train>0){
							model_error_acts_train[rand_exp.act]++;
						}

						for (int i=0;i<minValues.size();i++){
							if (predNextState[i]!=rand_exp.next[i]) model_error_comp_train[i]++;
//								model_error_comp[i] += (predNextState[i]-new_state[i])/(maxValues[i]-minValues[i]);
						}


						float error_train_r = fabs(predReward-rand_exp.reward)/rRange;
						model_error_train_r += error_train_r;
					}

					for (int i=0;i<numactions;i++){
						if (numex[i]!=0) model_error_acts_train[i]/=numex[i];
						model_acc_train[i][trainStep/eval_freq] += model_error_acts_train[i];
					}

					model_error_train_r /= (numactions*K);
					reward_model_acc_train[trainStep/eval_freq] += model_error_train_r;

					for (int i=0;i<minValues.size();i++){
						model_error_comp_train[i] /= (numactions*K);
						comp_acc_train[i][trainStep/eval_freq] += model_error_comp_train[i];
					}

//					std::fill(numex.begin(), numex.end(), 0);
//					std::fill(model_error_acts_train.begin(), model_error_acts_train.end(), 0);
//					std::fill(model_error_comp_train.begin(), model_error_comp_train.end(), 0);

					std::cout << "Evaluation during pretraining, step " << trainStep << std::endl;


					float model_error_test_r = 0;
					std::vector<float> model_error_acts(numactions,0);
					std::vector<float> model_error_comp(minValues.size(),0);
					std::vector<float> virtualState;
					float virtualReward;
					int virtualAct;
					for (int evalTest=0;evalTest<K;evalTest++){
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
							if (error_test>0){
								model_error_acts[sample_act_test]++;
							}
//							model_error_acts[sample_act_test] += error_test;

							for (int i=0;i<minValues.size();i++){
								if (predNextState[i]!=new_state[i]) model_error_comp[i]++;
//								model_error_comp[i] += (predNextState[i]-new_state[i])/(maxValues[i]-minValues[i]);
							}


							float error_test_r = fabs(predReward-reward)/rRange;
							model_error_test_r += error_test_r;
						}
					}

					float toterror;
					for (int i=0;i<numactions;i++){
						model_error_acts[i]/=K;
						toterror+= model_error_acts[i];
						model_acc[i][trainStep/eval_freq] += model_error_acts[i];
					}
					std::cout<<"error model : "<<toterror/numactions<<std::endl;

					model_error_test_r /= (K*numactions);
					reward_model_acc[trainStep/eval_freq] += model_error_test_r;

					for (int i=0;i<minValues.size();i++){
						model_error_comp[i] /= (K*numactions);
						comp_acc[i][trainStep/eval_freq] += model_error_comp[i];
					}

					step_reached[trainStep/eval_freq]++;

					for (std::map<int, std::vector<float>>::iterator it = model_acc.begin();
							it != model_acc.end(); ++it){
						std::ofstream ofs(rootPath.string()+"/model_acc_"+action_names[it->first]+".ser");
						boost::archive::text_oarchive oa(ofs);
						oa & it->second;
					}

					for (int i=0;i<minValues.size();i++){
						std::ofstream ofs(rootPath.string()+"/component_acc_"+std::to_string(i)+".ser");
						boost::archive::text_oarchive oa(ofs);
						oa & comp_acc[i];
					}

					for (std::map<int, std::vector<float>>::iterator it = model_acc_train.begin();
							it != model_acc_train.end(); ++it){
						std::ofstream ofs(rootPath.string()+"/model_acc_train_"+action_names[it->first]+".ser");
						boost::archive::text_oarchive oa(ofs);
						oa & it->second;
					}

					for (int i=0;i<minValues.size();i++){
						std::ofstream ofs(rootPath.string()+"/component_acc_train"+std::to_string(i)+".ser");
						boost::archive::text_oarchive oa(ofs);
						oa & comp_acc_train[i];
					}

					std::ofstream ofs(rootPath.string()+"/reward_model_acc"+".ser");
					boost::archive::text_oarchive oa_model_acc_test_r(ofs);
					oa_model_acc_test_r & reward_model_acc;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/reward_model_acc_train"+".ser");
					boost::archive::text_oarchive oa_model_acc_train_r(ofs);
					oa_model_acc_train_r & reward_model_acc_train;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/x_axis.ser");
					boost::archive::text_oarchive x_axis(ofs);
					x_axis & eval_steps;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/num_trials.ser");
					boost::archive::text_oarchive num_trials(ofs);
					num_trials & step_reached;
					ofs.close();
					ofs.clear();

				}

			}

			trial_step += pretrain_steps;
		}


		// STEP BY STEP DOMAIN
		if (!episodic){

			tutor_feedback t_feedback(0.,0., 0);
			occ_info_t info(0,0,0,0,0);

			int a = 0;
			int endExploration = 0;
			int endTutor = 10000;

			//////////////////////////////////
			// non-episodic
			//////////////////////////////////
			for (unsigned step = 0; step < maxsteps; ++step){

				if (step % eval_freq == 0 && step !=0){
					std::cout << "Trial " << j << ",eval at step "<< trial_step+step << std::endl;
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
					float toterror;
					for (int i=0;i<numactions;i++){
						model_error_acts[i]/=K;
						toterror+= model_error_acts[i];
						model_acc[i][(trial_step+step)/eval_freq] += model_error_acts[i];

					}
					std::cout<<"error model : "<<toterror/numactions<<std::endl;
					toterror = 0;

					model_error_test_r /= (K*numactions);
					reward_model_acc[(trial_step+step)/eval_freq] += model_error_test_r;

					for (int i=0;i<minValues.size();i++){
						model_error_comp[i] /= (K*numactions);
						comp_acc[i][(trial_step+step)/eval_freq] += model_error_comp[i];
					}

					accu_rewards[(trial_step+step)/eval_freq] += trial_reward;
//					accu_tutor_rewards_2[(trial_step+step)/eval_freq] += trial_tutor_reward_2;
					accu_tutor_rewards[(trial_step+step)/eval_freq] += trial_tutor_reward;
					step_reached[(trial_step+step)/eval_freq]++;

					var_prop[(trial_step+step)/eval_freq] += avg_var_prop/eval_freq;
					nov_prop[(trial_step+step)/eval_freq] += avg_nov_prop/eval_freq;
					reward_prop[(trial_step+step)/eval_freq] += avg_reward_prop/eval_freq;
					sync_prop[(trial_step+step)/eval_freq] += avg_sync_prop/eval_freq;

					avg_var_prop = 0.;
					avg_nov_prop = 0.;
					avg_reward_prop = 0.;
					avg_sync_prop = 0.;

				}

				std::vector<float> es = e->sensation();

				// first step
				if (step == 0){

					// first action
					int a = agent->first_action(es, &avg_var_prop, &avg_nov_prop, &avg_reward_prop, &avg_sync_prop);
					occ_info_t info = e->apply(a);
					if (with_tutor){
						t_feedback = e->tutorAction();
						e->apply_tutor(t_feedback.action);
					}
					agent->setRewarding(false);


				}
				else {
					// next action
					a = agent->next_action(info.reward, es, &avg_var_prop, &avg_nov_prop, &avg_reward_prop, &avg_sync_prop);
					info = e->apply(a);
					t_feedback = e->tutorAction();
					if (with_tutor){
						e->apply_tutor(t_feedback.action);
					}
				}

				if (step==endExploration) {
					agent->setRewarding(true);
				}
				if (step==endTutor){
					e->tutorStop();
				}


				trial_reward += info.reward;
				trial_tutor_reward += info.tutor_reward;
//				trial_tutor_reward_2 += t_feedback.reward;

				if (step % 200 == 0 && step != 0){
					for (std::map<int, std::vector<float>>::iterator it = model_acc.begin();
							it != model_acc.end(); ++it){
						std::ofstream ofs(rootPath.string()+"/model_acc_"+action_names[it->first]+".ser");
						boost::archive::text_oarchive oa(ofs);
						oa & it->second;
					}

					for (int i=0;i<minValues.size();i++){
						std::ofstream ofs(rootPath.string()+"/component_acc_"+std::to_string(i)+".ser");
						boost::archive::text_oarchive oa(ofs);
						oa & comp_acc[i];
					}

					std::ofstream ofs(rootPath.string()+"/reward_model_acc"+".ser");
					boost::archive::text_oarchive oa_model_acc_test_r(ofs);
					oa_model_acc_test_r & reward_model_acc;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/accumulated_reward.ser");
					boost::archive::text_oarchive oa_accu_reward(ofs);
					oa_accu_reward & accu_rewards;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/accu_tutor_rewards.ser");
					boost::archive::text_oarchive oa_tutor_r(ofs);
					oa_tutor_r & accu_tutor_rewards;
					ofs.close();
					ofs.clear();

//					ofs.open(rootPath.string()+"/accu_tutor_rewards_2.ser");
//					boost::archive::text_oarchive oa_tutor_r_2(ofs);
//					oa_tutor_r_2 & accu_tutor_rewards_2;
//					ofs.close();
//					ofs.clear();

					ofs.open(rootPath.string()+"/var_prop.ser");
					boost::archive::text_oarchive oa_var(ofs);
					oa_var & var_prop;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/nov_prop.ser");
					boost::archive::text_oarchive oa_nov(ofs);
					oa_nov & nov_prop;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/sync_prop.ser");
					boost::archive::text_oarchive oa_sync(ofs);
					oa_sync & sync_prop;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/reward_prop.ser");
					boost::archive::text_oarchive oa_reward(ofs);
					oa_reward & reward_prop;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/x_axis.ser");
					boost::archive::text_oarchive x_axis(ofs);
					x_axis & eval_steps;
					ofs.close();
					ofs.clear();

					ofs.open(rootPath.string()+"/num_trials.ser");
					boost::archive::text_oarchive num_trials(ofs);
					num_trials & step_reached;
					ofs.close();
					ofs.clear();
				}

			}



		}

		// EPISODIC DOMAINS
		else {


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
			/*	act_count[a].first++;
				if (info.success){
					act_count[a].second++;
				}*/

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

				for (std::map<int, std::vector<float>>::iterator it = model_acc.begin();
						it != model_acc.end(); ++it){
					std::ofstream ofs(rootPath.string()+"/model_acc_"+action_names[it->first]+".ser");
					boost::archive::text_oarchive oa(ofs);
					oa & it->second;
				}

				for (int i=0;i<minValues.size();i++){
					std::ofstream ofs(rootPath.string()+"/component_acc_"+std::to_string(i)+".ser");
					boost::archive::text_oarchive oa(ofs);
					oa & comp_acc[i];
				}

				std::ofstream ofs(rootPath.string()+"/reward_model_acc"+".ser");
				boost::archive::text_oarchive oa_model_acc_test_r(ofs);
				oa_model_acc_test_r & reward_model_acc;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/accumulated_reward.ser");
				boost::archive::text_oarchive oa_accu_reward(ofs);
				oa_accu_reward & accu_rewards;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/accu_tutor_rewards.ser");
				boost::archive::text_oarchive oa_tutor_r(ofs);
				oa_tutor_r & accu_tutor_rewards;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/accu_tutor_rewards_2.ser");
				boost::archive::text_oarchive oa_tutor_r_2(ofs);
				oa_tutor_r_2 & accu_tutor_rewards_2;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/var_prop.ser");
				boost::archive::text_oarchive oa_var(ofs);
				oa_var & var_prop;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/nov_prop.ser");
				boost::archive::text_oarchive oa_nov(ofs);
				oa_nov & nov_prop;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/sync_prop.ser");
				boost::archive::text_oarchive oa_sync(ofs);
				oa_sync & sync_prop;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/reward_prop.ser");
				boost::archive::text_oarchive oa_reward(ofs);
				oa_reward & reward_prop;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/x_axis.ser");
				boost::archive::text_oarchive x_axis(ofs);
				x_axis & eval_steps;
				ofs.close();
				ofs.clear();

				ofs.open(rootPath.string()+"/num_trials.ser");
				boost::archive::text_oarchive num_trials(ofs);
				num_trials & step_reached;
				ofs.close();
				ofs.clear();
			}

		}

		delete agent;
	}

}



 // end main

