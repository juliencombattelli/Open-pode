/*
 * PS4Controller.h
 *
 *  Created on: 23 nov. 2015
 *      Author: Julien
 */

#ifndef PS4CONTROLLER_H_
#define PS4CONTROLLER_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

enum PS4_Key
{
    KEY_CROSS       = BTN_B,
    KEY_CIRCLE      = BTN_C,
    KEY_SQUARE      = BTN_A,
    KEY_TRIANGLE    = BTN_X,

	// arbitrary values to avoid collision with other enumerators
    KEY_DPAD_UP     = 0xFFFC,
    KEY_DPAD_DOWN   = 0xFFFD,
    KEY_DPAD_LEFT   = 0xFFFE,
    KEY_DPAD_RIGHT  = 0xFFFF,

    KEY_R1          = BTN_Z,
    KEY_R2          = BTN_TR,
    KEY_R3          = BTN_START,

    KEY_L1          = BTN_Y,
    KEY_L2          = BTN_TL,
    KEY_L3          = BTN_SELECT,

    KEY_PS          = BTN_MODE,
    KEY_OPTIONS     = BTN_TR2,
    KEY_SHARE       = BTN_TL2
};

enum PS4_Key_event_type
{
    Key_release,
    Key_press,
    Key_autorepeat
};

class PS4Controller
{
public:

	PS4Controller();

	void process_input();

	bool is_key_press(PS4_Key key) const;

	int get_js_left_x() const { return m_jsl_x_value; }
	int get_js_left_y() const { return m_jsl_y_value; }
	int get_js_right_x() const { return m_jsr_x_value; }
	int get_js_right_y() const { return m_jsr_y_value; }

	static const std::string device_ev;
	static const std::string device_js;

	static const float max_stick_value;
	static const float stick_offset;

private:

	int fdopen(std::string device, int mode);

	bool m_is_cross_press;
	bool m_is_square_press;
	bool m_is_triangle_press;
	bool m_is_circle_press;

	bool m_is_up_press;
	bool m_is_down_press;
	bool m_is_left_press;
	bool m_is_right_press;

	bool m_is_r1_press;
	bool m_is_r2_press;
	bool m_is_r3_press;

	bool m_is_l1_press;
	bool m_is_l2_press;
	bool m_is_l3_press;

	bool m_is_ps_press;
	bool m_is_share_press;
	bool m_is_options_press;

	int m_jsl_x_value;
	int m_jsl_y_value;

	int m_jsr_x_value;
	int m_jsr_y_value;

	input_event m_ev;
	js_event m_js;

	int m_fd_ev;
	int m_fd_js;

	int m_rd_ev;
	int m_rd_js;
};



#endif /* PS4CONTROLLER_H_ */
