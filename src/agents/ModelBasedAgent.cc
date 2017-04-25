/** \file ModelBasedAgent.cc
    Implements the ModelBasedAgent class
    \author Todd Hester
 */

#include "../../include/agents/ModelBasedAgent.hh"
#include <algorithm>

#include <sys/time.h>

// planners
#include "../../include/planners/ValueIteration.hh"
#include "../../include/planners/PolicyIteration.hh"
#include "../../include/planners/PrioritizedSweeping.hh"
#include "../../include/planners/ETUCT.hh"
#include "../../include/planners/ParallelETUCT.hh"
#include "../../include/planners/PO_ETUCT.hh"
#include "../../include/planners/PO_ParallelETUCT.hh"
#include "../../include/planners/MBS.hh"

// models
#include "../../include/models/RMaxModel.hh"
#include "../../include/models/FactoredModel.hh"
#include "../../include/models/ExplorationModel.hh"

using namespace std;

ModelBasedAgent::ModelBasedAgent(int numactions, int numattentions, float gamma,
		float rmax, float rrange,
		int modelType,	int exploreType,
		int predType, int nModels, int plannerType,
		float epsilon, float lambda, float MAX_TIME,
		float m, const std::vector<float> &featmin,
		const std::vector<float> &featmax,
		std::vector<int> nstatesPerDim, int history, float v, float n, float tutorBonus,
		bool depTrans, float featPct, bool stoch, bool episodic, bool rewarding, int batchFreq,
		Random rng):
		  featminEnv(featmin), featmaxEnv(featmax),
		  numactions(numactions), numattentions(numattentions), gamma(gamma), rmax(rmax), rrange(rrange),
		  qmax(rmax/(1.0-gamma)), depTrans(depTrans),
		  modelType(modelType), exploreType(exploreType),
		  predType(predType), nModels(nModels), plannerType(plannerType),
		  epsilon(epsilon), lambda(lambda), MAX_TIME(MAX_TIME),
		  M(m), statesPerDim(nstatesPerDim), history(history), v(v), n(n), tutorBonus(tutorBonus),
		  featPct(featPct),
		  stoch(stoch), episodic(episodic), rewarding(rewarding), rng(rng), batchFreq(batchFreq), internalState(numattentions)
{

	if (statesPerDim[0] > 0){
		cout << "MBA: Planner will use states discretized by various amounts per dim with continuous model" << endl;
	}

	initParams();

}



ModelBasedAgent::ModelBasedAgent(int numactions, int numattentions, float gamma,
		float rmax, float rrange,
		int modelType,	int exploreType,
		int predType, int nModels, int plannerType,
		float epsilon, float lambda, float MAX_TIME,
		float m, const std::vector<float> &featmin,
		const std::vector<float> &featmax,
		int nstatesPerDim, int history, float v, float n, float tutorBonus,
		bool depTrans, float featPct,
		bool stoch, bool episodic, bool rewarding, int batchFreq, Random rng):
		  featminEnv(featmin), featmaxEnv(featmax),
		  numactions(numactions), numattentions(numattentions), gamma(gamma), rmax(rmax), rrange(rrange),
		  qmax(rmax/(1.0-gamma)), depTrans(depTrans),
		  modelType(modelType), exploreType(exploreType),
		  predType(predType), nModels(nModels), plannerType(plannerType),
		  epsilon(epsilon), lambda(lambda), MAX_TIME(MAX_TIME),
		  M(m), statesPerDim(featmin.size(),nstatesPerDim),  history(history), v(v), n(n), tutorBonus(tutorBonus),
		  featPct(featPct),
		  stoch(stoch), episodic(episodic), rewarding(rewarding), rng(rng), batchFreq(batchFreq), internalState(numattentions)
{

	if (statesPerDim[0] > 0){
		cout << "MBA: Planner will use states discretized by " << statesPerDim[0] << " with continuous model" << endl;
	}

	initParams();

}


