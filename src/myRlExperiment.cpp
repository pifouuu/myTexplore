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

const unsigned NUMTRIALS = 30;


/*
experience generateExp(Environment* virtualEnv, int act){
	std::vector<float> sample = virtualEnv->generateSample();
	float virtualReward;

	experience exp;
	exp.s = sample;

	exp.act = act;

	virtualReward = virtualEnv->apply(act).RL_reward;

	sample = virtualEnv->sensation();
	exp.next = sample;

	exp.reward = virtualReward;

	exp.terminal = virtualEnv->terminal();

	virtualEnv->reset();
	return exp;
}
*/

void save_result(std::vector<float> &tab, std::string name, boost::filesystem::path rootPath){
	ofstream ofs;
	const char* pointer = reinterpret_cast<const char*>(&tab[0]);
	size_t bytes = tab.size() * sizeof(tab);
	ofs.open(rootPath.string()+"/"+name, ios::out | ios::binary);
	ofs.write(pointer, bytes);
	ofs.close();
	ofs.clear();
}

/*
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
*/

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
	int M = 5;
	int modelType = C45TREE;
	int predType = AVERAGE;
	int plannerType = ET_UCT_ACTUAL;
	int nmodels = 5;
	bool reltrans = true;
	bool deptrans = false;
	float featPct = 0.2;
	int k = 1000;
	char *filename = NULL;
	bool stochastic = false;
	int history = 0;
	int seed = 1;
	//int pretrain_steps = 0;
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
			{"k", 1, 0, 'k'},
			{"filename", 1, 0, 'f'},
			{"history", 1, 0, 'y'},

			{"env", 1, 0, 1},
			{"deterministic", 0, 0, 2},
			{"stochastic", 0, 0, 3},
			{"nepisodes", 1, 0, 12},
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

	// Read and update parameters from command line arguments
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

		case 2:
			stochastic = false;
			cout << "stochastic: " << stochastic << endl;
			break;

		case 3:
			stochastic = true;
			cout << "stochastic: " << stochastic << endl;
			break;

		case 1:
			// already processed this one
			cout << "env: " << envType << endl;
			break;

		/*case 14:
			PRETRAIN = true;
			pretrain_steps = std::atof(optarg);
			break;*/
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
			return 1;
			break;
		}
	}

	// Random generator for all random searches/selections
	Random rng(1 + seed);

	// Init time
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	// Init name for saving results
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

	for (unsigned j = 0; j < NUMTRIALS; ++j) {

		boost::filesystem::path rootPath ( "/home/pierre/workspace/myTexplore/resultats/folder1/" + name +"/trial_"+std::to_string(j));
		boost::system::error_code returnedError;

		boost::filesystem::create_directories(rootPath, returnedError);

		// Add pretrain steps in vector lengths if need be
		// Initialize all result vectors
		std::vector<float> model_acc((maxsteps)/eval_freq+1, 0.);
		std::vector<float> model_acc_train((maxsteps)/eval_freq+1, 0.);

		std::vector<float> reward_acc((maxsteps)/eval_freq+1, 0.);
		std::vector<float> reward_acc_train((maxsteps)/eval_freq+1, 0.);

		std::vector<float> accumulated_RL_rewards((maxsteps)/eval_freq+1, 0.);
		std::vector<float> accumulated_ALL_rewards((maxsteps)/eval_freq+1, 0.);
		std::vector<float> accumulated_MATCHING_rewards((maxsteps)/eval_freq+1, 0.);
		std::vector<float> accumulated_OPPOSITE_rewards((maxsteps)/eval_freq+1, 0.);
		std::vector<float> accumulated_RED_rewards((maxsteps)/eval_freq+1, 0.);

		std::vector<float> accumulated_rewards_pick_red((maxsteps)/eval_freq+1, 0.);

		std::vector<float> accumulated_rewards_pick_blue((maxsteps)/eval_freq+1, 0.);

		std::vector<float> var_prop((maxsteps)/eval_freq+1, 0);
		std::vector<float> nov_prop((maxsteps)/eval_freq+1, 0);
		std::vector<float> reward_prop((maxsteps)/eval_freq+1, 0);
		std::vector<float> sync_prop((maxsteps)/eval_freq+1, 0);

		// Initialize global results for this trial
		int trial_step = 0;
		float trial_RL_reward = 0.;
		float trial_ALL_reward = 0.;
		float trial_MATCHING_reward = 0.;
		float trial_OPPOSITE_reward = 0.;
		float trial_RED_reward = 0.;

		//float trial_reward_pick_red = 0.;
		//float trial_reward_pick_blue = 0.;

		float avg_var_prop = 0.;
		float avg_nov_prop = 0.;
		float avg_reward_prop = 0.;
		float avg_sync_prop = 0.;

		Environment* e;
		// Only infiniteBlocks is supported for now

		if (strcmp(envType, "infiniteBlocks") == 0){
			e = new InfiniteBlocks(rng, roomsize, stochastic, rTrain, taskTrain);
		}
		else {
			std::cerr << "Invalid env type" << endl;
			exit(-1);
		}

		const int numactions = e->getNumActions();

		/*std::map<int, std::vector<float>> act_success_rates;
		for (int i = 0; i<numactions; i++){
			act_success_rates[i] = std::vector<float>((maxsteps)/eval_freq+1, 0.);
		}
		std::map<int, std::pair<float,float>> act_success_rate;
		for (int i=0; i<numactions;i++){
			act_success_rate[i] = std::make_pair(0.,0.);
		}*/

		std::map<int,std::string> action_names = e->get_action_names();

		// get max/min value for all features of the state in the environment.
		// This is cheating as the agent should not know this beforehand.
		std::vector<float> minValues;
		std::vector<float> maxValues;
		e->getMinMaxFeatures(&minValues, &maxValues);
		bool episodic = e->isEpisodic();

		// get max/min reward for the domain
		float rMax = 0.0;
		float rMin = -1.0;
		e->getMinMaxReward(&rMin, &rMax);
		float rRange = rMax - rMin;

		// Initializing the first task and the associated reward
		e->setReward(rTrain);
		e->setTask(taskTrain);

		// Construct agent here.
		Agent* agent;

		//Temporary
		float vTrain = 0.;

		if (strcmp(agentType, "modelbased") == 0 || strcmp(agentType, "rmax") || strcmp(agentType, "texplore")){
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
					history, vTrain, nTrain, tTrain,
					deptrans, reltrans, featPct, stochastic, episodic, batchFreq,
					rng);
			agent->setTrueEnv(e);
		}

		else {
			std::cerr << "ERROR: Invalid agent type" << endl;
			exit(-1);
		}

		int virtualSeed = 12;

		Random virtualRng(virtualSeed);
		Environment* virtualInfinite = new InfiniteBlocks(virtualRng, roomsize, stochastic, rTrain, taskTrain);
		virtualInfinite->setDebug(false);
		virtualInfinite->setVerbose(false);

		/*
		if (pretrain_steps>0){

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
					
					accumulated_RL_rewards[trainStep/eval_freq] += trial_RL_reward;
					accumulated_rewards_pick_red[trainStep/eval_freq] += trial_reward_pick_red;

					accumulated_rewards_pick_blue[trainStep/eval_freq] += trial_reward_pick_blue;

					accumulated_ALL_rewards[trainStep/eval_freq] += trial_ALL_reward;
					accumulated_MATCHING_rewards[trainStep/eval_freq] += trial_MATCHING_reward;
					accumulated_OPPOSITE_rewards[trainStep/eval_freq] += trial_OPPOSITE_reward;
					accumulated_RED_rewards[trainStep/eval_freq] += trial_RED_reward;

					save_results(act_success_rates, model_acc,
							reward_acc,
							accumulated_RL_rewards,
							accumulated_ALL_rewards,
							accumulated_MATCHING_rewards,
							accumulated_OPPOSITE_rewards,
							accumulated_RED_rewards,
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
		*/


		// STEP BY STEP DOMAIN
		if (!episodic){

			tutor_feedback t_feedback(0.,0., 0);
			occ_info_t info;

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
					if (rTrain==0){
						agent->setRewarding(false);
					}
					else{
						agent->setRewarding(true);
					}

					//e->setReward(rEval);
				}
				else {
					// next action
					a = agent->next_action(info.RL_reward, es, &avg_var_prop, &avg_nov_prop, &avg_reward_prop, &avg_sync_prop);
					info = e->apply(a);
					t_feedback = e->tutorAction();
					e->apply_tutor(t_feedback.action);
				}

				trial_RL_reward += info.RL_reward;
				trial_ALL_reward += info.ALL_reward;
				trial_MATCHING_reward += info.MATCHING_reward;
				trial_OPPOSITE_reward += info.OPPOSITE_reward;
				trial_RED_reward += info.RED_reward;

