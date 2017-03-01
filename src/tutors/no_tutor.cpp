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

tutor_feedback no_tutor::next_action(const std::vector<float> &s, const int a) {

	float reward = 0.;
	if (previous_action == 1 || previous_action == 2){
		if (a==5 && s[0]==s[9] && s[1]==s[10]){reward+=1;}
	}

	int act;
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
	return tutor_feedback(reward, act);
}

bool no_tutor::red_box_ok(const std::vector<float> &s){
	bool no_error = true;
	int idx = 0;
	while (no_error && idx<numactions-3){
		no_error = (s[6*idx+2+12]==0 || s[6*idx+5+12]==0);
		idx++;
	}
	return no_error;
}

bool no_tutor::blue_box_ok(const std::vector<float> &s){
	bool no_error = true;
	int idx = 0;
	while (no_error && idx<numactions-3){
		no_error = (s[6*idx+2+12]==1 || s[6*idx+4+12]==0);
		idx++;
	}
	return no_error;
}