void ModelBasedAgent::initParams(){

	nstates = 0;
	nactions = 0;

	model = NULL;
	planner = NULL;

	modelUpdateTime = 0.0;
	planningTime = 0.0;
	actionTime = 0.0;

	modelChanged = false;

	TIMEDEBUG = false; //true;
	AGENTDEBUG = false;
	ACTDEBUG = false;//true;
	SIMPLEDEBUG = true; //true; //false; //true;

	actions[std::string("LOOK_RED_BOX")] = 2;
	actions[std::string("LOOK_BLUE_BOX")] = 3;
	actions[std::string("LOOK_BLUE_BLOCKS")] = 1;
	actions[std::string("LOOK_RED_BLOCKS")] = 0;

	featmin = std::vector<float>(0,numattentions);
	featmin.insert(featmin.end(), featminEnv.begin(), featminEnv.end());

	featmax = std::vector<float>(1, numattentions);
	featmax.insert(featmax.end(), featmaxEnv.begin(), featmaxEnv.end());

	int first_attention = rng.uniformDiscrete(0, numattentions-1);
	internalState[first_attention]=1;

	for (int i=0;i<featmin.size();i++){
		if (i<numattentions) relTrans.push_back(0);
		else relTrans.push_back(1);;
	}

	// check
	if (qmax <= 0.1 && (exploreType == TWO_MODE_PLUS_R ||
			exploreType == CONTINUOUS_BONUS_R ||
			exploreType == CONTINUOUS_BONUS ||
			exploreType == THRESHOLD_BONUS_R)) {
		std::cerr << "For this exploration type, rmax needs to be an additional positive bonus value, not a replacement for the q-value" << endl;
		exit(-1);
	}

	if (exploreType == TWO_MODE || exploreType == TWO_MODE_PLUS_R){
		std::cerr << "This exploration type does not work in this agent." << endl;
		exit(-1);
	}

	seeding = false;

	if (SIMPLEDEBUG)
		cout << "qmax: " << qmax  << endl;

}

ModelBasedAgent::~ModelBasedAgent() {
	delete planner;
	delete model;
	featminEnv.clear();
	featmaxEnv.clear();
	prevstate.clear();
}

void ModelBasedAgent::setRewarding(bool val){
	model->setRewarding(val);
}

occ_info_t ModelBasedAgent::apply(int action){
	float reward = 0;
	float tutor_reward = 0;
	bool success = false;

	std::fill(internalState.begin(), internalState.end(),0);
	internalState[action] = 1;
	return occ_info_t(reward, success, 0, 0, tutor_reward);
}

occ_info_t ModelBasedAgent::virtualApply(std::vector<float> &attention, int action){
	float reward = 0;
	float tutor_reward = 0;
	bool success = false;

	std::fill(attention.begin(), attention.end(),0);
	attention[action] = 1;
	return occ_info_t(reward, success, 0, 0, tutor_reward);
}

std::vector<float> ModelBasedAgent::generateSample(){
	std::vector<float> res(numattentions,0);
	int o = rng.uniformDiscrete(0, numattentions-1);
	res[o]=1;
	return res;
}

const std::vector<float> &ModelBasedAgent::attention() const {
	return internalState;
}

bool ModelBasedAgent::train_only(experience e){
	if (model == NULL)
		initModel(e.s.size());
	return (model->updateWithExperience(e));
}

bool ModelBasedAgent::train_only_many(std::vector<experience> e){
	if (model == NULL)
		initModel(e[0].s.size());
	return (model->updateWithExperiences(e));
}

void ModelBasedAgent::getMinMaxFeatures(std::vector<float> *minFeat,
		std::vector<float> *maxFeat){

	minFeat->resize(numattentions, 0);
	maxFeat->resize(numattentions, 1);

}


std::tuple<std::vector<float>,float,float> ModelBasedAgent::pred(std::vector<float> & s, int act){
	StateActionInfo sa_info;
	model->setTesting(true);
	float conf = model->getStateActionInfo(s, act, &sa_info);
	model->setTesting(false);
	std::map<std::vector<float>, float> preds = sa_info.transitionProbs;
	auto pr = std::max_element
			(
					std::begin(preds), std::end(preds),
					[] (const std::pair<std::vector<float> , float> & p1,
							const std::pair<std::vector<float> , float> & p2) {
		return p1.second < p2.second;
	}
			);
	return std::make_tuple(pr->first, sa_info.envReward,sa_info.termProb);
}

