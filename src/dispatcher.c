
	#include <stdbool.h>
	#include <string.h>
	#include <iso646.h>

	#include "dispatcher.h"	
	
	/**************************************************************
					LOCAL FUNCTIONS DECLARATION
	**************************************************************/
	
	/**
	 *	Dercrement task delays
	 *	@param - NONE
	 *	@return - NONE
	 */
	static inline void	_DecrementDelays(void);
	
	/**
	 *	Execute first task with expired timer
	 *	@param - NONE
	 *	@return - NONE
	 */
	static inline void	_ExecuteTask(void);
	
	/**
	 *	Queue service handling (sorting and etc.)
	 *	@param - NONE
	 *	@return - NONE
	 */
	static inline void	_QueueService(void);
	
	/**
	 *	Increment queue LEN variable
	 *	@param - NONE
	 *	@return - NONE
	 */
	static inline void	_IncLen(void);
	
	/**************************************************************
					LOCAL VARIABLES & TYPES
	**************************************************************/
	
	typedef enum {
		D_STEP_DECREMENT_DELAYS,
		D_STEP_TASK_EXECUTION,
		D_STEP_QUEUES_HANDLING,
		D_STEP_COUNT,
	}d_step_enum;

	volatile _Bool dispatcher_tick;


	
	static struct {
		struct{
			d_step_enum		step;
			bool			tick;
			bool			has_expired;
			uint_fast8_t	fragmentation;
		}state;
	
		uint_fast8_t	len;
		d_periodic_task_str	queue[D_QUEUE_SIZE];
	}dp;

	/**************************************************************
					GENERAL FUNCTIONS
	**************************************************************/

	/**
	 *	Dispatcher initialization
	 */
	void D_Init(void){
		memset(&dp, 0, sizeof(dp));
	}

	/**
	 *	Dispatcher tick source
	 */
	void D_RegisterTick(void) {
		D_DISABLE_IRQ;
		dp.state.tick = true;
		D_ENABLE_IRQ;
	}
	
	/**
	 *	Dispatcher handler
	 */			
	void D_Handler(void){

		assert(dp.state.step <= D_STEP_COUNT);

		switch(dp.state.step) { default: break;

			case D_STEP_DECREMENT_DELAYS:

				if(dp.state.tick) {
					dp.state.tick = false;
					_DecrementDelays();
					if(dp.state.has_expired) { dp.state.step++; }
				}
				break;

			case D_STEP_TASK_EXECUTION:
				_ExecuteTask();
				dp.state.step++;
				break;

			case D_STEP_QUEUES_HANDLING:
				_QueueService();
				dp.state.step = 0;
				break;
		}
	}
	
	/**************************************************************
					CONTROL FUNCTIONS
	**************************************************************/
	
	/**
	 *	Add task to infinity execution queue with given period
	 */
	bool D_SetPeriodicalTask(register d_task_fptr task, register d_period period){

		assert(task);
		assert(period > 0);
		assert(dp.len < D_QUEUE_SIZE);

		if( (task) 
		and (period > 0)
		and (dp.len < D_QUEUE_SIZE)	) 
		{
			// Getting queue free address
			dp.queue[dp.len] = (d_periodic_task_str){ task, period };
			dp.len++;
			return true;
		}
		else { return false; }
	}
	
	/**
	 *	Delete task from execution queue
	 *	@param - task 
	 *	@return - BOOL, false - nothing has found
	 */
	 uint_fast8_t D_ClearTasks(d_task_fptr task){
		register uint_fast8_t i;
		register uint_fast8_t deleted_count = 0;
		 
		if(dp.len > 0) { for(i = 0; i < dp.len; i++){

			if(dp.queue[i].task == task){
				dp.queue[i].task = NULL;
				deleted_count++;
				dp.state.fragmentation++;
			}

			_QueueService();
		}}

		return deleted_count;
	}
			
	/**************************************************************
					LOCAL FUNCTIONS DEFINITION
	**************************************************************/

	/**
	 *	Dercrement task delays
	 *	@param - NONE
	 *	@return - NONE
	 */
	static inline void	_DecrementDelays(void){
		register uint_fast8_t  i;
		 
		if(dp.len > 0) {
			for(i = 0; i < dp.len; i++){
				if(dp.queue[i].delay > 0) { dp.queue[i].delay--; }
				else if(dp.queue[i].task) { dp.state.has_expired = true; }
			}	
		}
	}
	
	/**
	 *	Execute first task with expired timer
	 *	@param - NONE
	 *	@return - NONE
	 */
	static inline void _ExecuteTask(void){
		register uint_fast8_t  i;

		// Getting expired task
		for(i = 0; i < dp.len; i++){

			if(dp.queue[i].delay == 0){

				if(dp.queue[i].task){
					dp.queue[i].task();
					dp.queue[i].task = 0;
					dp.state.fragmentation++;
					return;
				}
			}
		}
		dp.state.has_expired = false;
	}
	
	/**
	 *	Queue service handling (sorting and etc.)
	 *	@param - NONE
	 *	@return - NONE
	 */
	static inline void	_QueueService(void){
		register uint_fast8_t read;
		register uint_fast8_t write;
		register bool is_shifted;

		// Sorting part only for now
		//
		if(dp.state.fragmentation > 0) {


			for(read = 0, write = 0, is_shifted = false; 
				read < dp.len; (read++, write++)){
			
				while(not dp.queue[read].task){
					is_shifted = true;
					read++;
					if(read == dp.len) break;
				}
			
				if(is_shifted) { dp.queue[write] = dp.queue[read]; }
			}

			dp.len = write;
			dp.state.fragmentation = 0;
		}
	}