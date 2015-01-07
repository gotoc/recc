/*
	Copyright 2014 Robert Elder Software Inc.  All rights reserved.

	This software is not currently available under any license, and unauthorized use
	or copying is not permitted.

	This software will likely be available under a common open source license in the
	near future.  Licensing is currently pending feedback from a lawyer.  If you have
	an opinion on this subject you can send it to recc [at] robertelder.org.

	This program comes with ABSOLUTELY NO WARRANTY.  In no event shall Robert Elder
	Software Inc. be liable for incidental or consequential damages in connection with
	use of this software.
*/
#include "user_proc.h"

void user_proc_1(void){
	while(1){
		printf("You're currently running a very simple microkernel that was built\n");
		printf("for the purposes of demonstrating the 'One Page CPU' design, and\n");
		printf("cross compiler collection.  This microkernel implements inter-process\n");
		printf("communication, premptive context switching, interrupt based I/O, along\n");
		printf("with a very simple timer that counts simulated clock ticks.\n");
		printf("\nSome single-character commands include:\n\n");
		printf("t -  Prints the number of simulated clock ticks since kernel start.\n");
		printf("s -  Prints the stack pointer values of each task.\n");
		printf("p -  Prints the priority of each task.\n");
		task_exit();
	}
}

void user_proc_2(void){
	struct kernel_message m;
        m.message_type = OUTPUT_CHARACTER;
        m.data = 'M';
	while(1){
        	release_processor();
	}
}

void clock_tick_notifier(void){
	struct kernel_message clock_server_message;
	struct kernel_message * clock_server_message_ptr = &clock_server_message;
	struct kernel_message clock_server_reply;
	struct kernel_message * clock_server_reply_ptr = &clock_server_reply;
	clock_server_message_ptr->message_type = CLOCK_TICK_NOTIFY;
	clock_server_message_ptr->data = 1;
	while(1){
		block_on_event(CLOCK_TICK_EVENT);
		send_message(clock_server_message_ptr, 4, clock_server_reply_ptr);
		switch(clock_server_reply_ptr->message_type){
			case MESSAGE_ACKNOWLEDGED:{
				break;
			}default:{
				assert(0, "Unknown message type.\n");
			}
		}
	}
}

unsigned int num_ticks = 0;

void clock_server(void){
	struct kernel_message message_to_reply;
	struct kernel_message received_message;
	message_to_reply.message_type = MESSAGE_ACKNOWLEDGED;
	while(1){
		receive_message(&received_message);
		switch(received_message.message_type){
			case CLOCK_TICK_NOTIFY:{
				num_ticks++;
				break;
			}default:{
				assert(0, "Unknown message type.\n");
			}
		}
		reply_message(&message_to_reply, received_message.source_id);
	}
}

void uart1_out_ready_notifier(void){
	struct kernel_message output_server_message;
	struct kernel_message * output_server_message_ptr = &output_server_message;
	struct kernel_message output_server_reply;
	struct kernel_message * output_server_reply_ptr = &output_server_reply;
	output_server_message_ptr->message_type = UART1_OUT_READY_NOTIFY;
	output_server_message_ptr->data = 1;
	while(1){
		block_on_event(UART1_OUT_READY);
		send_message(output_server_message_ptr, 6, output_server_reply_ptr);
		switch(output_server_reply_ptr->message_type){
			case MESSAGE_ACKNOWLEDGED:{
				break;
			}default:{
				assert(0, "Unknown message type.\n");
			}
		}
	}
}

void uart1_out_server(void){
	struct message_queue output_requests;
	struct message_queue * output_requests_ptr = &output_requests;
	struct kernel_message message_to_reply;
	struct kernel_message * message_to_reply_ptr = &message_to_reply;
	struct kernel_message received_message;
	struct kernel_message * received_message_ptr = &received_message;
	struct kernel_message poped_message;
	struct kernel_message * poped_message_ptr = &poped_message;
	unsigned int can_output = 1; /* Initially, it is ok to send a character when the CPU starts, because we haven't got an interrupt yet */
	message_to_reply.message_type = MESSAGE_ACKNOWLEDGED;
	message_queue_init(output_requests_ptr, MAX_NUM_PROCESSES);
	while(1){
		receive_message(received_message_ptr);
		switch(received_message_ptr->message_type){
			case UART1_OUT_READY_NOTIFY:{
				if(message_queue_current_count(output_requests_ptr)){
					poped_message = message_queue_pop_start(output_requests_ptr);
					putchar_nobusy(poped_message_ptr->data);
					can_output = 0;
					reply_message(message_to_reply_ptr, poped_message_ptr->source_id);
				}else{
					can_output = 1;
				}
				reply_message(message_to_reply_ptr, received_message_ptr->source_id);
				break;
			}case OUTPUT_CHARACTER:{
				if(can_output){
					putchar_nobusy(received_message_ptr->data);
					reply_message(message_to_reply_ptr, received_message_ptr->source_id);
					can_output = 0;
				}else{
					message_queue_push_end(output_requests_ptr, *received_message_ptr);
				}
				break;
			}default:{
				assert(0, "Unknown message type.\n");
			}
		}
	}
}

