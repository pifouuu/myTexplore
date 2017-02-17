/*
 * BlockRoom.hh
 *
 *  Created on: 14 févr. 2017
 *      Author: pierre
 */

#ifndef INCLUDE_ENVS_BLOCKROOM_HH_
#define INCLUDE_ENVS_BLOCKROOM_HH_

#include "../common/Random.h"
#include "../common/core.hh"
#include <random>
#include <algorithm>



class BlockRoom: public Environment {
public:
	// Constructor
	BlockRoom(Random &rand);

	/* Blockroom is not supposed to become a base class so no need
	 * for a virtual constructor ? TO be changed if so.
	 */
	~BlockRoom();

	float apply(int action);
	bool terminal();
	void reset();
	int getNumActions();
	void getMinMaxFeatures(std::vector<float> *minFeat, std::vector<float> *maxFeat);

	void getMinMaxReward(float* minR, float* maxR);

	friend std::ostream &operator<<(std::ostream &out, const BlockRoom &blockroom);

	/** Prints the current map. */
	void print_map() const;

	Random &rng;
	bool LWDEBUG;
	int height;
	int width;

	int nbRedBlocks;
	int nbBlueBlocks;

	enum color{
		RED,
		BLUE
	};

	std::default_random_engine engine;


	struct block_t{
		float* ns;
		float* ew;
		float* color;
		float* is_in_robot_hand;
		float* is_in_blue_box;
		float* is_in_red_box;
		block_t(float* a, float* b, float* c, float* d,
				float* e, float* f){
			ns = a;
			ew = b;
			color = c;
			is_in_robot_hand = d;
			is_in_blue_box = e;
			is_in_red_box = f;
		}
	};

	float& agent_ns;
	float& agent_ew;
	float& block_hold;
	float& agent_eye_ns;
	float& agent_eye_ew;
	float& red_box_ns;
	float& red_box_ew;
	float& blue_box_ns;
	float& blue_box_ew;
	float& tutor_eye_ns;
	float& tutor_eye_ew;
	std::vector<block_t> blocks;

	std::map<std::string, int> actions;
	int num_actions;

	std::vector<float> s;
	const std::vector<float> &sensation() const;
	int applyNoise(int action);
	std::vector<int> find_red_block_under_hand();
	std::vector<int> find_blue_block_under_hand();
	bool terminal() const;
	bool eye_hand_sync();



};



#endif /* INCLUDE_ENVS_BLOCKROOM_HH_ */
