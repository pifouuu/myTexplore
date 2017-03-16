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


void s_dep_tutor::setDebug(bool d){
  ACTDEBUG = d;
}

void s_dep_tutor::printState(const std::vector<float> &s){
  for (unsigned j = 0; j < s.size(); j++){
    cout << s[j] << ", ";
  }
}
