/*
 * complete_linear_movement.h
 *
 *  Created on: 18 nov. 2015
 *      Author: guillaume
 */

#ifndef COMPLETE_LINEAR_MOVEMENT_H_
#define COMPLETE_LINEAR_MOVEMENT_H_

#include "Movement.h"

class Paw;

class complete_linear_movement : public Movement
{
public:
	complete_linear_movement(float angle, float distance, int step_number);
	virtual ~complete_linear_movement() {}

	void determine_x_paws_position(Paw &paw);
	void determine_y_paws_position(Paw &paw);
	void determine_z_paws_position(Paw &paw);

	virtual float determine_real_distance(Paw &paw);
	virtual void compute_variables(Paw & paw);
	virtual float* determine_paw_position(Paw &paw);
	virtual bool is_sequence_finished(Paw &paw);
};

#endif /* COMPLETE_LINEAR_MOVEMENT_H_ */
