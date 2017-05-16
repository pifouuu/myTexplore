/** \file ExplorationModel.cc
    Implements the ExplorationModel class.
    Reward bonuses based on the variance in model predictions are described in: Hester and Stone, "Real Time Targeted Exploration in Large Domains", ICDL 2010.
    And intrinsic reward bonuses based on variance novelty as described in:
    Hester and Stone, "Intinrisically Motivated Model Learning for a Developing Curious Agent", ICDL 2012.
    \author Todd Hester
 */

#include "../../include/models/ExplorationModel.hh"


using namespace std;


ExplorationModel::ExplorationModel(MDPModel* innermodel, int modelType,
		int predType, int nModels,
		float m, int numactions,
		float rmax, float qmax, float rrange,
		int nfactors, float v, float n, float tutorBonus,
		const std::vector<float> &fmax,
		const std::vector<float> &fmin, Random rng):
		  modelType(modelType), predType(predType),
		  nModels(nModels),
		  M(m), numactions(numactions), rmax(rmax), qmax(qmax), rrange(rrange),
		  nfactors(nfactors), v(v), n(n), tutorBonus(tutorBonus), rng(rng)
{

	model = innermodel;

	MODEL_DEBUG = false; //true;

	cout << "Exploration Model, v: " << v << ", n: " << n << endl;

	featmax = fmax;
	featmin = fmin;

}


ExplorationModel::ExplorationModel(const ExplorationModel &em):
		modelType(em.modelType), predType(em.predType),
		nModels(em.nModels),
		M(em.M), numactions(em.numactions), rmax(em.rmax), qmax(em.qmax), rrange(em.rrange),
		nfactors(em.nfactors), v(em.v), n(em.n), tutorBonus(em.tutorBonus), rng(em.rng)
{
	model = em.model->getCopy();
	MODEL_DEBUG = em.MODEL_DEBUG;
	featmax = em.featmax;
	featmin = em.featmin;
	statespace = em.statespace;
}

ExplorationModel* ExplorationModel::getCopy(){
	ExplorationModel* copy = new ExplorationModel(*this);
	return copy;
}


ExplorationModel::~ExplorationModel() {
	delete model;
}

/*std::list<std::tuple<std::vector<float>, int, StateActionInfo>> ExplorationModel::eval(std::list<std::vector<float>> samples){
	return model->eval(samples);
}*/

void ExplorationModel::setTrueEnv(Environment* e){

	trueEnv = e;

}

void ExplorationModel::setTesting(bool mode){
	testMode = mode;
}

void ExplorationModel::setRewarding(bool val){
	model->setRewarding(val);
}

void ExplorationModel::setTutorBonus(float val){
	tutorBonus = val;
}

void ExplorationModel::setNovelty(float val){
	n = val;
}

bool ExplorationModel::updateWithExperiences(std::vector<experience> &instances){
	bool changed = model->updateWithExperiences(instances);
	bool visitChange = false;

	// keep track of which states we've been to for this mode
	for (unsigned i = 0; i < instances.size(); i++){

		if (n!=0.){
			std::vector<float> last2 = instances[i].s;
			last2.push_back(instances[i].act);
			bool retval = addStateToSet(last2);
			visitChange = visitChange || retval;
		}
	}

	return (changed || visitChange);
}


// update all the counts, check if model has changed
// stop counting at M
bool ExplorationModel::updateWithExperience(experience &e){
	//if (MODEL_DEBUG) cout << "updateWithExperience " << &last << ", " << act
	//        << ", " << &curr << ", " << reward << endl;


	bool changed = model->updateWithExperience(e);
	bool visitChange = false;
	bool tutorChange = false;

	if (n!=0.){
		std::vector<float> last2 = e.s;
		last2.push_back(e.act);
		bool retval = addStateToSet(last2);
		visitChange = visitChange || retval;
	}

	if (tutorBonus!=0.){
		std::vector<float> curTstate = trueEnv->getTstate();
		if (curTstate != lastTstate) {
			tutorChange = true;
		}
		lastTstate = curTstate;
	}

	return (changed || visitChange || tutorChange);
}


