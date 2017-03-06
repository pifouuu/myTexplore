/*
 * no_tutor.cpp
 *
 *  Created on: 21 févr. 2017
 *      Author: pierre
 */

#include "../../include/tutors/no_tutor.hpp"

no_tutor::no_tutor(int numactions):
	numactions(numactions)
{
	ACTDEBUG = false;
};

no_tutor::~no_tutor() {};

tutor_feedback no_tutor::first_action(const std::vector<float> &s){};

/*int no_tutor::get_blocks_right(const std::vector<float> &s) {
	int nb_blocks_right = 0;
	for (int i = 9; i<s.size();i+=6){
		nb_blocks_right += ((s[i+2]==0 && s[i+5]==1) || (s[i+2]==1 && s[i+4]==1));
	}
	return nb_blocks_right;
}

bool no_tutor::is_tutor_terminal(){
	return get_blocks_right()==nbBlueBlocks+nbRedBlocks;
}*/

tutor_feedback no_tutor::next_action(const std::vector<float> &s, const int a) {

	float reward = 0.;
	if (s[2]>=0){
		int idx = s[2];
		int color = s[5*idx+9+2];
		/*if (is_tutor_terminal()){
			reward =+ 10;
		}
		else*/
		if (color == 1 && a ==3 && s[0]==s[7] && s[1]==s[8] ) {reward += 1;}
		if (color == 0 && a ==3 && s[0]==s[5] && s[1]==s[6] ) {reward += 1;}

	}

	/*int act;
	if (s[2]>=0){
		int idx = s[2];
		int color = s[6*idx+2+11];
		if (color == 1) {act = 2;}
		else if (color == 0) {act = 1;}
	}
	else {
		if (!red_box_ok(s)) {
			act = 1;
		}
		else if (!blue_box_ok(s)) {
			act = 2;
		}
		else {
			if (previous_action>2 && previous_action<numactions) {
				act = previous_action;
			}
			else {
				bool found_block = false;
				int test = 0;
				do {
					found_block = (s[6*test+11+4]==0 && s[6*test+11+5]==0);
					test++;
				} while (!found_block && test<numactions-2);
				if (test==numactions-2){
					std::cout << "All blocks in place !" << std::endl;
					act = 0;
				}
				else {
					act = test-1+3;
				}
			}
		}
	}

	previous_action = act;
	*/
	return tutor_feedback(reward, 0);
}

/* WILL NOT WORK !!!!!!!! */
//bool no_tutor::red_box_ok(const std::vector<float> &s){
//	bool no_error = true;
//	int idx = 0;
//	while (no_error && idx<numactions-3){
//		no_error = (s[5*idx+2+12]==0 || s[5*idx+5+12]==0);
//		idx++;
//	}
//	return no_error;
//}
//
//bool no_tutor::blue_box_ok(const std::vector<float> &s){
//	bool no_error = true;
//	int idx = 0;
//	while (no_error && idx<numactions-3){
//		no_error = (s[6*idx+2+12]==1 || s[6*idx+4+12]==0);
//		idx++;
//	}
//	return no_error;
//}


