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


void no_tutor::setTrueEnv(Environment* e){

	trueEnv = e;
}

no_tutor::~no_tutor() {};


