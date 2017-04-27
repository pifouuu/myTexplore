/*
 * InfiniteBlocks.cc
 *
 *  Created on: 14 févr. 2017
 *      Author: pierre
 */

#include "../../include/envs/InfiniteBlocks.hh"
#include <algorithm>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;

InfiniteBlocks::InfiniteBlocks(Random &rand, int size, bool with_tutor, bool stochastic, float finalReward):
			size(size),
			stochastic(stochastic),
			rng(rand),
			WITH_TUTOR(with_tutor),
			finalReward(finalReward),
			agent_ns(size/2),
			agent_ew(size/2),
			red_block_hold(0),
			blue_block_hold(0),
			t_state(2),
			numstep(0),
			red_box_count_red(0),
			red_box_count_blue(0),
			blue_box_count_red(0),
			blue_box_count_blue(0)
{
	int cnt_actions = 0;
	int cnt_tutor_actions = 0;

	actions[std::string("NORTH")] = cnt_actions++;
	actions[std::string("SOUTH")] = cnt_actions++;
	actions[std::string("EAST")] = cnt_actions++;
	actions[std::string("WEST")] = cnt_actions++;
//	actions[std::string("GO_TO_EYE")] = cnt_actions++;


	actions[std::string("PICK")] = cnt_actions++;
//	actions[std::string("PUT_DOWN")] = cnt_actions++;
	actions[std::string("PUT_IN")] = cnt_actions++;


	if (WITH_TUTOR){
		tutor_eye_ns = &(t_state[0]);
		tutor_eye_ew = &(t_state[1]);

		tutor_actions[std::string("LOOK_AGENT")] = cnt_tutor_actions++;
		tutor_actions[std::string("LOOK_RED_BOX")] = cnt_tutor_actions++;
		tutor_actions[std::string("LOOK_BLUE_BOX")] = cnt_tutor_actions++;
		tutor_actions[std::string("LOOK_BLUE_BLOCKS")] = cnt_tutor_actions++;
		tutor_actions[std::string("LOOK_RED_BLOCKS")] = cnt_tutor_actions++;
	}

	int nb_fix_actions = cnt_actions;

	numactions = cnt_actions;
	num_tutor_actions = cnt_tutor_actions;
	for (std::map<std::string, int>::iterator it = actions.begin();
			it != actions.end() ; ++it ){
		action_names[it->second] = it->first;
	}

	agent_ns = size/2;
	agent_ew = size/2;
	red_block_hold = 0;
	blue_block_hold = 0;

	position red_blocks_pos(size-1,size-1);
	objects.push_back(red_blocks_pos);


	position blue_blocks_pos(0,0);
	objects.push_back(blue_blocks_pos);


	position red_box_pos(0,size-1);
	objects.push_back(red_box_pos);


	position blue_box_pos(size-1,0);
	objects.push_back(blue_box_pos);


	red_blocks_ns = &(objects[0].ypos);
	red_blocks_ew = &(objects[0].xpos);
	blue_blocks_ns = &(objects[1].ypos);
	blue_blocks_ew = &(objects[1].xpos);
	red_box_ns = &(objects[2].ypos);
	red_box_ew = &(objects[2].xpos);
	blue_box_ns = &(objects[3].ypos);
	blue_box_ew = &(objects[3].xpos);

}

InfiniteBlocks::~InfiniteBlocks() {}

std::vector<float> InfiniteBlocks::sensation() {
	std::vector<float> s = {agent_ns,agent_ew,red_block_hold,blue_block_hold};
	for (int i=0; i<objects.size();i++){
		s.push_back(objects[i].xpos);
		s.push_back(objects[i].ypos);
	}
	return s;
}

std::vector<float> InfiniteBlocks::generateSample(){
	agent_ns = rng.uniformDiscrete(0, size-1);
	agent_ew = rng.uniformDiscrete(0, size-1);
	float tirage = rng.uniformDiscrete(0,2);
	if (tirage==0) red_block_hold = 1, blue_block_hold = 0;
	else if (tirage==1) red_block_hold = 0, blue_block_hold = 1;
	else red_block_hold = 0, blue_block_hold = 0;
	std::vector<float> s = {agent_ns,agent_ew,red_block_hold,blue_block_hold};
	for (int i=0; i<objects.size();i++){
		s.push_back(objects[i].xpos);
		s.push_back(objects[i].ypos);
	}
	return s;
}