int ModelBasedAgent::first_action(const std::vector<float> &s, float* avg_var_prop, float* avg_nov_prop, float* avg_reward_prop, float* avg_sync_prop) {
	if (AGENTDEBUG) cout << "first_action(s)" << endl;

	if (model == NULL)
		initModel(s.size());

	planner->setFirst();

	// in case we didn't do it after seeding
	if (plannerType == PARALLEL_ET_UCT || plannerType == PAR_ETUCT_ACTUAL)
		planner->planOnNewModel();

	// choose an action
	int act = chooseAction(s, avg_var_prop, avg_nov_prop, avg_reward_prop, avg_sync_prop);

	// save curr state/action for next time
	saveStateAndAction(s, act);

	if (ACTDEBUG){
		cout << "Took action " << act << " from state "
				<< (s)[0];
		for (unsigned i = 1; i < s.size(); i++){
			cout << "," << (s)[i];
		}
		cout << endl;
	}

	// return that action
	return act;

}

int ModelBasedAgent::next_action(float r, const std::vector<float> &s, float* avg_var_prop, float* avg_nov_prop, float* avg_reward_prop, float* avg_sync_prop) {
	if (AGENTDEBUG) {
		cout << "next_action(r = " << r
				<< ", s = " << &s << ")" << endl;
	}

	if (SIMPLEDEBUG) cout << "Got Reward " << r << endl;

	// update our models
	// this is where we possibly plan again if model changes
	updateWithNewExperience(prevstate, s, prevact, r, false);

	// choose an action
	int act = chooseAction(s, avg_var_prop, avg_nov_prop, avg_reward_prop, avg_sync_prop);

	// save curr state/action for next time
	saveStateAndAction(s, act);

	if (ACTDEBUG){
		cout << "Took action " << act << " from state "
				<< (s)[0];
		for (unsigned i = 1; i < s.size(); i++){
			cout << "," << (s)[i];
		}
		cout << endl;
	}

	// return that action
	return act;

}

void ModelBasedAgent::last_action(float r) {
	if (AGENTDEBUG) cout << "last_action(r = " << r
			<< ")" << endl;

	if (AGENTDEBUG) cout << "Got Reward " << r;

	// update our models
	// this is where we possibly plan again if model changes
	updateWithNewExperience(prevstate, prevstate, prevact, r, true);

}



/////////////////////////////
// Functional functions :) //
/////////////////////////////


void ModelBasedAgent::initModel(int nfactors){
	if ( AGENTDEBUG) cout << "initModel nFactors: " << nfactors << endl;

	bool needConf =
			(exploreType != NO_EXPLORE && exploreType != EXPLORE_UNKNOWN &&
					exploreType != EPSILONGREEDY && exploreType != UNVISITED_BONUS &&
					exploreType != UNVISITED_ACT_BONUS);

	std::vector<float> featRange(featmax.size(), 0);
	for (unsigned i = 0; i < featmax.size(); i++){
		featRange[i] = featmax[i] - featmin[i];
		cout << "feature " << i << " has range " << featRange[i] << endl;
	}
	cout << "reward range: " << rrange << endl;

	float treeRangePct = 0.0001;

	// m5 tree
	if (modelType == M5MULTI || modelType == M5SINGLE ||
			modelType == M5ALLMULTI || modelType == M5ALLSINGLE ||
			modelType == ALLM5TYPES){
		treeRangePct = 0.0001;
	}

	// just to ensure the diff models are on different random values
	for (int i = 0; i < modelType; i++){
		rng.uniform(0, 1);
	}

	// 0 - traditional rmax model (unknown until m visits, then ML)
	if (modelType == RMAX) {
		//    model = new RMaxModel(M, numactions, rng);
	}

	// any tree or stump will be mdptree
	else if (modelType == C45TREE || modelType == STUMP ||
			modelType == M5MULTI || modelType == M5SINGLE ||
			modelType == M5ALLMULTI || modelType == M5ALLSINGLE ||
			modelType == ALLM5TYPES ||
			modelType == LSTMULTI || modelType == LSTSINGLE ||
			modelType == GPREGRESS || modelType == GPTREE){

		model = new FactoredModel(0,numactions, relTrans, M, modelType, predType, nModels, treeRangePct, featRange,
				rrange, needConf, depTrans, featPct, stoch, episodic, rewarding, rng);
	}

	/*
  else if (modelType == GPREGRESS){
    model = new GPmdp(0, numactions, relTrans, rng);
  }
	 */

	// pass model into exploration model wrapper model
	if (exploreType != NO_EXPLORE && exploreType != EPSILONGREEDY){
		MDPModel* m2 = model;

		model = new ExplorationModel(m2, modelType, exploreType,
				predType, nModels, M, numactions,
				rmax, qmax, rrange, nfactors, v, n, tutorBonus,
				featmax, featmin, rng);

		model->setTrueEnv(trueEnv);
		model->setTesting(false);
	}

	initPlanner();
	planner->setModel(model);
	planner->setTrueEnv(trueEnv);

}

