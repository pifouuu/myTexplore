/*
 * BlockRoom.cc
 *
 *  Created on: 14 févr. 2017
 *      Author: pierre
 */

#include "../../include/envs/BlockRoom.hh"
#include <algorithm>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;

BlockRoom::BlockRoom(Random &rand, bool with_tutor):
	height(10),
	width(10),
	nbRedBlocks(2),
	nbBlueBlocks(3),
	rng(rand),
	WITH_TUTOR(with_tutor),
	state_dim_base(9),
	s(state_dim_base+6*(nbRedBlocks+nbBlueBlocks)+2*with_tutor),
	agent_ns(&(s[0])),
	agent_ew(&(s[1])),
	block_hold(&(s[2])),
	agent_eye_ns(&(s[3])),
	agent_eye_ew(&(s[4])),
	red_box_ns(&(s[5])),
	red_box_ew(&(s[6])),
	blue_box_ns(&(s[7])),
	blue_box_ew(&(s[8])),
	numstep(0)
{
	int cnt_actions = 0;
	int cnt_tutor_actions = 0;

	/*actions[std::string("NORTH")] = cnt_actions++;
	actions[std::string("SOUTH")] = cnt_actions++;
	actions[std::string("EAST")] = cnt_actions++;
	actions[std::string("WEST")] = cnt_actions++;*/
	actions[std::string("GO_TO_EYE")] = cnt_actions++;
	actions[std::string("LOOK_RED_BOX")] = cnt_actions++;
	actions[std::string("LOOK_BLUE_BOX")] = cnt_actions++;
	/*actions[std::string("PICK_BLUE")] = cnt_actions++;
	actions[std::string("PICK_RED")] = cnt_actions++;*/
	actions[std::string("PICK")] = cnt_actions++;
	actions[std::string("PUT_DOWN")] = cnt_actions++;
	actions[std::string("PUT_IN")] = cnt_actions++;


	if (WITH_TUTOR){
		tutor_eye_ns = &(s[state_dim_base]);
		tutor_eye_ew = &(s[state_dim_base+1]);
		actions[std::string("LOOK_TUTOR")] = cnt_actions++;


		tutor_actions[std::string("LOOK_AGENT")] = cnt_tutor_actions++;
		tutor_actions[std::string("LOOK_RED_BOX")] = cnt_tutor_actions++;
		tutor_actions[std::string("LOOK_BLUE_BOX")] = cnt_tutor_actions++;
	}

	int nb_fix_actions = cnt_actions;

	for (int i = 0; i<nbRedBlocks; i++){
		block_t block(
				&(s[6*i+state_dim_base+2*with_tutor + 0]),
				&(s[6*i+state_dim_base+2*with_tutor + 1]),
				&(s[6*i+state_dim_base+2*with_tutor + 2]),
				&(s[6*i+state_dim_base+2*with_tutor + 3]),
				&(s[6*i+state_dim_base+2*with_tutor + 4]),
				&(s[6*i+state_dim_base+2*with_tutor + 5]));
		blocks.push_back(block);

		std::string name = "LOOK_RED_BLOCK_";
		name += std::to_string(i);
		actions[name] = cnt_actions++;
		if (WITH_TUTOR){
			tutor_actions[name] = cnt_tutor_actions++;
		}

	}

	for (int i = 0; i<nbBlueBlocks; i++){
		block_t block(
				&(s[6*(i+nbRedBlocks)+state_dim_base+2*with_tutor + 0]),
				&(s[6*(i+nbRedBlocks)+state_dim_base+2*with_tutor + 1]),
				&(s[6*(i+nbRedBlocks)+state_dim_base+2*with_tutor + 2]),
				&(s[6*(i+nbRedBlocks)+state_dim_base+2*with_tutor + 3]),
				&(s[6*(i+nbRedBlocks)+state_dim_base+2*with_tutor + 4]),
				&(s[6*(i+nbRedBlocks)+state_dim_base+2*with_tutor + 5]));
		blocks.push_back(block);

		std::string name = "LOOK_BLUE_BLOCK_";
		name += std::to_string(i);
		actions[name]= cnt_actions++;
		if (WITH_TUTOR){
			tutor_actions[name] = cnt_tutor_actions++;
		}
	}

	numactions = cnt_actions;
	num_tutor_actions = cnt_tutor_actions;
	for (std::map<std::string, int>::iterator it = actions.begin();
			it != actions.end() ; ++it ){
		action_names[it->second] = it->first;
	}
	reset();

	if (BRDEBUG) print_map();
}

