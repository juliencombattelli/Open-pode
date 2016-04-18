//============================================================================
// Name        : Hexapode.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "Hexapode.h"
#include "bcm2835.h"
#include "Linear_movement.h"
#include "No_movement.h"
#include "complete_linear_movement.h"
#include <cmath>

const int SEQUENCE_NUMBER = 3;

Hexapode::Hexapode() : m_i2c(new i2cdev),
					   m_left_side(side_left, m_i2c), m_right_side(side_right, m_i2c),
					   m_movement(nullptr), // init to no_movement
					   m_current_sequence_number(0), //remettre -1
					   m_current_step_number(0),
					   m_step_number(1),
					   m_paw_spreading(80),
					   m_center_height(-50)

{
	bcm2835_gpio_clr(7);
}

Hexapode::~Hexapode()
{
	delete m_i2c;
	delete m_movement;
}

void Hexapode::run()
{
	m_timer.reset();

	move(new complete_linear_movement( 0, 40, 20));

	toggle();

	while(1)
	{
		m_controller.process_input();

		if(m_timer.elapsed().millis() >= 20.0)
		{
			m_timer.reset();

			if((abs(m_controller.m_jsl_x_value) <= 2000) and (abs(m_controller.m_jsl_y_value) <= 2000))
			{
				move(new No_movement());
			}
			else if(((m_current_step_number <= 1) && (m_movement->m_type == no_movement)) || (m_movement->m_type != no_movement))
			{
				int step_number = abs(140 - sqrt(m_controller.m_jsl_y_value*m_controller.m_jsl_y_value + m_controller.m_jsl_x_value*m_controller.m_jsl_x_value)/32000.*140.+12);
				if(step_number < 12)
					step_number = 12;

				move(new complete_linear_movement(  atan2(m_controller.m_jsl_x_value , m_controller.m_jsl_y_value)*180/M_PI,
													40, step_number));
			}

			if(m_controller.m_is_r2_press)
				m_paw_spreading += 0.2;
			else if(m_controller.m_is_r1_press)
				m_paw_spreading -= 0.2;

			if(m_controller.m_is_l2_press)
				m_center_height += 0.5;
			else if(m_controller.m_is_l1_press)
				m_center_height -= 0.5;

			if(!update(((m_center_height+30.) / (110. + 80.)) * (m_controller.m_jsr_y_value/32767.), m_center_height, m_paw_spreading))
				toggle();
		}
	}
}

void Hexapode::determine_real_distance_for_movement()
{
	double real_distance_left  = m_left_side.get_real_distance();
	double real_distance_right = m_right_side.get_real_distance();
	double min_distance = min(real_distance_left, real_distance_right);

	if(min_distance != (m_movement->m_distance / 2))
		m_movement->m_corrected_distance = min_distance;
	else
		m_movement->m_corrected_distance = m_movement->m_distance;
	m_movement->compute_variables();
}

void Hexapode::toggle()
{

	m_current_sequence_number++;
	if(m_current_sequence_number >= SEQUENCE_NUMBER)
		m_current_sequence_number = 0;

	m_current_step_number = 0;
	m_left_side.memorize_current_paw_position();
	m_right_side.memorize_current_paw_position();

	m_left_side.change_sequence_number(m_current_sequence_number, m_current_step_number);
	m_right_side.change_sequence_number(m_current_sequence_number, m_current_step_number);

	determine_real_distance_for_movement();

}

void Hexapode::move(Movement *mvt)
{
	/* TODO :
	 * 	compute the distance for each side for circular mvt
	 */
	if(m_movement != nullptr)//for init
		delete m_movement;
	else
		m_step_number =  mvt->m_step_number;

	m_movement = mvt;

	if(m_step_number != m_movement->m_step_number)//if the previous isn't the same of the new one
	{
		//create an approximation of the current step number
		m_current_step_number = m_movement->m_step_number * m_current_step_number / m_step_number;
		m_step_number = m_movement->m_step_number;
	}
	m_left_side.memorize_movement(mvt, m_current_step_number);
	m_right_side.memorize_movement(mvt, m_current_step_number);

	determine_real_distance_for_movement();
}

int Hexapode::update(double a, double b, double paw_spreading)
{
	int result_right, result_left;
	result_right = m_right_side.update(m_current_sequence_number, a, b, paw_spreading);
	result_left  = m_left_side.update(m_current_sequence_number, a, b, paw_spreading);
	m_current_step_number ++;
	m_movement->m_current_step_number = m_current_step_number;
	return result_right & result_left;
}