// calculate state info such as transition probs, known/unknown, reward prediction
float ExplorationModel::getStateActionInfo(const std::vector<float> &state, int act, StateActionInfo* retval){
	//if (MODEL_DEBUG) cout << "getStateActionInfo, " << &state <<  ", " << act << endl;

	retval->transitionProbs.clear();

	float conf = model->getStateActionInfo(state, act, retval);


	//cout << "state: " << state[0] << " act: " << act;

	if (MODEL_DEBUG)// || (conf > 0.0 && conf < 1.0))
		cout << "reward: " << retval->sumReward() << " conf: " << conf << endl;

	if (!testMode){

		// check exploration bonuses

		// small bonus for states far from visited states with same action
		if (n!=0.){
			std::vector<float> state2 = state;
			state2.push_back(act);
			float bonus = 0.;
			float featDist = getFeatDistToVisitedSA(state2);
			if (featDist > 0){
				// modify reward with proportional bonus of n
				bonus = featDist * n;
				if (MODEL_DEBUG){
					cout << "   State-Action novel state bonus, dist: " << featDist
							<< " n: " << n << ", bonus, " << bonus << endl;
				}
			}
			retval->novBonus += bonus;
		}

		// use some % of v if we're doing continuous bonus
		if (v!=0.){
			if (conf < 1.0){
				// percent of conf
				float bonus = (1.0-conf)*v;
				retval->varBonus += bonus;
				if (MODEL_DEBUG){
					cout << "   State-Action continuous bonus conf: "
							<< conf
							<< ", using v*(1-conf): "
							<< bonus << endl;
				}
			}
		}

		if (tutorBonus!=0.){
			bool sync = trueEnv->isSyncTutor(state);
			float bonus = 0;
			if (sync){
				bonus = tutorBonus;
				if (MODEL_DEBUG){
					cout << "   State-Action tutor bonus: " << endl;
				}
			}
			retval->syncBonus += bonus;
		}
	}

	float sumReward = retval->sumReward();

	if (MODEL_DEBUG)
		cout << "   Conf: " << conf << "   Avg reward: " << sumReward << endl;
	if (isnan(sumReward))
		cout << "ERROR: Model returned reward of NaN" << endl;

	return true;

}

// add state to set (if its not already in it)
bool ExplorationModel::addStateToSet(const std::vector<float> &s){
	std::pair<std::set<std::vector<float> >::iterator, bool> retval;
	retval = statespace.insert(s);
	return retval.second;
}


// check if state is in set (so we know if we've visited it)
bool ExplorationModel::checkForState(const std::vector<float> &s){
	return (statespace.count(s) == 1);
}

// get distance in feature space from this state to one we've visited
float ExplorationModel::getFeatDistToVisitedSA(const std::vector<float> &s){

	// if we've visited this exact s,a then dist is 0
	if (checkForState(s)){
		return 0;
	}

	// otherwise go through all states and find minimum distance
	float maxDist = 0;
	unsigned nfeats = s.size()-1;
	std::vector<float> featRange(nfeats, 0);
	for (unsigned i = 0; i < nfeats; i++){
		featRange[i] = featmax[i] - featmin[i];
		maxDist += 1.0;//featmax[i] - featmin[i];

		//cout << "feat " << i << " diff: " << (featmax[i] - featmin[i]) << " max: " << maxDist << endl;
	}

	float minDist = maxDist;//nfeats;
	unsigned actionIndex = nfeats;

	for (std::set<std::vector<float> >::iterator i = statespace.begin(); i != statespace.end(); i++){
		// ignore if not the same action
		if (s[actionIndex] != (*i)[actionIndex]) continue;

		// otherwise, sum all features that are different
		float count = 0;
		for (unsigned j = 0; j < nfeats; j++){
			// distance based on magnitude of feature difference
			// normalize by feature range
			count += fabs(s[j] - (*i)[j]) / featRange[j];
		}
		if (count < minDist) minDist = count;

	}

	return (float)minDist/(float)nfeats;

}