bool InfiniteBlocks::terminal() const {
	return false;
}

std::map<int, std::string> InfiniteBlocks::get_action_names(){
	return action_names;
}

bool InfiniteBlocks::isSyncTutor(std::vector<float> state) const {
	return (state[0]==*tutor_eye_ns && state[1]==*tutor_eye_ew && tutor_attentive);
}
int InfiniteBlocks::get_blocks_in() const {
	return red_box_count_blue+blue_box_count_blue+red_box_count_red+blue_box_count_red;
}

//int InfiniteBlocks::get_blocks_right() const {
//	int nb_blocks_right = 0;
//	for (auto block: blocks){
//		nb_blocks_right += (*block.is_in_blue_box && *block.color==1) || (*block.is_in_red_box && *block.color==0);
//	}
//	return nb_blocks_right;
//}


int InfiniteBlocks::getNumActions() {
	if (BRDEBUG) cout << "Return number of actions: " << numactions << endl;
	return numactions; //num_actions;
}

int InfiniteBlocks::getNumObjects(){
	return objects.size();
}

int InfiniteBlocks::getNumTutorActions() {
	if (BRDEBUG) cout << "Return number of tutor actions: " << num_tutor_actions << endl;
	return num_tutor_actions; //num_actions;
}

void InfiniteBlocks::getMinMaxFeatures(std::vector<float> *minFeat,
		std::vector<float> *maxFeat){

	minFeat->resize(objects.size()*2+4, 0.0);
	maxFeat->resize(objects.size()*2+4, size-1);

	(*maxFeat)[2] = 1;
	(*maxFeat)[3] = 1;

}

void InfiniteBlocks::getMinMaxReward(float *minR,
		float *maxR){

	*minR = 0.0;
	*maxR = 100.0;

}

void InfiniteBlocks::print_map(std::vector<float> &attention) const{

	int blockSize=80;
	Size sizeBlock(blockSize, blockSize);

	std::map<std::pair<int,int>,std::list<Mat>> posToImg;

	Mat chessBoard(blockSize*size,blockSize*size,CV_8UC3,Scalar::all(0));
	unsigned char color=0;
	string img_dir = "/home/pierre/workspace/myTexplore/images/";
	Mat red_block_img = imread(img_dir+"red_block.png",CV_LOAD_IMAGE_COLOR);
	resize(red_block_img, red_block_img, sizeBlock);
	Mat blue_block_img = imread(img_dir+"blue_block.png",CV_LOAD_IMAGE_COLOR);
	resize(blue_block_img, blue_block_img, sizeBlock);
	Mat blue_box_img = imread(img_dir+"blue_box.png",CV_LOAD_IMAGE_COLOR);
	resize(blue_box_img, blue_box_img, sizeBlock);
	Mat red_box_img = imread(img_dir+"red_box.png",CV_LOAD_IMAGE_COLOR);
	resize(red_box_img, red_box_img, sizeBlock);
	Mat agent_eye_img = imread(img_dir+"agent_eye.png",CV_LOAD_IMAGE_COLOR);
	resize(agent_eye_img, agent_eye_img, sizeBlock);
	Mat tutor_eye_img = imread(img_dir+"tutor_eye.png",CV_LOAD_IMAGE_COLOR);
	resize(tutor_eye_img, tutor_eye_img, sizeBlock);
	Mat agent_hand_img = imread(img_dir+"agent_hand.png",CV_LOAD_IMAGE_COLOR);
	resize(agent_hand_img, agent_hand_img, sizeBlock);



	for(int i=0;i<blockSize*size;i=i+blockSize){
		color=~color;
		for(int j=0;j<blockSize*size;j=j+blockSize){
			Mat ROI=chessBoard(Rect(i,j,blockSize,blockSize));
			ROI.setTo(Scalar::all(color));
			color=~color;
		}
	}


	if (red_block_hold){
		posToImg[std::pair<int,int>(blockSize*(agent_ew),blockSize*(agent_ns))].push_back(red_block_img);
	}
	if (blue_block_hold){
		posToImg[std::pair<int,int>(blockSize*(agent_ew),blockSize*(agent_ns))].push_back(blue_block_img);
	}
	posToImg[std::pair<int,int>(blockSize*(*blue_blocks_ew),blockSize*(*blue_blocks_ns))].push_back(blue_block_img);
	posToImg[std::pair<int,int>(blockSize*(*red_blocks_ew),blockSize*(*red_blocks_ns))].push_back(red_block_img);
	posToImg[std::pair<int,int>(blockSize*(*blue_box_ew),blockSize*(*blue_box_ns))].push_back(blue_box_img);
	posToImg[std::pair<int,int>(blockSize*(*red_box_ew),blockSize*(*red_box_ns))].push_back(red_box_img);
	position posEye = getAttDir(attention);
	posToImg[std::pair<int,int>(blockSize*(posEye.xpos),blockSize*(posEye.ypos))].push_back(agent_eye_img);

	if (WITH_TUTOR){
		posToImg[std::pair<int,int>(blockSize*(*tutor_eye_ew),blockSize*(*tutor_eye_ns))].push_back(tutor_eye_img);
	}

	posToImg[std::pair<int,int>(blockSize*(agent_ew),blockSize*(agent_ns))].push_back(agent_hand_img);

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
			int i = 0;
			while (!elem.second.empty()){
				Size size3(blockSize/4, blockSize/4);
				resize(elem.second.front(), elem.second.front(), size3);
				int x_decal = i % 4;
				int y_decal = i/4;
				elem.second.front().copyTo(chessBoard(cv::Rect(elem.first.first+x_decal*blockSize/4,
						elem.first.second+y_decal*blockSize/4,
						elem.second.front().cols,
						elem.second.front().rows)));
				elem.second.pop_front();
				i++;
			}
		}
	}

	imshow("Chess board", chessBoard);
	waitKey(1);
}

