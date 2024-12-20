/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : task_menu.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				50ul
#define DEL_MEN_XX_MAX				50ul

#define MOTOR_ID_MAX				2ul
#define SUBMENU_ID_MAX				3ul
//#define POWER_ID_DEFAULT			false
#define SPEED_ID_MAX				9ul
//#define SPIN_ID_DEFAULT				false



/********************** internal data declaration ****************************/
task_menu_dta_t task_menu_dta =
	{DEL_MEN_XX_MIN, ST_MEN_MAIN, EV_MEN_MEN_IDLE, false};

task_menu_parameters_t task_menu_parameters_list[] = {
		{1, 1, false, 0, false},
		{2, 1, false, 0, false},
};


#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_menu 		= "Task Menu (Interactive Menu)";
const char *p_task_menu_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_menu_cnt;
volatile uint32_t g_task_menu_tick_cnt;

/********************** external functions definition ************************/
void task_menu_init(void *parameters)
{
	task_menu_dta_t *p_task_menu_dta;
	task_menu_st_t	state;
	task_menu_ev_t	event;
	bool b_event;

		/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_menu_init), p_task_menu);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_menu), p_task_menu_);

	g_task_menu_cnt = G_TASK_MEN_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_menu_cnt), g_task_menu_cnt);

	init_queue_event_task_menu();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_menu_dta = &task_menu_dta;

	/* Print out: Task execution FSM */
	state = p_task_menu_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_menu_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_menu_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

    displayInit( DISPLAY_CONNECTION_GPIO_4BITS );

    displayCharPositionWrite(0, 0);
	displayStringWrite("Motor_main_1");

	displayCharPositionWrite(0, 1);
	displayStringWrite("Motor_main_2");

	HAL_GPIO_WritePin(LED_A_PORT, LED_A_PIN, LED_A_ON);

	g_task_menu_tick_cnt = G_TASK_MEN_TICK_CNT_INI;

}