BlockRoom::~BlockRoom() {}

const std::vector<float> &BlockRoom::sensation() const {
	if (BRDEBUG) print_map();
	  return s;
}

bool BlockRoom::terminal() const {
	return false;
}

std::map<int, std::string> BlockRoom::get_action_names(){
	return action_names;
}

int BlockRoom::get_blocks_in() const {
	int nb_blocks_in = 0;
	for (int i = state_dim_base + 2*WITH_TUTOR + 4; i<s.size();i+=6){
		nb_blocks_in += (s[i]==1 || s[i+1]==1);
	}
}

int BlockRoom::get_blocks_right() const {
	int nb_blocks_right = 0;
	for (int i = state_dim_base + 2*WITH_TUTOR; i<s.size();i+=6){
		nb_blocks_right += ((s[i+2]==0 && s[i+5]==1) || (s[i+2]==1 && s[i+4]==1));
	}
}


int BlockRoom::getNumActions() {
  if (BRDEBUG) cout << "Return number of actions: " << numactions << endl;
  return numactions; //num_actions;
}

int BlockRoom::getNumTutorActions() {
  if (BRDEBUG) cout << "Return number of tutor actions: " << num_tutor_actions << endl;
  return num_tutor_actions; //num_actions;
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

	int blockSize=80;
	Size size(blockSize, blockSize);

	std::map<std::pair<int,int>,std::list<Mat>> posToImg;

	Mat chessBoard(blockSize*height,blockSize*width,CV_8UC3,Scalar::all(0));
	unsigned char color=0;
	string img_dir = "/home/pierre/workspace/myTexplore/images/";
	Mat red_block_img = imread(img_dir+"red_block.png",CV_LOAD_IMAGE_COLOR);
	resize(red_block_img, red_block_img, size);
	Mat blue_block_img = imread(img_dir+"blue_block.png",CV_LOAD_IMAGE_COLOR);
	resize(blue_block_img, blue_block_img, size);
	Mat blue_box_img = imread(img_dir+"blue_box.png",CV_LOAD_IMAGE_COLOR);
	resize(blue_box_img, blue_box_img, size);
	Mat red_box_img = imread(img_dir+"red_box.png",CV_LOAD_IMAGE_COLOR);
	resize(red_box_img, red_box_img, size);
	Mat agent_eye_img = imread(img_dir+"agent_eye.png",CV_LOAD_IMAGE_COLOR);
	resize(agent_eye_img, agent_eye_img, size);
	Mat tutor_eye_img = imread(img_dir+"tutor_eye.png",CV_LOAD_IMAGE_COLOR);
	resize(tutor_eye_img, tutor_eye_img, size);
	Mat agent_hand_img = imread(img_dir+"agent_hand.png",CV_LOAD_IMAGE_COLOR);
	resize(agent_hand_img, agent_hand_img, size);



	for(int i=0;i<blockSize*height;i=i+blockSize){
		color=~color;
		for(int j=0;j<blockSize*width;j=j+blockSize){
			Mat ROI=chessBoard(Rect(i,j,blockSize,blockSize));
			ROI.setTo(Scalar::all(color));
			color=~color;
		}
	}

	for (std::vector<block_t>::const_iterator it = blocks.begin();  it != blocks.end(); ++it){
		if (*(it->color)==RED){
			int x = blockSize*(*(it->ew));
			int y = blockSize*(*(it->ns));
			posToImg[std::pair<int,int>(x,y)].push_back(red_block_img);
			/*red_block_img.copyTo(chessBoard(cv::Rect(blockSize*(*(it->ns)),
					blockSize*(*(it->ew)),red_block_img.cols, red_block_img.rows)));*/
		}
		if (*(it->color)==BLUE){
			int x = blockSize*(*(it->ew));
			int y = blockSize*(*(it->ns));
			posToImg[std::pair<int,int>(x,y)].push_back(blue_block_img);
			/*blue_block_img.copyTo(chessBoard(cv::Rect(blockSize*(*(it->ns)),
					blockSize*(*(it->ew)),blue_block_img.cols, blue_block_img.rows)));*/
		}
	}

	posToImg[std::pair<int,int>(blockSize*(*blue_box_ew),blockSize*(*blue_box_ns))].push_back(blue_box_img);
	posToImg[std::pair<int,int>(blockSize*(*red_box_ew),blockSize*(*red_box_ns))].push_back(red_box_img);
	posToImg[std::pair<int,int>(blockSize*(*agent_eye_ew),blockSize*(*agent_eye_ns))].push_back(agent_eye_img);
	if (WITH_TUTOR){
		posToImg[std::pair<int,int>(blockSize*(*tutor_eye_ew),blockSize*(*tutor_eye_ns))].push_back(tutor_eye_img);
	}
	posToImg[std::pair<int,int>(blockSize*(*agent_ew),blockSize*(*agent_ns))].push_back(agent_hand_img);

	for (auto elem : posToImg){
		if (elem.second.size()==1){
			elem.second.front().copyTo(chessBoard(cv::Rect(elem.first.first,
					elem.first.second,elem.second.front().cols, elem.second.front().rows)));
		}
		if (elem.second.size()>1 && elem.second.size()<5){
			int i = 0;
			while (!elem.second.empty()){
				Size size2(blockSize/2, blockSize/2);
				resize(elem.second.front(), elem.second.front(), size2);
				int x_decal = i % 2;
				int y_decal = i/2;
				elem.second.front().copyTo(chessBoard(cv::Rect(elem.first.first+x_decal*blockSize/2,
								elem.first.second+y_decal*blockSize/2,
								elem.second.front().cols,
								elem.second.front().rows)));
				elem.second.pop_front();
				i++;
			}
		}
		else if (elem.second.size()>4) {
			std::cout << "fuck" << std::endl;
		}
	}

	imshow("Chess board", chessBoard);
	waitKey(1);
}

