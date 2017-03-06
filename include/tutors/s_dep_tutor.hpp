/*
 * state_dependent.hpp
 *
 *  Created on: 21 févr. 2017
 *      Author: pierre
 */

#ifndef INCLUDE_TUTORS_S_DEP_TUTOR_HPP_
#define INCLUDE_TUTORS_S_DEP_TUTOR_HPP_

#include "../common/core.hh"
#include <vector>


class s_dep_tutor: public Tutor {
public:
	s_dep_tutor(int numactions);

	virtual ~s_dep_tutor();

	virtual void setDebug(bool d);
	tutor_feedback first_action(const std::vector<float> &s);
	tutor_feedback next_action(const std::vector<float> &s, const int a);

	void printState(const std::vector<float> &s);
	bool red_box_ok(const std::vector<float> &s);
	bool blue_box_ok(const std::vector<float> &s);

private:
	bool ACTDEBUG;
	const int numactions;
	int previous_action;
};



#endif /* INCLUDE_TUTORS_S_DEP_TUTOR_HPP_ */