void InfiniteBlocks::setDebug(bool b){
	BRDEBUG = b;
}

void InfiniteBlocks::setVerbose(bool b){
	IS_REAL = b;
}

/*
std::vector<int> InfiniteBlocks::find_block_under_eye() {
	std::vector<int> l;
	int cnt = 0;
	for (auto block: blocks){
		if (*(block.ns)==*agent_eye_ns && *(block.ew)==*agent_eye_ew){
			if (!NOPICKBACK || (!(*(block.is_in_blue_box))&& !(*(block.is_in_red_box)))){
				l.push_back(cnt);
			}
		}
		cnt++;
	}
	return(l);
}

std::vector<int> InfiniteBlocks::find_red_block_under_hand() {
	std::vector<int> l;
	for (std::vector<block_t>::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (*(it->ns)==(agent_ns) && *(it->ew)==(agent_ew) && *(it->color)==RED){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}

std::vector<int> InfiniteBlocks::find_blue_block_under_hand() {
	std::vector<int> l;
	for (std::vector<block_t>::iterator it = blocks.begin(); it != blocks.end(); ++it){
		if (*(it->ns)==(agent_ns) && *(it->ew)==(agent_ew) && *(it->color)==BLUE){
			l.push_back(it-blocks.begin());
		}
	}
	return(l);
}
*/

position InfiniteBlocks::getAttDir(std::vector<float> attention) const{
	return objects[attention[0]];
}

bool InfiniteBlocks::eye_hand_sync(std::vector<float> attention){
	position pos = getAttDir(attention);
	return (pos.xpos==agent_ew && pos.ypos==agent_ns);
}

void InfiniteBlocks::apply_tutor(int action){
	if (action==actions["LOOK_AGENT"]){
		(*tutor_eye_ew) = (agent_ew);
		(*tutor_eye_ns) = (agent_ns);
	}
	if (action==tutor_actions["LOOK_RED_BOX"])	{
		(*tutor_eye_ew) = (*red_box_ew);
		(*tutor_eye_ns) = (*red_box_ns);
	}
	if (action==tutor_actions["LOOK_BLUE_BOX"]){
		(*tutor_eye_ew) = (*blue_box_ew);
		(*tutor_eye_ns) = (*blue_box_ns);
	}
	if (action==tutor_actions["LOOK_BLUE_BLOCKS"]){
		(*tutor_eye_ew) = (*blue_blocks_ew);
		(*tutor_eye_ns) = (*blue_blocks_ns);
	}
	if (action==tutor_actions["LOOK_RED_BLOCKS"]){
		(*tutor_eye_ew) = (*red_blocks_ew);
		(*tutor_eye_ns) = (*red_blocks_ns);
	}
}