//				trial_tutor_reward_2 += t_feedback.reward;

				if (step % eval_freq == 0){

					std::cout << "Trial " << j << ",eval at step "<< trial_step+step << std::endl;

					//std::pair<float,float> errors = evaluation(virtualInfinite, agent, numactions, &rng, minValues, maxValues, rRange);


					accumulated_RL_rewards[(trial_step+step)/eval_freq] += trial_RL_reward;
					accumulated_ALL_rewards[(trial_step+step)/eval_freq] += trial_ALL_reward;
					accumulated_MATCHING_rewards[(trial_step+step)/eval_freq] += trial_MATCHING_reward;
					accumulated_OPPOSITE_rewards[(trial_step+step)/eval_freq] += trial_OPPOSITE_reward;
					accumulated_RED_rewards[(trial_step+step)/eval_freq] += trial_RED_reward;


					var_prop[(trial_step+step)/eval_freq] += avg_var_prop;
					nov_prop[(trial_step+step)/eval_freq] += avg_nov_prop;
					reward_prop[(trial_step+step)/eval_freq] += avg_reward_prop;
					sync_prop[(trial_step+step)/eval_freq] += avg_sync_prop;

				}

				if (step % 200 == 0 && step != 0){

					save_result(model_acc, "model_acc", rootPath);

					save_result(reward_acc, "reward_acc", rootPath);
					save_result(accumulated_RL_rewards, "accumulated_RL_rewards", rootPath);
					save_result(accumulated_ALL_rewards, "accumulated_ALL_rewards", rootPath);
					save_result(accumulated_MATCHING_rewards, "accumulated_MATCHING_rewards", rootPath);
					save_result(accumulated_OPPOSITE_rewards, "accumulated_OPPOSITE_rewards", rootPath);
					save_result(accumulated_RED_rewards, "accumulated_RED_rewards", rootPath);
					save_result(var_prop, "var_prop", rootPath);
					save_result(sync_prop, "sync_prop", rootPath);
					save_result(nov_prop, "nov_prop", rootPath);
					save_result(reward_prop, "reward_prop", rootPath);


				}

				if (step==stepsTrain) {
					if (rEval!=0){
						agent->setRewarding(true);
					}
					agent->setTutorBonus(tEval);
					agent->setNovelty(nEval);
					e->setTask(taskEval);
					e->setReward(rEval);
					if (resetQ){
						agent->forget();
						std::cout << "resetting Q"<<std::endl;
					}
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
		delete e;
	}

}



 // end main

