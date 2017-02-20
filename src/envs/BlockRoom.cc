/*
 * BlockRoom.cc
 *
 *  Created on: 14 févr. 2017
 *      Author: pierre
 */

#include "../../include/envs/BlockRoom.hh"
#include <algorithm>

BlockRoom::BlockRoom(Random &rand):
	height(10),
	width(10),
	nbRedBlocks(2),
	nbBlueBlocks(3),
	rng(rand),
	s(11+6*(nbRedBlocks+nbBlueBlocks)),
	agent_ns(s[0]),
	agent_ew(s[1]),
	block_hold(s[2]),
	agent_eye_ns(s[3]),
	agent_eye_ew(s[4]),
	red_box_ns(s[5]),
	red_box_ew(s[6]),
	blue_box_ns(s[7]),
	blue_box_ew(s[8]),
	tutor_eye_ns(s[9]),
	tutor_eye_ew(s[10])
{
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

	for (int i = 0; i<nbRedBlocks; i++){
		block_t block(
				&(s[6*i+11]),
				&(s[6*i+12]),
				&(s[6*i+13]),
				&(s[6*i+14]),
				&(s[6*i+15]),
				&(s[6*i+16]));
		blocks.push_back(block);

		std::string name = "LOOK_RED_BLOCK_";
		name += std::to_string(i);
		actions[name]= cnt_actions++;
	}

	for (int i = 0; i<nbBlueBlocks; i++){
		block_t block(
				&(s[6*(i+nbRedBlocks)+11]),
				&(s[6*(i+nbRedBlocks)+12]),
				&(s[6*(i+nbRedBlocks)+13]),
				&(s[6*(i+nbRedBlocks)+14]),
				&(s[6*(i+nbRedBlocks)+15]),
				&(s[6*(i+nbRedBlocks)+16]));
		blocks.push_back(block);

		std::string name = "LOOK_BLUE_BLOCK_";
		name += std::to_string(i);
		actions[name]= cnt_actions++;
	}

	num_actions = cnt_actions;
}

BlockRoom::~BlockRoom() {}

const std::vector<float> &BlockRoom::sensation() const {
	if (LWDEBUG) print_map();
	  return s;
}

bool BlockRoom::terminal() const {
	return false;
}

int BlockRoom::getNumActions() {
  if (LWDEBUG) cout << "Return number of actions: " << num_actions << endl;
  return num_actions; //num_actions;
}

void BlockRoom::getMinMaxFeatures(std::vector<float> *minFeat,
                                   std::vector<float> *maxFeat){

  minFeat->resize(s.size(), 0.0);
  float maxSize = height > width ? height : width;
  maxFeat->resize(s.size(), maxSize);

  (*minFeat)[2] = -1;
  (*maxFeat)[2] = nbRedBlocks+nbBlueBlocks-1;
}

void BlockRoom::getMinMaxReward(float *minR,
                                 float *maxR){

  *minR = 0.0;
  *maxR = 1.0;

}

void BlockRoom::print_map() const{

  std::cout << "\nBlock room" << std::endl;

  /*// for each row
  for (int j = height-1; j >= 0; --j){
    // for each column
    for (int i = 0; i < width; i++){
      if (agent_ns == j && agent_ew == i) cout << "A";
      else if (j == red_box_ns && i == red_box_ew) cout << "R";
      else if (j == blue_box_ns && i == blue_box_ew) cout << "B";
      else if (j == red_box_ns && i == red_box_ew) cout << "R";
      else if (j == red_box_ns && i == red_box_ew) cout << "R";
      else if (j == rooms[room_id].lock_ns && i == rooms[room_id].lock_ew) cout << "L";
      else if (j == rooms[room_id].door_ns && i == rooms[room_id].door_ew) cout << "D";
      else if (j == 0 || i == 0 || j == rooms[room_id].height-1 || i == rooms[room_id].width-1) cout << "X";
      else cout << ".";
    } // last col of row
    cout << endl;
  } // last row

  cout << "at " << ns << ", " << ew << endl;
  cout << "Key: " << have_key << " door: "<< door_open << endl;
  cout << "NORTH: key: " << key_n << ", door: " << door_n << ", lock: " << lock_n << endl;
  cout << "EAST: key: " << key_e << ", door: " << door_e << ", lock: " << lock_e << endl;
  cout << "SOUTH: key: " << key_s << ", door: " << door_s << ", lock: " << lock_s << endl;
  cout << "WEST: key: " << key_w << ", door: " << door_w << ", lock: " << lock_w << endl;*/


}

