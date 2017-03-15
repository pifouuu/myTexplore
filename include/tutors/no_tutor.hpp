/*
 * no_tutor.hpp
 *
 *  Created on: 28 févr. 2017
 *      Author: pierre
 */

#ifndef INCLUDE_TUTORS_NO_TUTOR_HPP_
#define INCLUDE_TUTORS_NO_TUTOR_HPP_
#include "../common/core.hh"

class no_tutor: public Tutor {
public:
	no_tutor(int numactions);

	virtual ~no_tutor();
	tutor_feedback first_action(const std::vector<float> &s);
	tutor_feedback next_action(const std::vector<float> &s, const int a);
	void setTrueEnv(Environment* e);

	bool red_box_ok(const std::vector<float> &s);
	bool blue_box_ok(const std::vector<float> &s);
	Environment* trueEnv;

private:
	bool ACTDEBUG;
	const int numactions;
	int previous_action;
};



#endif /* INCLUDE_TUTORS_NO_TUTOR_HPP_ */