void ModelBasedAgent::setTrueEnv(Environment* e){

	trueEnv = e;

}

void ModelBasedAgent::initPlanner(){
	if (AGENTDEBUG) cout << "InitPlanner type: " << plannerType << endl;

	int max_path = 20; //500;

	// init planner based on typ
	/*
  if (plannerType == VALUE_ITERATION){
    planner = new ValueIteration(numactions, gamma, 10, 1, modelType, featmax, featmin, statesPerDim, rng);
  }

  if (plannerType == MBS_VI){
    planner = new MBS(numactions, gamma, 500000, 10.0, modelType, featmax, featmin, statesPerDim, history, rng);
  }
  else if (plannerType == POLICY_ITERATION){
    planner = new PolicyIteration(numactions, gamma, 500000, 10.0, modelType, featmax, featmin, statesPerDim, rng);
  }
  else if (plannerType == PRI_SWEEPING){
    planner = new PrioritizedSweeping(numactions, gamma, 10.0, true, modelType, featmax, featmin, rng);
  }
  else if (plannerType == MOD_PRI_SWEEPING){
    planner = new PrioritizedSweeping(numactions, gamma, 10.0, false, modelType, featmax, featmin, rng);
  }*/
	if (plannerType == ET_UCT){
		planner = new ETUCT(numactions, gamma, rrange, lambda, 500000, MAX_TIME, max_path, modelType, featmax, featmin, statesPerDim, false, history, rng);
	}
	/* else if (plannerType == POMDP_ETUCT){
    planner = new PO_ETUCT(numactions, gamma, rrange, lambda, 500000, MAX_TIME, max_path, modelType, featmax, featmin, statesPerDim, true, history, rng);
  }
  else if (plannerType == POMDP_PAR_ETUCT){
    planner = new PO_ParallelETUCT(numactions, gamma, rrange, lambda, 500000, MAX_TIME, max_path, modelType, featmax, featmin, statesPerDim, true, history, rng);
  }*/
	else if (plannerType == ET_UCT_ACTUAL){
		planner = new ETUCT(numactions, gamma, rrange, lambda, 500000, MAX_TIME, max_path, modelType, featmax, featmin, statesPerDim, true, history, rng);
	}
	/*else if (plannerType == PARALLEL_ET_UCT){
    planner = new ParallelETUCT(numactions, gamma, rrange, lambda, 500000, MAX_TIME, max_path, modelType, featmax, featmin, statesPerDim, false, history, rng);
  }
  else if (plannerType == PAR_ETUCT_ACTUAL){
    planner = new ParallelETUCT(numactions, gamma, rrange, lambda, 500000, MAX_TIME, max_path, modelType, featmax, featmin, statesPerDim, true, history, rng);
  }*/
	/*else if (plannerType == ET_UCT_L1){
    planner = new ETUCT(numactions, gamma, rrange, 1.0, 500000, MAX_TIME, max_path, modelType, featmax, featmin, statesPerDim, false, history, rng);
  }*/
	else {
		std::cerr << "ERROR: invalid planner type: " << plannerType << endl;
		exit(-1);
	}

}

void ModelBasedAgent::updateWithNewExperience(const std::vector<float> &last, 
		const std::vector<float> &curr,
		int lastact, float reward,
		bool terminal){
	if (AGENTDEBUG) cout << "updateWithNewExperience(last = " << &last
			<< ", curr = " << &curr
			<< ", lastact = " << lastact
			<< ", r = " << reward
			<< ", t = " << terminal
			<< ")" << endl;

	double initTime = 0;
	double timeTwo = 0;
	double timeThree = 0;

	if (model == NULL)
		initModel(last.size());

	// update our models and see if they change
	if (false || TIMEDEBUG) initTime = getSeconds();

	modelChanged = planner->updateModelWithExperience(last, lastact, curr, reward, terminal) || modelChanged;

	if (false || TIMEDEBUG) timeTwo = getSeconds();

	if (AGENTDEBUG) cout << "Agent Added exp: " << modelChanged << endl;

	// tell the planner to update with the updated model
	if ((modelChanged && (!seeding || modelType == RMAX)
			&& (nactions % batchFreq == 0))){
		planner->planOnNewModel();
		modelChanged = false;
	}

	if (TIMEDEBUG){

		timeThree = getSeconds();

		planningTime += (timeThree-timeTwo);
		modelUpdateTime += (timeTwo - initTime);

		if (nactions % 10 == 0){
			cout << nactions
					<< " UpdateModel " << modelUpdateTime/ (float)nactions
					<< " createPolicy " << planningTime/(float)nactions << endl;

		}
	}



}