void task_menu_update(void *parameters)
{
	task_menu_dta_t *p_task_menu_dta;
	task_menu_parameters_t *p_task_menu_parameters;
	bool b_time_update_required = false;
	uint32_t index;
	//char menu_str1[18];
	//char menu_str2[18];

	p_task_menu_parameters = task_menu_parameters_list;

	/* Update Task Menu Counter */
	g_task_menu_cnt++;

	/* Protect shared resource (g_task_menu_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
    {
    	g_task_menu_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_menu_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
		{
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Menu Data Pointer */
		p_task_menu_dta = &task_menu_dta;


    	if (DEL_MEN_XX_MIN < p_task_menu_dta->tick)
		{
			p_task_menu_dta->tick--;
		}
		else
		{
			HAL_GPIO_TogglePin(LED_A_PORT, LED_A_PIN);

			//snprintf(menu_str, sizeof(menu_str), "%lu", (g_task_menu_cnt/1000ul));
			//displayCharPositionWrite(10, 1);
			//displayStringWrite(menu_str);

			p_task_menu_dta->tick = DEL_MEN_XX_MAX;

			if (true == any_event_task_menu())
			{
				p_task_menu_dta->flag = true;
				p_task_menu_dta->event = get_event_task_menu();
			}

			switch (p_task_menu_dta->state)
			{
				case ST_MEN_MAIN:



					if ((true == p_task_menu_dta->flag) && (EV_MEN_MEN_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_01_ACTIVE;

						displayCharPositionWrite(0, 0);
						displayStringWrite("Enter/Next/Escape");

						displayCharPositionWrite(0, 1);
						displayStringWrite("Motor 1        ");


					}


					break;

				case ST_MEN_01_ACTIVE:

					if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						index = p_task_menu_parameters->motor_id;

						if (index == MOTOR_ID_MAX){
						//p_task_menu_dta->state = ST_MEN_01_ACTIVE;
							index = 0;
							displayCharPositionWrite(0, 1);
							displayStringWrite("Motor 1        ");
						}
						else{
							displayCharPositionWrite(0, 1);
							displayStringWrite("Motor 2        ");
							index++;
						}
						p_task_menu_parameters = &task_menu_parameters_list[index];



					}
					else if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_02_ACTIVE;

						displayCharPositionWrite(0, 1);
						displayStringWrite("Power        ");

					}
					else if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;


						displayCharPositionWrite(0, 0);
						displayStringWrite("Motor_main_1          ");

						displayCharPositionWrite(0, 1);
						displayStringWrite("Motor_main_2          ");

					}

					break;


				case ST_MEN_02_ACTIVE:

					if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						if (p_task_menu_parameters->submenu == 1){
						//p_task_menu_dta->state = ST_MEN_01_ACTIVE;
							p_task_menu_parameters->submenu = 2;
							//memcpy(menu_str2,"Speed", sizeof("Speed"));
							displayCharPositionWrite(0, 1);
							displayStringWrite("Speed        ");

						}
						else if (p_task_menu_parameters->submenu == 2){
							p_task_menu_parameters->submenu = 3;
							//memcpy(menu_str2,"Spin", sizeof("Spin"));
							displayCharPositionWrite(0, 1);
							displayStringWrite("Spin        ");
						}
						else if (p_task_menu_parameters->submenu == 3){
							p_task_menu_parameters->submenu = 1;
							//memcpy(menu_str2,"Power", sizeof("Power"));
							displayCharPositionWrite(0, 1);
							displayStringWrite("Power        ");
						}
					}
					else if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						switch (p_task_menu_parameters->submenu){
							case 1:
								p_task_menu_dta->state = ST_MEN_03_POWER;
								//memcpy(menu_str2,"ON", sizeof("ON"));
								displayCharPositionWrite(0, 1);
								displayStringWrite("ON        ");
								break;
							case 2:
								p_task_menu_dta->state = ST_MEN_03_SPEED;
								//memcpy(menu_str2,"0", sizeof("0"));
								displayCharPositionWrite(0, 1);
								displayStringWrite("0          ");
								break;
							case 3:
								p_task_menu_dta->state = ST_MEN_03_SPIN;
								//memcpy(menu_str2,"L", sizeof("L"));
								displayCharPositionWrite(0, 1);
								displayStringWrite("LEFT          ");
								break;
							default:
								break;
						}

					}
					else if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_01_ACTIVE;
						//memcpy(menu_str2,"Motor 1", sizeof("Motor 1"));
						displayCharPositionWrite(0, 1);
						displayStringWrite("Motor 1       ");
					}
					break;

				case ST_MEN_03_POWER:

					if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						if (p_task_menu_parameters->power == false){
							p_task_menu_parameters->power = true;
							//memcpy(menu_str2,"ON", sizeof("ON"));
							displayCharPositionWrite(0, 1);
							displayStringWrite("ON       ");
						}
						else{
							p_task_menu_parameters->power = false;
							//memcpy(menu_str2,"OFF", sizeof("OFF"));
							displayCharPositionWrite(0, 1);
							displayStringWrite("OFF        ");
						}
					}
					else if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
						//memcpy(menu_str1,"Motor_main_1", sizeof("Motor_main_1"));
						//memcpy(menu_str2,"Motor_main_2", sizeof("Motor_main_2"));

						displayCharPositionWrite(0, 0);
						displayStringWrite("Motor_main_1          ");

						displayCharPositionWrite(0, 1);
						displayStringWrite("Motor_main_2     ");
					}
					else if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_02_ACTIVE;
						//memcpy(menu_str2,"Power", sizeof("Power"));
						displayCharPositionWrite(0, 1);
						displayStringWrite("Power           ");
					}
					break;


				case ST_MEN_03_SPEED:

						if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
						{
							//memcpy(menu_str2,"", sizeof(""));
							//p_task_menu_dta->flag = false;
							if (p_task_menu_parameters->speed == SPEED_ID_MAX){
								p_task_menu_parameters->speed = 0;
								//snprintf(menu_str2 + strlen(menu_str2), sizeof(menu_str2) - strlen(menu_str2), "%lu", p_task_menu_parameters->speed);
								displayCharPositionWrite(0, 1);
								displayStringWrite("0     ");
							}
							else{
								p_task_menu_parameters->speed++;
								char speed[5] = "";
								snprintf(speed + strlen(speed), sizeof(speed) - strlen(speed), "%lu", p_task_menu_parameters->speed);
								displayCharPositionWrite(0, 1);
								displayStringWrite(speed);
							}
						}
						else if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
						{
							p_task_menu_dta->flag = false;
							p_task_menu_dta->state = ST_MEN_MAIN;
							displayCharPositionWrite(0, 0);
							displayStringWrite("Motor_main_1     ");

							displayCharPositionWrite(0, 1);
							displayStringWrite("Motor_main_2     ");
						}
						else if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
						{
							p_task_menu_dta->flag = false;
							p_task_menu_dta->state = ST_MEN_02_ACTIVE;

							displayCharPositionWrite(0, 1);
							displayStringWrite("Speed     ");
						}
						break;

				case ST_MEN_03_SPIN:

						if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
						{
							p_task_menu_dta->flag = false;
							if (p_task_menu_parameters->spin == false){
								p_task_menu_parameters->spin = true;
								//memcpy(menu_str2,"Right", sizeof("Right"));
								displayCharPositionWrite(0, 1);
								displayStringWrite("RIGHT     ");
							}
							else{
								p_task_menu_parameters->spin = false;
								//memcpy(menu_str2,"Left", sizeof("Left"));

								displayCharPositionWrite(0, 1);
								displayStringWrite("LEFT     ");
							}
						}
						else if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
						{
							p_task_menu_dta->flag = false;
							p_task_menu_dta->state = ST_MEN_MAIN;
							//memcpy(menu_str1,"Motor_main_1", sizeof("Motor_main_1"));
							//memcpy(menu_str2,"Motor_main_2", sizeof("Motor_main_2"));

							displayCharPositionWrite(0, 0);
							displayStringWrite("Motor_main_1     ");

							displayCharPositionWrite(0, 1);
							displayStringWrite("Motor_main_2     ");
						}
						else if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
						{
							p_task_menu_dta->flag = false;
							p_task_menu_dta->state = ST_MEN_02_ACTIVE;
							//memcpy(menu_str2,"Spin", sizeof("Spin"));

							displayCharPositionWrite(0, 1);
							displayStringWrite("Spin     ");
						}
						break;
				default:

					p_task_menu_dta->tick  = DEL_MEN_XX_MIN;
					p_task_menu_dta->state = ST_MEN_MAIN;
					p_task_menu_dta->event = EV_MEN_MEN_IDLE;
					p_task_menu_dta->flag  = false;
					//memcpy(menu_str1,"Motor_main_1", sizeof("Motor_main_1"));
					//memcpy(menu_str2,"Motor_main_2", sizeof("Motor_main_2"));

					break;
			}

			//displayCharPositionWrite(0, 0);
			//displayStringWrite(menu_str1);
			//displayCharPositionWrite(0, 1);
			//displayStringWrite(menu_str1);
		}
	}
}

/********************** end of file ******************************************/

