/*
 * BlockRoom.cc
 *
 *  Created on: 14 févr. 2017
 *      Author: pierre
 */

#include "../../include/envs/BlockRoom.hh"

BlockRoom::BlockRoom(Random &rand):
	height(10),
	width(10),
	nbRedBlocks(2),
	nbBlueBlocks(3),
	rng(rand),
	s(12+6*(nbBlueBlocks+nbBlueBlocks))
{
	/* Agent and item attributes, actions */
	agent.ew = rng.uniformDiscrete(0, width-1);
	agent.ns = rng.uniformDiscrete(0, height-1);

	int cnt_actions = 0;
	actions[std::string("NORTH")] = cnt_actions++;
	actions[std::string("SOUTH")] = cnt_actions++;
	actions[std::string("EAST")] = cnt_actions++;
	actions[std::string("WEST")] = cnt_actions++;
	actions[std::string("PICK_BLUE")] = cnt_actions++;
	actions[std::string("PICK_RED")] = cnt_actions++;
	actions[std::string("PUT_DOWN")] = cnt_actions++;
	actions[std::string("PUT_IN")] = cnt_actions++;
	actions[std::string("LOOK_TUTOR")] = cnt_actions++;
	actions[std::string("LOOK_RED_BOX")] = cnt_actions++;
	actions[std::string("LOOK_BLUE_BOX")] = cnt_actions++;

	agent.eye_ew = rng.uniformDiscrete(0, width-1);
	agent.eye_ns = rng.uniformDiscrete(0, height-1);
	tutor.eye_ew = rng.uniformDiscrete(0, width -1);
	tutor.eye_ns = rng.uniformDiscrete(0, height-1);
	red_box.ew = rng.uniformDiscrete(0, width-1);
	red_box.ns = rng.uniformDiscrete(0, height-1);
	do {
		blue_box.ew = rng.uniformDiscrete(0, width-1);
		blue_box.ns = rng.uniformDiscrete(0, height-1);
	} while (red_box.ew==blue_box.ew && blue_box.ns==blue_box.ns);


	s = {
				agent.ew,
				agent.ns,
				agent.block_hold,
				agent.eye_ew,
				agent.eye_ns,
				agent.is_synchronous,
				tutor.eye_ew,
				tutor.eye_ns,
				red_box.ew,
				red_box.ns,
				blue_box.ew,
				blue_box.ns
		};

	std::vector<int> x(height*width-1);
	std::iota(x.begin(), x.end(), 0);

	std::shuffle(x.begin(), x.end(), g);

	for (std::vector<int>::iterator it = x.begin();
			it != x.begin()+nbRedBlocks;++it){
		int i = it-x.begin();
		blocks.push_back(block_t());
		blocks[i].color = RED;
		blocks[i].ew = (*it % width);
		blocks[i].ns = (*it / width);
		std::string name = "LOOK_RED_BLOCK_";
		name += std::to_string(i);
		actions[name]= cnt_actions++;
		s.push_back(blocks[i].ew);
		s.push_back(blocks[i].ns);
		s.push_back(blocks[i].color);
	}

	for (std::vector<int>::iterator it = x.begin()+nbRedBlocks;
			it != x.begin()+nbRedBlocks+nbBlueBlocks;++it){
		int i = it-(x.begin()+nbRedBlocks);
		blocks.push_back(block_t());
		blocks[i].color = BLUE;
		blocks[i].ew = (*it % width);
		blocks[i].ns = (*it / width);
		std::string name = "LOOK_BLUE_BLOCK_";
		name += std::to_string(i);
		actions[name]= cnt_actions++;
		s.push_back(blocks[i].ew);
		s.push_back(blocks[i].ns);
		s.push_back(blocks[i].color);
	}

	num_actions = cnt_actions;
}

BlockRoom::~BlockRoom() {}

const std::vector<float> &BlockRoom::sensation() const {
	return s;
}

bool BlockRoom::terminal() const {
	return false;
}

int BlockRoom::applyNoise(int action){
  switch(action) {
  case actions["NORTH"]:
  	  return rng.bernoulli(0.9) ? action : (rng.bernoulli(0.5) ? actions["EAST"] : actions["WEST"]);
  case actions["SOUTH"]:
    return rng.bernoulli(0.9) ? action : (rng.bernoulli(0.5) ? actions["EAST"] : actions["WEST"]);
  case actions["EAST"]:
	  return rng.bernoulli(0.9) ? action : (rng.bernoulli(0.5) ? actions["NORTH"] : actions["SOUTH"]);
  case actions["WEST"]:
    return rng.bernoulli(0.9) ? action : (rng.bernoulli(0.5) ? actions["NORTH"] : actions["SOUTH"]);
  default:
    return action;
  }
}

