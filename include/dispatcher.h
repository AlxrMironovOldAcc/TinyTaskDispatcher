
#ifndef DISPATCHER_H_
#define DISPATCHER_H_

	#include <stdint.h>
	#include <stdbool.h>
	
	#define D_QUEUE_SIZE			30
	#define	D_DISPATCHER_TICK_MS	10
	
	#define D_DISABLE_IRQ			/* USER CODE */
	#define D_ENABLE_IRQ			/* USER CODE */
	#define D_MS_TO_TICKS(ms)		( (ms/D_DISPATCHER_TICK_MS)? (ms/D_DISPATCHER_TICK_MS) : 1);

	typedef					void (*d_task_fptr)(void);
	typedef uint_fast16_t	d_period;
	
	typedef struct{
		d_period	delay;
		d_task_fptr	task;
	}d_periodic_task_str;

	/**
	 *	Dispatcher initialization
	 */
	extern void D_Init(void);

	/**
	 *	Dispatcher tick source
	 *	Registrate dispacther tick (not perform any another opertions)
	 *	Call it from dispatcher clock source with D_DISPATCHER_TICK_MS period.
	 */
	extern void D_RegisterTick(void);
	
	/**
	 *	Dispatcher handler
	 *  Should called in infinity cycle with freq > D_DISPATCHER_TICK_MS.
	 *  Sort, handle tasks queues, call ready tasks inside.
	 */			
	extern void D_Handler(void);

	/**
	 *	Add task to infinity execution queue with given period
	 *	@param - Task pointer  
	 *	@param - Period (in dispatcher ticks)
	 *	@return - BOOL, success
	 */
	extern bool D_SetPeriodicalTask(register d_task_fptr task, register d_period period);
	
	/**
	 *	Delete task from execution queue
	 *	@param - Task pointer 
	 *	@return - INT deleted tasks count
	 */
	extern uint_fast8_t D_ClearTasks(register d_task_fptr task);

#endif /* SYSTEM_H_ */

