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

	Random &rng;

	int height;
	int width;

	int nbRedBlocks;
	int nbBlueBlocks;

	enum color{
		RED,
		BLUE
	};

	struct block_t{
			float ns;
			float ew;
			float color;
			float is_in_robot_hand = false;
			float is_in_blue_box = false;
			float is_in_red_box = false;
		};

	struct agent_t{
		float ns;
		float ew;
		float block_hold = -1;
		float eye_ns;
		float eye_ew;
		float is_synchronous = false;
	};

	struct red_box_t{
		float ns;
		float ew;
		float color = RED;
	};

	struct blue_box_t{
		float ns;
		float ew;
		float color = BLUE;
	};

	struct tutor_t{
		float eye_ns;
		float eye_ew;
	};

	agent_t agent;
	blue_box_t blue_box;
	red_box_t red_box;
	std::vector<block_t> blocks;
	tutor_t tutor;

	std::map<std::string, int> actions;
	int num_actions;

	std::random_device rd;
	std::mt19937 g(rd());

	std::vector<float> s;
	const std::vector<float> &sensation() const;
	int applyNoise(int action);
	std::vector<int> find_red_block_under_hand();
	std::vector<int> find_blue_block_under_hand();
	bool terminal() const;
	bool eye_hand_sync();


};



#endif /* INCLUDE_ENVS_BLOCKROOM_HH_ */