//for calibration
void Hexapode::calibrate_servomotors(double x, double y, double z)
{
	Angles angles;
	int time3, time2, time1;

	angles = m_left_side.get_front_paw().move(x, y, z);
	time3 = static_cast<int>( -(angles.theta3*(180./3.14159)+90)*1.97 + m_left_side.get_front_paw().m_tibia.get_offset());
	time2 = static_cast<int>( angles.theta2*(180./3.14159)*1.97     + m_left_side.get_front_paw().m_femur.get_offset());
	time1 = static_cast<int>(-(angles.theta1*(180./3.14159)-90)*1.97 + m_left_side.get_front_paw().m_coxa.get_offset());
	m_left_side.get_module().set_off_time(channel0, time3);
	m_left_side.get_module().set_off_time(channel1, time2);
	m_left_side.get_module().set_off_time(channel2, time1);

	angles = m_left_side.get_front_paw().move(x, y, z);
	time3 = static_cast<int>( -(angles.theta3*(180./3.14159)+90)*1.97 + m_left_side.get_middle_paw().m_tibia.get_offset());
	time2 = static_cast<int>( angles.theta2*(180./3.14159)*1.97     + m_left_side.get_middle_paw().m_femur.get_offset());
	time1 = static_cast<int>(-(angles.theta1*(180./3.14159)-90)*1.97 + m_left_side.get_middle_paw().m_coxa.get_offset());
	m_left_side.get_module().set_off_time(channel3, time3);
	m_left_side.get_module().set_off_time(channel4, time2);
	m_left_side.get_module().set_off_time(channel5, time1);

	angles = m_left_side.get_front_paw().move(x, y, z);
	time3 = static_cast<int>( -(angles.theta3*(180./3.14159)+90)*1.97 + m_left_side.get_back_paw().m_tibia.get_offset());
	time2 = static_cast<int>( angles.theta2*(180./3.14159)*1.97     + m_left_side.get_back_paw().m_femur.get_offset());
	time1 = static_cast<int>(-(angles.theta1*(180./3.14159)-90)*1.97 + m_left_side.get_back_paw().m_coxa.get_offset());
	m_left_side.get_module().set_off_time(channel6, time3);
	m_left_side.get_module().set_off_time(channel7, time2);
	m_left_side.get_module().set_off_time(channel8, time1);

	angles = m_right_side.get_front_paw().move(x, -y, z);
	time3 = static_cast<int>( (angles.theta3*(180./3.14159)+90)*1.97 + m_right_side.get_front_paw().m_tibia.get_offset());
	time2 = static_cast<int>( -angles.theta2*(180./3.14159)*1.97     + m_right_side.get_front_paw().m_femur.get_offset());
	time1 = static_cast<int>(-(angles.theta1*(180./3.14159)+90)*1.97 + m_right_side.get_front_paw().m_coxa.get_offset());
	m_right_side.get_module().set_off_time(channel0, time3);
	m_right_side.get_module().set_off_time(channel1, time2);
	m_right_side.get_module().set_off_time(channel2, time1);

	angles = m_right_side.get_front_paw().move(x, -y, z);
	time3 = static_cast<int>( (angles.theta3*(180./3.14159)+90)*1.97 + m_right_side.get_middle_paw().m_tibia.get_offset());
	time2 = static_cast<int>( -angles.theta2*(180./3.14159)*1.97     + m_right_side.get_middle_paw().m_femur.get_offset());
	time1 = static_cast<int>(-(angles.theta1*(180./3.14159)+90)*1.97 + m_right_side.get_middle_paw().m_coxa.get_offset());
	m_right_side.get_module().set_off_time(channel3, time3);
	m_right_side.get_module().set_off_time(channel4, time2);
	m_right_side.get_module().set_off_time(channel5, time1);

	angles = m_right_side.get_front_paw().move(x, -y, z);
	time3 = static_cast<int>( (angles.theta3*(180./3.14159)+90)*1.97 + m_right_side.get_back_paw().m_tibia.get_offset());
	time2 = static_cast<int>( -angles.theta2*(180./3.14159)*1.97     + m_right_side.get_back_paw().m_femur.get_offset());
	time1 = static_cast<int>(-(angles.theta1*(180./3.14159)+90)*1.97 + m_right_side.get_back_paw().m_coxa.get_offset());
	m_right_side.get_module().set_off_time(channel6, time3);
	m_right_side.get_module().set_off_time(channel7, time2);
	m_right_side.get_module().set_off_time(channel8, time1);
}