std::vector<std::pair<int,int>> InfiniteBlocks::get_nearby_pos(int ns, int ew){
	std::vector<std::pair<int,int>> nearby_pos;
	if (ns<size-1){nearby_pos.push_back(std::make_pair(ns+1,ew));}
	if (ew<size-1){nearby_pos.push_back(std::make_pair(ns,ew+1));}
	if (ew>0){nearby_pos.push_back(std::make_pair(ns,ew-1));}
	if (ns>0){nearby_pos.push_back(std::make_pair(ns-1,ew));}
	return nearby_pos;
}



float InfiniteBlocks::getEuclidianDistance(std::vector<float> & s1, std::vector<float> & s2,
		std::vector<float> minValues, std::vector<float>maxValues){
	float res = 0.;
	unsigned nfeats = s1.size();
	std::vector<float> featRange(nfeats, 0);
	for (unsigned i = 0; i < nfeats; i++){
		featRange[i] = minValues[i] - maxValues[i];
	}
	if (s1.size()!=s2.size()){return -1;}
	for (int i=0; i<s1.size(); i++){
		res+=pow((s1[i]-s2[i])/featRange[i],2);
	}
	return sqrt(res/nfeats);
}

occ_info_t InfiniteBlocks::apply(int action, const std::vector<float> &attention){
	float reward = 0.;
	float tutor_reward = 0;
	bool success = false;
	float stoch_param = (stochastic ? 0.8 : 1.);


	if (action==actions["NORTH"]) {
		if ((agent_ns) < size-1) {
			(agent_ns)++;
			success = true;
		}
//		reward--;
	}
	if (action==actions["SOUTH"]) {
		if ((agent_ns) > 0) {
			(agent_ns)--;
			success = true;
		}
//		reward--;

	}
	if (action==actions["EAST"]) {
		if ((agent_ew) < size-1) {
			(agent_ew)++;
			success = true;
		}
//		reward--;

	}
	if (action==actions["WEST"]) {
		if ((agent_ew) > 0) {
			(agent_ew)--;
			success = true;
		}
//		reward--;

	}
	/*

	if (action==actions["GO_TO_EYE"]) {
		if (rng.bernoulli(stoch_param)) {
			(agent_ns) = (*agent_eye_ns);
			(agent_ew) = (*agent_eye_ew);
		}
		else {
			std::vector<std::pair<int,int>> nearby_pos = get_nearby_pos(*agent_eye_ns,*agent_eye_ew);
			std::shuffle(nearby_pos.begin(), nearby_pos.end(), engine);
			(agent_ns) = nearby_pos.front().first;
			(agent_ew) = nearby_pos.front().second;
		}
		success = true;
	}
	*/
	if (action == actions["PICK"]){
		if (!(red_block_hold) && !(blue_block_hold) && eye_hand_sync(attention)) {
			if (agent_ns==*red_blocks_ns && agent_ew==*red_blocks_ew) {
				if (rng.bernoulli(stoch_param)){
					red_block_hold = 1;
					success = true;
					if (IS_REAL) {
						std::cout << "Red block taken." << std::endl;
					}
				}
			}
			else if (agent_ns==*blue_blocks_ns && agent_ew==*blue_blocks_ew) {
				if (rng.bernoulli(stoch_param)){
					blue_block_hold = 1;
					success = true;
					if (IS_REAL) {
						std::cout << "Blue block taken." << std::endl;
					}
				}
			}
		}
//		reward--;

	}

	/*if (action==actions["PUT_DOWN"]) {
		std::vector<int> red_blocks_under = find_red_block_under_hand();
		std::vector<int> blue_blocks_under = find_blue_block_under_hand();
		if (((*block_hold)!=-1)
				&& eye_hand_sync()
				&& red_blocks_under.empty()
				&& blue_blocks_under.empty()
				&& ((*red_box_ns)!=(agent_ns) || (*red_box_ew)!=(agent_ew))
				&& ((*blue_box_ns)!=(agent_ns) || (*blue_box_ew)!=(agent_ew))){
			if (rng.bernoulli(stoch_param)){
	 *(blocks[(*block_hold)].ns) = (agent_ns);
	 *(blocks[(*block_hold)].ew) = (agent_ew);
			}
			else{
				std::vector<std::pair<int,int>> nearby_pos = get_nearby_pos(agent_ns,agent_ew);
				std::shuffle(nearby_pos.begin(), nearby_pos.end(), engine);
	 *(blocks[(*block_hold)].ns) = nearby_pos.front().first;
	 *(blocks[(*block_hold)].ew) = nearby_pos.front().second;
			}

			(*block_hold) = -1;
			success = true;
		}
	}*/

	if (action==actions["PUT_IN"]) {
		if (red_block_hold && eye_hand_sync(attention)){
			if (*red_box_ns==agent_ns && *red_box_ew==agent_ew){
				if (rng.bernoulli(stoch_param)){
					red_block_hold = 0;
					red_box_count_red++;
					reward += finalReward;
					tutor_reward += finalReward;
					success = true;
					if (IS_REAL){
						std::cout << "Red block put in red box." << std::endl;
					}
				}
			}
			else if (*blue_box_ns==agent_ns && *blue_box_ew==agent_ew){
				if (rng.bernoulli(stoch_param)){
					red_block_hold = 0;
					blue_box_count_red++;
					reward += finalReward;
					success = true;
					if (IS_REAL){
						std::cout << "Red block put in blue box." << std::endl;
					}
				}
			}
		}
		else if (blue_block_hold && eye_hand_sync(attention)){
			if (*red_box_ns==agent_ns && *red_box_ew==agent_ew){
				if (rng.bernoulli(stoch_param)){
					blue_block_hold = 0;
					red_box_count_blue++;
					reward += finalReward;
					success = true;
					if (IS_REAL){
						std::cout << "Blue block put in red box." << std::endl;
					}
				}
			}
			else if (*blue_box_ns==agent_ns && *blue_box_ew==agent_ew){
				if (rng.bernoulli(stoch_param)){
					blue_block_hold = 0;
					blue_box_count_blue++;
					reward += finalReward;
					success = true;
					if (IS_REAL){
						std::cout << "Blue block put in blue box." << std::endl;
					}
				}
			}
		}
//		reward--;

	}

	actions_occurences[action].push_back(numstep);
	numstep++;
	return occ_info_t(reward, success, 0, 0, tutor_reward);
}