/*void ModelBasedAgent::evaluate_model(){
	planner->evaluate_model();
}*/

int ModelBasedAgent::chooseAction(const std::vector<float> &s, float* avg_var_prop, float* avg_nov_prop, float* avg_reward_prop, float* avg_sync_prop){
	if (AGENTDEBUG) cout << "chooseAction(s = " << &s
			<< ")" << endl;

	double initTime = 0;
	double timeTwo = 0;

	// get action to take from planner
	if (TIMEDEBUG) initTime = getSeconds();
	int act = planner->getBestAction(s, avg_var_prop, avg_nov_prop, avg_reward_prop, avg_sync_prop);
	if (TIMEDEBUG) {
		timeTwo = getSeconds();
		planningTime += (timeTwo - initTime);
	}

	if (rng.bernoulli(epsilon)){
		//if (true) cout << "Random action" << endl;
		act = rng.uniformDiscrete(0, numactions-1);
	}

	/* if (tutorType == BONUS && rng.bernoulli(tutorBonus)){
	  act = LOOK_TUTOR;
  }*/

	if (SIMPLEDEBUG){
		cout << endl << "Action " << nactions
				<< ": State " << (s)[0];
		for (unsigned i = 1; i < s.size(); i++){
			cout << "," << (s)[i];
		}
		cout << ", Took action " << act << ", ";
	}

	nactions++;

	// return index of action
	return act;
}

void ModelBasedAgent::saveStateAndAction(const std::vector<float> &s, int act){
	if (AGENTDEBUG) cout << "saveStateAndAction(s = " << &s
			<< ", act = " << act
			<< ")" << endl;
	prevstate = s;
	prevact = act;

}





double ModelBasedAgent::getSeconds(){
	struct timezone tz;
	timeval timeT;
	gettimeofday(&timeT, &tz);
	return  timeT.tv_sec + (timeT.tv_usec / 1000000.0);
}


void ModelBasedAgent::seedExp(std::vector<experience> seeds){
	if (AGENTDEBUG) cout << "seed experiences" << endl;

	if (seeds.size() == 0) return;

	if (model == NULL)
		initModel(seeds[0].s.size());

	seeding = true;
	planner->setSeeding(true);

	// for each seeding experience, update our model
	for (unsigned i = 0; i < seeds.size(); i++){
		experience e = seeds[i];

		// update our models
		// this is where we possibly run qmax again if model(s) change
		updateWithNewExperience(e.s, e.next, e.act, e.reward, e.terminal);

		/*
    cout << "Seeding with experience " << i << endl;
    cout << "last: " << (*curr)[0] << ", " << (*curr)[1] << ", " 
	 << (*curr)[2] << endl;
    cout << "act: " << e.act << " r: " << e.reward << endl;
    cout << "next: " << (*next)[0] << ", " << (*next)[1] << ", " 
	 << (*next)[2] << ", " << e.terminal << endl;
		 */

	}

	seeding = false;
	planner->setSeeding(false);

	if (seeds.size() > 0)
		planner->planOnNewModel();

}


void ModelBasedAgent::setDebug(bool d){
	AGENTDEBUG = d;
}

void ModelBasedAgent::savePolicy(const char* filename){
	planner->savePolicy(filename);
}



void ModelBasedAgent::logValues(ofstream *of, int xmin, int xmax, int ymin, int ymax){

	// call planner
	if (plannerType == PARALLEL_ET_UCT){
		//    ((ParallelETUCT*)planner)->logValues(of, xmin, xmax, ymin, ymax);
	}
	if (plannerType == ET_UCT){
		((ETUCT*)planner)->logValues(of, xmin, xmax, ymin, ymax);
	}

}
