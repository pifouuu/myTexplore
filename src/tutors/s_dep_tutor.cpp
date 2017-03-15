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

void s_dep_tutor::setTrueEnv(Environment* e){

	trueEnv = e;
}

s_dep_tutor::~s_dep_tutor() {};


bool s_dep_tutor::red_box_ok(const std::vector<float> &s){
	bool no_error = true;
	int idx = 0;
	while (no_error && idx<numactions-3){
		no_error = (s[5*idx+2+11]==0 || s[6*idx+4+11]==0);
		idx++;
	}
	return no_error;
}

bool s_dep_tutor::blue_box_ok(const std::vector<float> &s){
	bool no_error = true;
	int idx = 0;
	while (no_error && idx<numactions-3){
		no_error = (s[5*idx+2+11]==1 || s[5*idx+3+11]==0);
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