void uart1_in_ready_notifier(void){
	struct kernel_message input_server_message;
	struct kernel_message * input_server_message_ptr = &input_server_message;
	struct kernel_message input_server_reply;
	struct kernel_message * input_server_reply_ptr = &input_server_reply;
	input_server_message_ptr->message_type = UART1_IN_READY_NOTIFY;
	input_server_message_ptr->data = 1;
	while(1){
		block_on_event(UART1_IN_READY);
		send_message(input_server_message_ptr, 8, input_server_reply_ptr);
		switch(input_server_reply_ptr->message_type){
			case MESSAGE_ACKNOWLEDGED:{
				break;
			}default:{
				assert(0, "Unknown message type.\n");
			}
		}
	}
}

void uart1_in_server(void){
	struct kernel_message message_to_reply;
	struct kernel_message * message_to_reply_ptr = &message_to_reply;
	struct kernel_message received_message;
	struct kernel_message * received_message_ptr = &received_message;
	struct kernel_message output_server_message;
	struct kernel_message * output_server_message_ptr = &output_server_message;
	struct kernel_message output_server_reply;
	struct kernel_message * output_server_reply_ptr = &output_server_reply;
	message_to_reply_ptr->message_type = MESSAGE_ACKNOWLEDGED;
	output_server_message_ptr->message_type = OUTPUT_CHARACTER;
	while(1){
		receive_message(received_message_ptr);
		switch(received_message_ptr->message_type){
			case UART1_IN_READY_NOTIFY:{
				output_server_message_ptr->data = getchar_nobusy();
				/*  Send the character to output */
				send_message(output_server_message_ptr, 6, output_server_reply_ptr);
				switch(output_server_reply_ptr->message_type){
					case MESSAGE_ACKNOWLEDGED:{
						break;
					}default:{
						assert(0, "Unknown message type.\n");
					}
				}
				/*  Let the command server know what is being typed */
				send_message(output_server_message_ptr, 9, output_server_reply_ptr);
				switch(output_server_reply_ptr->message_type){
					case MESSAGE_ACKNOWLEDGED:{
						break;
					}default:{
						assert(0, "Unknown message type.\n");
					}
				}
				reply_message(message_to_reply_ptr, received_message_ptr->source_id);
				break;
			}default:{
				assert(0, "Unknown message type.\n");
			}
		}
	}
}


void command_server(void){
	struct kernel_message received_message;
	struct kernel_message * received_message_ptr = &received_message;
	struct kernel_message input_server_reply;
	struct kernel_message * input_server_reply_ptr = &input_server_reply;
	input_server_reply_ptr->message_type = MESSAGE_ACKNOWLEDGED;
	while(1){
		receive_message(received_message_ptr);
		switch(received_message_ptr->message_type){
			case OUTPUT_CHARACTER:{
				switch(received_message_ptr->data){
					case 116:{/* letter 't' */
						printf("\n%d\n", num_ticks);
						break;
					}case 115:{/* letter 's' */
						unsigned int i;
						printf("\n");
						for(i = 0; i < 9; i++){
							printf("Task %d SP: 0x%X\n", i, pcb_ptrs[i]->stack_pointer);
						}
						break;
					}case 112:{/* letter 'p' */
						unsigned int i;
						printf("\n");
						for(i = 0; i < 9; i++){
							printf("Task %d Priority: %d\n", i, pcb_ptrs[i]->priority);
						}
						break;
					}default:{
						printf("\n");
						printf("Unknown command.");
					}
				}
				break;
			}default:{
				assert(0, "Unknown message type.\n");
			}
		}
		reply_message(input_server_reply_ptr, received_message_ptr->source_id);
	}
}