void BlockRoom::reset(){
	agent_ew = rng.uniformDiscrete(0, width-1);
	agent_ns = rng.uniformDiscrete(0, height-1);
	agent_eye_ew = rng.uniformDiscrete(0, width-1);
	agent_eye_ns = rng.uniformDiscrete(0, height-1);
	tutor_eye_ew = rng.uniformDiscrete(0, width -1);
	tutor_eye_ns = rng.uniformDiscrete(0, height-1);
	red_box_ew = rng.uniformDiscrete(0, width-1);
	red_box_ns = rng.uniformDiscrete(0, height-1);
	do {
		blue_box_ew = rng.uniformDiscrete(0, width-1);
		blue_box_ns = rng.uniformDiscrete(0, height-1);
	} while (red_box_ew==blue_box_ew && blue_box_ns==blue_box_ns);

	std::vector<int> x(height*width-1);
	std::iota(x.begin(), x.end(), 0);

	std::shuffle(x.begin(), x.end(), engine);

	for (std::vector<int>::iterator it = x.begin();
			it != x.begin()+nbRedBlocks;++it){
		int i = it-x.begin();
		*(blocks[i].color) = RED;
		*(blocks[i].ew) = (*it % width);
		*(blocks[i].ns) = (*it / width);
	}

	for (std::vector<int>::iterator it = x.begin()+nbRedBlocks;
			it != x.begin()+nbRedBlocks+nbBlueBlocks;++it){
		int i = it-(x.begin()+nbRedBlocks);
		*(blocks[i].color) = BLUE;
		*(blocks[i].ew) = (*it % width);
		*(blocks[i].ns) = (*it / width);
	}

}

int BlockRoom::applyNoise(int action){
	if (action == actions["NORTH"]){
		return rng.bernoulli(0.9) ? action : (rng.bernoulli(0.5) ?
				actions["EAST"] : actions["WEST"]);
	}
	if (action == actions["SOUTH"]){
		return rng.bernoulli(0.9) ? action : (rng.bernoulli(0.5) ?
				actions["EAST"] : actions["WEST"]);
	}
	if (action == actions["EAST"]){
		return rng.bernoulli(0.9) ? action : (rng.bernoulli(0.5) ?
				actions["EAST"] : actions["WEST"]);
	}
	if (action==actions["WEST"]){
		return rng.bernoulli(0.9) ? action : (rng.bernoulli(0.5) ?
				actions["NORTH"] : actions["SOUTH"]);
	}
	else {
		return action;
	}
}