std::vector<int> BlockRoom::find_red_block_under_hand() {
	std::vector<int> l;
	for (std::vector::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (it->ns==agent.ns && it->ew==agent.ew && it->color==RED){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}

std::vector<int> BlockRoom::find_blue_block_under_hand() {
	std::vector<int> l;
	for (std::vector::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (it->ns==agent.ns && it->ew==agent.ew && it->color==BLUE){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}

bool BlockRoom::eye_hand_sync(){
	return ((BlockRoom::agent.eye_ns==BlockRoom::agent.ns)
			&& (BlockRoom::agent.eye_ew==BlockRoom::agent.ew));
}

float BlockRoom::apply(int action){
	float reward = 0.;

	if (action==actions["NORTH"]) {
		if (agent.ns < height-1) {agent.ns++;}
	}
	if (action==actions["SOUTH"]) {
		if (agent.ns > 0) {agent.ns--;}
	}
	if (action==actions["EAST"]) {
		if (agent.ew < width-1) {agent.ew++;}
	}
	if (action==actions["WEST"]) {
		if (agent.ew > 0) {agent.ew--;}
	}
	if (action==actions["PICK_BLUE"]) {
		if (agent.block_hold==-1 && eye_hand_sync()) {
			std::vector<int> blue_blocks_under = find_blue_block_under_hand();
			if (!blue_blocks_under.empty()) {
				std::shuffle(blue_blocks_under.begin(), blue_blocks_under.end(), g);
				int idx = blue_blocks_under.pop_back();
				blocks[idx].is_in_robot_hand = true;
				blocks[idx].ns = -1;
				blocks[idx].ew = -1;
				agent.block_hold = idx;
			}
		}
	}
	if (action==actions["PICK_RED"]) {
		if (agent.block_hold==-1 && eye_hand_sync()) {
			std::vector<int> red_blocks_under = find_red_block_under_hand();
			if (!red_blocks_under.empty()) {
				std::shuffle(red_blocks_under.begin(), red_blocks_under.end(), g);
				int idx = red_blocks_under.pop_back();
				blocks[idx].is_in_robot_hand = true;
				blocks[idx].ns = -1;
				blocks[idx].ew = -1;
				agent.block_hold = idx;
			}
		}
	}
	if (action==actions["PUT_DOWN"]) {
		std::list<int> red_blocks_under = find_red_block_under_hand();
		std::list<int> blue_blocks_under = find_blue_block_under_hand();
		if ((agent.block_hold!=-1)
				&& eye_hand_sync()
				&& red_blocks_under.empty()
				&& blue_blocks_under.empty()
				&& (red_box.ns!=agent.ns || red_box.ew!=agent.ew)
				&& (red_box.ns!=agent.ns || red_box.ew!=agent.ew)){
			blocks[agent.block_hold].is_in_robot_hand = false;
			blocks[agent.block_hold].ns = agent.ns;
			blocks[agent.block_hold].ew = agent.ew;
			agent.block_hold = -1;
		}
	}
	if (action==actions["PUT_IN"]) {
		if (agent.block_hold!=-1){
			if (red_box.ns==agent.ns && red_box.ew==agent.ew){
				blocks[agent.block_hold].is_in_robot_hand = false;
				blocks[agent.block_hold].is_in_red_box = true;
				blocks[agent.block_hold].ns = red_box.ns;
				blocks[agent.block_hold].ew = red_box.ew;
				agent.block_hold = -1;
			}
			else if (blue_box.ns==agent.ns && blue_box.ew==agent.ew){
				blocks[agent.block_hold].is_in_robot_hand = false;
				blocks[agent.block_hold].is_in_blue_box = true;
				blocks[agent.block_hold].ns = blue_box.ns;
				blocks[agent.block_hold].ew = blue_box.ew;
				agent.block_hold = -1;
			}
		}
	}
	if (action==action["LOOK_TUTOR"]){
		agent.eye_ew = tutor.eye_ew;
		agent.eye_ns = tutor.eye_ns,
		reward=+1;
	}
	if (action==actions["LOOK_RED_BOX"])	{
		agent.eye_ew = red_box.ew;
		agent.eye_ns = red_box.ns;
	}
	if (action==actions["LOOK_BLUE_BOX"]){
		agent.eye_ew = blue_box.ew;
		agent.eye_ns = blue_box.ns;
	}
	if (action>num_actions-nbBlueBlocks-nbRedBlocks-1
			&& action<num_actions){
		if (!(blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].is_in_blue_box)
				&& !(blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].is_in_red_box)
				&& !(blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].is_in_robot_hand))
		agent.eye_ew = blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].ew;
		agent.eye_ns = blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].ns;
	}

	update_sensors();
	return reward;
}