void BlockRoom::reset(){
	(*block_hold) = -1;
	(*agent_ew) = rng.uniformDiscrete(0, width-1);
	(*agent_ns) = rng.uniformDiscrete(0, height-1);
	(*agent_eye_ew) = rng.uniformDiscrete(0, width-1);
	(*agent_eye_ns) = rng.uniformDiscrete(0, height-1);
	if (WITH_TUTOR){
		(*tutor_eye_ew) = rng.uniformDiscrete(0, width -1);
		(*tutor_eye_ns) = rng.uniformDiscrete(0, height-1);
	}
	(*red_box_ew) = rng.uniformDiscrete(0, width-1);
	(*red_box_ns) = rng.uniformDiscrete(0, height-1);
	do {
		(*blue_box_ew) = rng.uniformDiscrete(0, width-1);
		(*blue_box_ns) = rng.uniformDiscrete(0, height-1);
	} while ((*red_box_ew)==(*blue_box_ew) && (*blue_box_ns)==(*blue_box_ns));

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
	return action;
}

std::vector<int> BlockRoom::find_block_under(int ns, int ew) {
	std::vector<int> l;
	for (std::vector<block_t>::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (*(it->ns)==(ns) && *(it->ew)==(ew)){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}

std::vector<int> BlockRoom::find_red_block_under_hand() {
	std::vector<int> l;
	for (std::vector<block_t>::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (*(it->ns)==(*agent_ns) && *(it->ew)==(*agent_ew) && *(it->color)==RED){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}

std::vector<int> BlockRoom::find_blue_block_under_hand() {
	std::vector<int> l;
	for (std::vector<block_t>::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (*(it->ns)==(*agent_ns) && *(it->ew)==(*agent_ew) && *(it->color)==BLUE){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}

bool BlockRoom::eye_hand_sync(){
	return (*(BlockRoom::agent_eye_ns)==*(BlockRoom::agent_ns)
			&& *(BlockRoom::agent_eye_ew)==*(BlockRoom::agent_ew));
}
void BlockRoom::apply_tutor(int action){
	if (action==actions["LOOK_TUTOR"]){
		(*tutor_eye_ew) = (*agent_eye_ew);
		(*tutor_eye_ns) = (*agent_eye_ns);
	}
	if (action==tutor_actions["LOOK_RED_BOX"])	{
		(*tutor_eye_ew) = (*red_box_ew);
		(*tutor_eye_ns) = (*red_box_ns);
	}
	if (action==tutor_actions["LOOK_BLUE_BOX"]){
		(*tutor_eye_ew) = (*blue_box_ew);
		(*tutor_eye_ns) = (*blue_box_ns);
	}
	if (action>num_tutor_actions-nbBlueBlocks-nbRedBlocks-1
			&& action<num_tutor_actions){
		int num_block = action-(num_tutor_actions-nbBlueBlocks-nbRedBlocks);
		if (!(*(blocks[num_block].is_in_blue_box))
				&& !(*(blocks[num_block].is_in_red_box))
				&& !(*(blocks[num_block].is_in_robot_hand))){
			(*tutor_eye_ew) = *(blocks[num_block].ew);
			(*tutor_eye_ns) = *(blocks[num_block].ns);
		}
	}
}

std::vector<std::pair<int,int>> BlockRoom::get_nearby_pos(int ns, int ew){
	std::vector<std::pair<int,int>> nearby_pos;
	if (ns<height-1){nearby_pos.push_back(std::make_pair(ns+1,ew));}
	if (ew<width-1){nearby_pos.push_back(std::make_pair(ns,ew+1));}
	if (ew>0){nearby_pos.push_back(std::make_pair(ns,ew-1));}
	if (ns>0){nearby_pos.push_back(std::make_pair(ns-1,ew));}
	return nearby_pos;
}

float BlockRoom::getStateActionInfoError(const std::vector<float> s, std::vector<StateActionInfo> preds){
	float diff = 0.;
	for (int action = 0;action<numactions;action++){
		std::vector<float> next_state = s;
		std::map< std::vector<float> , float> trueTransitionProbs;
		if (action==actions["GO_TO_EYE"]) {
			next_state[0] = s[3];
			next_state[1] = s[4];
			if (s[2]>=0){
				next_state[6*s[2]+state_dim_base+2*WITH_TUTOR]=s[3];
				next_state[6*s[2]+state_dim_base+2*WITH_TUTOR+1]=s[4];
			}
			trueTransitionProbs[next_state] = 1.;
		}
		if (action == actions["PICK"]){
			if ((s[2])==-1 && s[3]==s[0] && s[4]==s[1]) {
				std::vector<int> blocks_under = find_block_under(s[0],s[1]);
				if (!blocks_under.empty()) {
					int nb_blocks = blocks_under.size();
					for (auto block:blocks_under){
						next_state[2]=block;
						next_state[6*block+state_dim_base+2*WITH_TUTOR + 3] = 1;
						trueTransitionProbs[next_state] = 1./nb_blocks;
					}
				}
				else {
					trueTransitionProbs[next_state] = 1.;
				}
			}
			else {
				trueTransitionProbs[next_state] = 1.;
			}
		}
		if (action==actions["PUT_DOWN"]) {
			if (s[2]!=-1
					&& s[3]==s[0] && s[4]==s[1]
					&& find_block_under(s[0],s[1]).empty()
					&& ((s[5])!=(s[0]) || (s[6])!=(s[1]))
					&& ((s[7])!=(s[0]) || (s[8])!=(s[1]))){
				next_state[6*s[2]+state_dim_base+2*WITH_TUTOR+3] = 0;
				next_state[2]=-1;
				trueTransitionProbs[next_state] = 1.;
			}
			else {
				trueTransitionProbs[next_state] = 1.;
			}
		}
		if (action==actions["PUT_IN"]) {
			if ((s[2])!=-1){
				if ((s[5])==(s[0]) && (s[6])==(s[1])){
					next_state[6*s[2]+state_dim_base+2*WITH_TUTOR+3] = 0;
					next_state[6*s[2]+state_dim_base+2*WITH_TUTOR+5] = 1;
					next_state[2]=-1;
					trueTransitionProbs[next_state]=1.;
				}
				else if ((s[7])==(s[0]) && (s[8])==(s[1])){
					next_state[6*s[2]+state_dim_base+2*WITH_TUTOR+3] = 0;
					next_state[6*s[2]+state_dim_base+2*WITH_TUTOR+4] = 1;
					next_state[2]=-1;
					trueTransitionProbs[next_state]=1.;
				}
				else {
					trueTransitionProbs[next_state] = 1.;
				}
			}
			else {
				trueTransitionProbs[next_state] = 1.;
			}
		}
		if (WITH_TUTOR && action==actions["LOOK_TUTOR"]){
			if ((s[4]) != (s[10]) || (s[3]) != (s[9])){
				next_state[4] = (s[10]);
				next_state[3] = (s[9]);
				trueTransitionProbs[next_state] = 1.;
			}
			else {
				trueTransitionProbs[next_state]=1.;
			}
		}
		if (WITH_TUTOR && action==actions["LOOK_RED_BOX"]){
			next_state[4] = (s[6]);
			next_state[3] = (s[5]);
			trueTransitionProbs[next_state] = 1.;
		}
		if (WITH_TUTOR && action==actions["LOOK_BLUE_BOX"]){
			next_state[4] = (s[8]);
			next_state[3] = (s[7]);
			trueTransitionProbs[next_state] = 1.;
		}
		if (action>numactions-nbBlueBlocks-nbRedBlocks-1
				&& action<numactions){
			int num_block = action-(numactions-nbBlueBlocks-nbRedBlocks);
			if (s[6*num_block+state_dim_base+2*WITH_TUTOR+3]==0
					&& s[6*num_block+state_dim_base+2*WITH_TUTOR+4]==0
					&& s[6*num_block+state_dim_base+2*WITH_TUTOR+5]==0) {
				next_state[4] = s[6*num_block+state_dim_base+2*WITH_TUTOR+1];
				next_state[3] = s[6*num_block+state_dim_base+2*WITH_TUTOR];
				trueTransitionProbs[next_state]=1.;
			}
			else{
				trueTransitionProbs[next_state] = 1;
			}
		}
		for (auto elem:trueTransitionProbs){
			auto insert_pair = preds[action].transitionProbs.insert(elem);
			if (insert_pair.second){
				diff += pow(insert_pair.first->second,2);
			}
			else {
				diff += pow(insert_pair.first->second-elem.second,2);
			}
		}
		for (auto elem:preds[action].transitionProbs){
			if (trueTransitionProbs.find(elem.first) == trueTransitionProbs.end()){
				diff += pow(elem.second,2);
			}
		}
	}

	return diff;
}

occ_info_t BlockRoom::apply(int action){
	float reward = 0.;
	float virtual_reward = 0;
	bool success = false;


	/*if (action==actions["NORTH"]) {
		if ((*agent_ns) < height-1) {
			(*agent_ns)++;
			success = true;
		}
	}
	if (action==actions["SOUTH"]) {
		if ((*agent_ns) > 0) {
			(*agent_ns)--;
			success = true;
		}
	}
	if (action==actions["EAST"]) {
		if ((*agent_ew) < width-1) {
			(*agent_ew)++;
			success = true;
		}
	}
	if (action==actions["WEST"]) {
		if ((*agent_ew) > 0) {
			(*agent_ew)--;
			success = true;
		}
	}*/
	if (action==actions["GO_TO_EYE"]) {
		if (rng.bernoulli(0.8)) {
			(*agent_ns) = (*agent_eye_ns);
			(*agent_ew) = (*agent_eye_ew);
		}
		else {
			std::vector<std::pair<int,int>> nearby_pos = get_nearby_pos(*agent_eye_ns,*agent_eye_ew);
			std::shuffle(nearby_pos.begin(), nearby_pos.end(), engine);
			(*agent_ns) = nearby_pos.front().first;
			(*agent_ew) = nearby_pos.front().second;
		}
		success = true;
	}
	if (action == actions["PICK"]){
		if ((*block_hold)==-1 && eye_hand_sync()) {
			std::vector<int> blocks_under = find_block_under(*agent_ns,*agent_ew);
			if (!blocks_under.empty()) {
				std::shuffle(blocks_under.begin(), blocks_under.end(), engine);
				int idx = blocks_under.back();
				if (rng.bernoulli(0.8)){
					*(blocks[idx].is_in_robot_hand) = true;
					(*block_hold) = idx;
				}
				success = true;
			}
		}
	}
	/*if (action==actions["PICK_BLUE"]) {
		if ((*block_hold)==-1 && eye_hand_sync()) {
			std::vector<int> blue_blocks_under = find_blue_block_under_hand();
			if (!blue_blocks_under.empty()) {
				std::shuffle(blue_blocks_under.begin(), blue_blocks_under.end(), engine);
				int idx = blue_blocks_under.back();
				*(blocks[idx].is_in_robot_hand) = true;
				(*block_hold) = idx;
				success = true;
			}
		}
	}
	if (action==actions["PICK_RED"]) {
		if ((*block_hold)==-1 && eye_hand_sync()) {
			std::vector<int> red_blocks_under = find_red_block_under_hand();
			if (!red_blocks_under.empty()) {
				std::shuffle(red_blocks_under.begin(), red_blocks_under.end(), engine);
				int idx = red_blocks_under.back();
				*(blocks[idx].is_in_robot_hand) = true;
				(*block_hold) = idx;
				success = true;
			}
		}
	}*/
	if (action==actions["PUT_DOWN"]) {
		std::vector<int> red_blocks_under = find_red_block_under_hand();
		std::vector<int> blue_blocks_under = find_blue_block_under_hand();
		if (((*block_hold)!=-1)
				&& eye_hand_sync()
				&& red_blocks_under.empty()
				&& blue_blocks_under.empty()
				&& ((*red_box_ns)!=(*agent_ns) || (*red_box_ew)!=(*agent_ew))
				&& ((*blue_box_ns)!=(*agent_ns) || (*blue_box_ew)!=(*agent_ew))){
			*(blocks[(*block_hold)].is_in_robot_hand) = false;
			if (rng.bernoulli(0.8)){
				*(blocks[(*block_hold)].ns) = (*agent_ns);
				*(blocks[(*block_hold)].ew) = (*agent_ew);
			}
			else{
				std::vector<std::pair<int,int>> nearby_pos = get_nearby_pos(*agent_ns,*agent_ew);
				std::shuffle(nearby_pos.begin(), nearby_pos.end(), engine);
				*(blocks[(*block_hold)].ns) = nearby_pos.front().first;
				*(blocks[(*block_hold)].ew) = nearby_pos.front().second;
			}

			(*block_hold) = -1;
			success = true;
		}
	}
	if (action==actions["PUT_IN"]) {
		if ((*block_hold)!=-1){
			if ((*red_box_ns)==(*agent_ns) && (*red_box_ew)==(*agent_ew)){
				*(blocks[(*block_hold)].is_in_robot_hand) = false;
				if (rng.bernoulli(0.8)){
					*(blocks[(*block_hold)].is_in_red_box) = true;
					*(blocks[(*block_hold)].ns) = (*red_box_ns);
					*(blocks[(*block_hold)].ew) = (*red_box_ew);
				}
				else{
					std::vector<std::pair<int,int>> nearby_pos = get_nearby_pos(*red_box_ns,*red_box_ew);
					std::shuffle(nearby_pos.begin(), nearby_pos.end(), engine);
					*(blocks[(*block_hold)].ns) = nearby_pos.front().first;
					*(blocks[(*block_hold)].ew) = nearby_pos.front().second;
				}
				(*block_hold) = -1;
				success = true;
			}
			else if ((*blue_box_ns)==(*agent_ns) && (*blue_box_ew)==(*agent_ew)){
				*(blocks[(*block_hold)].is_in_robot_hand) = false;
				if (rng.bernoulli(0.8)){
					*(blocks[(*block_hold)].is_in_blue_box) = true;
					*(blocks[(*block_hold)].ns) = (*blue_box_ns);
					*(blocks[(*block_hold)].ew) = (*blue_box_ew);
				}
				else{
					std::vector<std::pair<int,int>> nearby_pos = get_nearby_pos(*blue_box_ns,*blue_box_ew);
					std::shuffle(nearby_pos.begin(), nearby_pos.end(), engine);
					*(blocks[(*block_hold)].ns) = nearby_pos.front().first;
					*(blocks[(*block_hold)].ew) = nearby_pos.front().second;
				}
				(*block_hold) = -1;
				success = true;
			}
		}
	}
	if (WITH_TUTOR && action==actions["LOOK_TUTOR"]){
		if ((*agent_eye_ew) != (*tutor_eye_ew) || (*agent_eye_ns) != (*tutor_eye_ns)){
			(*agent_eye_ew) = (*tutor_eye_ew);
			(*agent_eye_ns) = (*tutor_eye_ns),
			reward=+1;
			success = true;
		}
	}
	if (action==actions["LOOK_RED_BOX"])	{
		(*agent_eye_ew) = (*red_box_ew);
		(*agent_eye_ns) = (*red_box_ns);
		success = true;
	}
	if (action==actions["LOOK_BLUE_BOX"]){
		(*agent_eye_ew) = (*blue_box_ew);
		(*agent_eye_ns) = (*blue_box_ns);
		success = true;
	}
	if (action>numactions-nbBlueBlocks-nbRedBlocks-1
			&& action<numactions){
		int num_block = action-(numactions-nbBlueBlocks-nbRedBlocks);
		if (!(*(blocks[num_block].is_in_blue_box))
				&& !(*(blocks[num_block].is_in_red_box))
				&& !(*(blocks[num_block].is_in_robot_hand))){
			(*agent_eye_ew) = *(blocks[num_block].ew);
			(*agent_eye_ns) = *(blocks[num_block].ns);
			success = true;
		}
	}
	if ((*block_hold)>-1){
		*(blocks[(*block_hold)].ns) = (*agent_ns);
		*(blocks[(*block_hold)].ew) = (*agent_ew);
	}

	actions_occurences[action].push_back(numstep);
	numstep++;
	return occ_info_t(reward, success, get_blocks_in(), get_blocks_right());
}



