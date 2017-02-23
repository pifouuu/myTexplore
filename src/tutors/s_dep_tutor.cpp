/*
 * state_dependent.cpp
 *
 *  Created on: 21 févr. 2017
 *      Author: pierre
 */

#include "../../include/tutors/s_dep_tutor.hpp"

s_dep_tutor::s_dep_tutor(int numactions):
	numactions(numactions)
{
	ACTDEBUG = false;
};

s_dep_tutor::~s_dep_tutor() {};

int s_dep_tutor::first_action(const std::vector<float> &s) {
	if (ACTDEBUG){
		std::cout<< "Tutor observe state :";
		printState(s);
		std::cout << std::endl;
	}

	int act;
	if (s[2]>=0){
		int idx = s[2];
		int color = s[6*idx+2+12];
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
			bool found_block = false;
			int test = 0;
			do {
				found_block = (s[6*test+12+4]==0 && s[6*test+12+5]==0);
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
	previous_action = act;

	if (ACTDEBUG)
		cout << "Took action " << act << endl;
	return act;
}

int s_dep_tutor::next_action(const std::vector<float> &s) {
	if (ACTDEBUG){
		std::cout<< "Tutor observe state :";
		printState(s);
		std::cout << std::endl;
	}

	int act;
	if (s[2]>=0){
		int idx = s[2];
		int color = s[6*idx+2+12];
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
					found_block = (s[6*test+12+4]==0 && s[6*test+12+5]==0);
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

	if (ACTDEBUG){
		cout << "Took action " << act << " from state "
				<< (s)[0];
		for (unsigned i = 1; i < s.size(); i++){
			cout << "," << (s)[i];
		}
		cout << endl;
	}
	return act;
}

bool s_dep_tutor::red_box_ok(const std::vector<float> &s){
	bool no_error = true;
	int idx = 0;
	while (no_error && idx<numactions-3){
		no_error = (s[6*idx+2+12]==0 || s[6*idx+5+12]==0);
		idx++;
	}
	return no_error;
}

bool s_dep_tutor::blue_box_ok(const std::vector<float> &s){
	bool no_error = true;
	int idx = 0;
	while (no_error && idx<numactions-3){
		no_error = (s[6*idx+2+12]==1 || s[6*idx+4+12]==0);
		idx++;
	}
	return no_error;
}

void s_dep_tutor::setDebug(bool d){
  ACTDEBUG = d;
}

void s_dep_tutor::printState(const std::vector<float> &s){
  for (unsigned j = 0; j < s.size(); j++){
    cout << s[j] << ", ";
  }
}
