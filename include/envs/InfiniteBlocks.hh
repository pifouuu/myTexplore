/*
 * InfiniteBlocks.hh
 *
 *  Created on: 14 févr. 2017
 *      Author: pierre
 */

#ifndef INCLUDE_ENVS_InfiniteBlocks_HH_
#define INCLUDE_ENVS_InfiniteBlocks_HH_

#include "../common/Random.h"
#include "../common/core.hh"
#include <random>
#include <algorithm>



class InfiniteBlocks: public Environment {
public:
	// Constructor

	InfiniteBlocks(Random &rand, int size, bool with_tutor, bool stochastic, float finalReward);

	/* InfiniteBlocks is not supposed to become a base class so no need
	 * for a virtual constructor ? TO be changed if so.
	 */
	~InfiniteBlocks();


	void apply_tutor(int action);
	bool terminal() const;
	void reset();
	int getNumActions();
	int getNumObjects();
//	int trueBestAction(std::vector<float> attention);
	std::vector<float> generateSample();
	int getNumTutorActions();
	std::vector<float> generate_state();
	float getEuclidianDistance(std::vector<float> & s1, std::vector<float> & s2,
			std::vector<float> minValues, std::vector<float>maxValues);
	std::pair<std::vector<float>,float> getMostProbNextState(std::vector<float> s, int action);
	void getMinMaxFeatures(std::vector<float> *minFeat, std::vector<float> *maxFeat);

	void getMinMaxReward(float* minR, float* maxR);
	bool isEpisodic(){ return false;}

	friend std::ostream &operator<<(std::ostream &out, const InfiniteBlocks &InfiniteBlocks);

	/** Prints the current map. */
	void print_map(std::vector<float> attention) const;

	int size;
	bool stochastic;
	Random &rng;
	bool WITH_TUTOR;
	bool tutor_attentive = true;
	float finalReward;

	int get_blocks_in() const;
	int get_blocks_right() const;
	void tutorStop();
	position getAttDir(std::vector<float> attention) const;

	std::default_random_engine engine;



	occ_info_t apply(int action, const std::vector<float> &attention);
	float getStateActionInfoError(std::vector<float> s, std::vector<StateActionInfo> preds);

	float agent_ns;
	float agent_ew;
	float blue_block_hold;
	float red_block_hold;

	std::vector<position> objects;

	float* red_blocks_ns;
	float* blue_blocks_ns;
	float* red_blocks_ew;
	float* blue_blocks_ew;
	float* red_box_ns;
	float* red_box_ew;
	float* blue_box_ns;
	float* blue_box_ew;

	std::vector<float> t_state;
	float* tutor_eye_ns;
	float* tutor_eye_ew;

	int red_box_count_red;
	int blue_box_count_red;
	int red_box_count_blue;
	int blue_box_count_blue;

	std::map<std::string, int> tutor_actions;
	std::map<std::string, int> actions;
	int numactions;
	int num_tutor_actions;


	std::map<int, std::string> action_names;

	std::map<int, std::string> get_action_names();

	const std::vector<float> &sensation() const;
	int applyNoise(int action);
	std::vector<std::pair<int,int>> get_nearby_pos(int, int);
	bool isSyncTutor(std::vector<float>) const;
	std::vector<int> find_red_block_under_hand();
	std::vector<int> find_blue_block_under_hand();
	std::vector<int> find_block_under_eye();
	bool eye_hand_sync(std::vector<float> attention);
	void setDebug(bool b);
	void setVerbose(bool b);
	tutor_feedback tutorAction();


	bool BRDEBUG = false;
	bool IS_REAL = true;
	bool NOPICKBACK = true;
	int numstep;
	std::map<int, std::list<int>> actions_occurences;

};



#endif /* INCLUDE_ENVS_InfiniteBlocks_HH_ */