std::vector<int> BlockRoom::find_red_block_under_hand() {
	std::vector<int> l;
	for (std::vector<block_t>::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (*(it->ns)==agent_ns && *(it->ew)==agent_ew && *(it->color)==RED){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}

std::vector<int> BlockRoom::find_blue_block_under_hand() {
	std::vector<int> l;
	for (std::vector<block_t>::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (*(it->ns)==agent_ns && *(it->ew)==agent_ew && *(it->color)==BLUE){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}

bool BlockRoom::eye_hand_sync(){
	return ((BlockRoom::agent_eye_ns==BlockRoom::agent_ns)
			&& (BlockRoom::agent_eye_ew==BlockRoom::agent_ew));
}

float BlockRoom::apply(int action){
	float reward = 0.;

	if (action==actions["NORTH"]) {
		if (agent_ns < height-1) {agent_ns++;}
	}
	if (action==actions["SOUTH"]) {
		if (agent_ns > 0) {agent_ns--;}
	}
	if (action==actions["EAST"]) {
		if (agent_ew < width-1) {agent_ew++;}
	}
	if (action==actions["WEST"]) {
		if (agent_ew > 0) {agent_ew--;}
	}
	if (action==actions["PICK_BLUE"]) {
		if (block_hold==-1 && eye_hand_sync()) {
			std::vector<int> blue_blocks_under = find_blue_block_under_hand();
			if (!blue_blocks_under.empty()) {
				std::shuffle(blue_blocks_under.begin(), blue_blocks_under.end(), engine);
				int idx = blue_blocks_under.back();
				*(blocks[idx].is_in_robot_hand) = true;
				*(blocks[idx].ns) = -1;
				*(blocks[idx].ew) = -1;
				block_hold = idx;
			}
		}
	}
	if (action==actions["PICK_RED"]) {
		if (block_hold==-1 && eye_hand_sync()) {
			std::vector<int> red_blocks_under = find_red_block_under_hand();
			if (!red_blocks_under.empty()) {
				std::shuffle(red_blocks_under.begin(), red_blocks_under.end(), engine);
				int idx = red_blocks_under.back();
				*(blocks[idx].is_in_robot_hand) = true;
				*(blocks[idx].ns) = -1;
				*(blocks[idx].ew) = -1;
				block_hold = idx;
			}
		}
	}
	if (action==actions["PUT_DOWN"]) {
		std::vector<int> red_blocks_under = find_red_block_under_hand();
		std::vector<int> blue_blocks_under = find_blue_block_under_hand();
		if ((block_hold!=-1)
				&& eye_hand_sync()
				&& red_blocks_under.empty()
				&& blue_blocks_under.empty()
				&& (red_box_ns!=agent_ns || red_box_ew!=agent_ew)
				&& (red_box_ns!=agent_ns || red_box_ew!=agent_ew)){
			*(blocks[block_hold].is_in_robot_hand) = false;
			*(blocks[block_hold].ns) = agent_ns;
			*(blocks[block_hold].ew) = agent_ew;
			block_hold = -1;
		}
	}
	if (action==actions["PUT_IN"]) {
		if (block_hold!=-1){
			if (red_box_ns==agent_ns && red_box_ew==agent_ew){
				*(blocks[block_hold].is_in_robot_hand) = false;
				*(blocks[block_hold].is_in_red_box) = true;
				*(blocks[block_hold].ns) = red_box_ns;
				*(blocks[block_hold].ew) = red_box_ew;
				block_hold = -1;
			}
			else if (blue_box_ns==agent_ns && blue_box_ew==agent_ew){
				*(blocks[block_hold].is_in_robot_hand) = false;
				*(blocks[block_hold].is_in_blue_box) = true;
				*(blocks[block_hold].ns) = blue_box_ns;
				*(blocks[block_hold].ew) = blue_box_ew;
				block_hold = -1;
			}
		}
	}
	if (action==action["LOOK_TUTOR"]){
		agent_eye_ew = tutor_eye_ew;
		agent_eye_ns = tutor_eye_ns,
		reward=+1;
	}
	if (action==actions["LOOK_RED_BOX"])	{
		agent_eye_ew = red_box_ew;
		agent_eye_ns = red_box_ns;
	}
	if (action==actions["LOOK_BLUE_BOX"]){
		agent_eye_ew = blue_box_ew;
		agent_eye_ns = blue_box_ns;
	}
	if (action>num_actions-nbBlueBlocks-nbRedBlocks-1
			&& action<num_actions){
		if (!(*(blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].is_in_blue_box))
				&& !(*(blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].is_in_red_box))
				&& !(*(blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].is_in_robot_hand)))
		agent_eye_ew = *(blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].ew);
		agent_eye_ns = *(blocks[action-(num_actions-nbBlueBlocks-nbRedBlocks-1)].ns);
	}

	return reward;
}