//int InfiniteBlocks::trueBestAction(std::vector<float> attention){
//	int res = -1;
//	if (red_block_hold || blue_block_hold) {
//		if (((agent_ns==*red_box_ns && agent_ew==*red_box_ew)||
//				(agent_ns==*blue_box_ns && agent_ew==*blue_box_ew)) && eye_hand_sync(attention)){
//			res = actions["PUT_IN"];
//		}
//		else if (attention[2]!=1 && attention[3]!=1){
//			res = (rng.bernoulli(0.5) ? actions["LOOK_RED_BOX"] : actions["LOOK_BLUE_BOX"]);
//		}
//	}
//	else if (!(red_block_hold) && !(blue_block_hold)){
//		if (attention[0]!=1 && attention[1]!=1){
//			res = (rng.bernoulli(0.5) ? actions["LOOK_RED_BLOCKS"] : actions["LOOK_BLUE_BLOCKS"]);
//		}
//		else{
//			if (eye_hand_sync(attention)){
//				res = actions["PICK"];
//			}
//		}
//	}
//	if (res==-1){
//		res = actions["GO_TO_EYE"];
//	}
//	return res;
//}

tutor_feedback InfiniteBlocks::tutorAction(){
	float tutor_reward = 0.;
	float reward = 0.;
	int tutoract;
	if (red_block_hold) {tutoract = tutor_actions["LOOK_RED_BOX"];}
	if (blue_block_hold) {tutoract = tutor_actions["LOOK_BLUE_BOX"];}
	if (!(red_block_hold)&&!(blue_block_hold)){
		tutoract = tutor_actions["LOOK_RED_BLOCKS"];
	}

	return tutor_feedback(tutor_reward, reward, tutoract);
}

void InfiniteBlocks::tutorStop(){
	tutor_attentive = false;
}

std::vector<float> InfiniteBlocks::generate_state(){}
void InfiniteBlocks::reset(){}
std::pair<std::vector<float>,float> InfiniteBlocks::getMostProbNextState(std::vector<float> state, int action){}
float InfiniteBlocks::getStateActionInfoError(std::vector<float> s, std::vector<StateActionInfo> preds){}


