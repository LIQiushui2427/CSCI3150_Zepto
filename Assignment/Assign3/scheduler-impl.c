#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
// YI Jian 1155157207 Bonus assignment 3
// Operating System: Ubuntu 22.04 LTS
// Discussed with Classmate (ZHOU Yixiang) who has taken this course and TAs about the implementation of queue.
void outprint(int time_x, int time_y, int pid, int arrival_time, int remaining_time);

int get_new_job(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int highest_queue, int next_process_id, int current_time){

    printf("get New Job.\n");

    int batch = next_process_id;

    while(batch < proc_num && current_time >= proc[batch].arrival_time)batch++;
    printf("batch: %d\n", batch);
    while(next_process_id < batch){
        int batch_time = proc[next_process_id].arrival_time;
        int cur_batch_limit = next_process_id;

        while(proc[cur_batch_limit].arrival_time == batch_time)cur_batch_limit++;
        //for cases
        if(cur_batch_limit - next_process_id > 1){
            // remember the smallest pid's
        }
        
        int min_pid = 0;
        for (int i =  next_process_id; i <= cur_batch_limit - 1; i++){
            Process *p = (Process*)malloc(sizeof(Process));
            p->turnaround_time = 0;
            p->service_time = 0;
            p->waiting_time = 0;
            p->completion_time = 0;
            min_pid = proc[i].process_id;
            p->process_id = proc[i].process_id;
            p->arrival_time = proc[i].arrival_time;
            p->execution_time = proc[i].execution_time;
            printf("EnQueue: %d\n", p->process_id);
            ProcessQueue[highest_queue] = EnQueue(ProcessQueue[highest_queue], *p);
        }
        next_process_id = cur_batch_limit;
    }
    return batch == proc_num ? -1 : batch;
}

void my_free(LinkedQueue* q){
    if (q->next != NULL){
        my_free(q->next);
    }
    free(q);
}

void priority_boost(LinkedQueue** ProcessQueue, int highest_queue){
    printf("S period reached.\n");
    LinkedQueue* TempQueue = (LinkedQueue*)malloc(sizeof(LinkedQueue));
    TempQueue->next = NULL;
    int ready_queue = highest_queue;
    while(ready_queue >= 0){
        while(!IsEmptyQueue(ProcessQueue[ready_queue]))TempQueue = EnQueue(TempQueue, DeQueue(ProcessQueue[ready_queue]));
        ready_queue--;
    }
    //Brute Force Sorting by Process ID
    int flag = 1;
    while(flag > 0){
        flag = 0;
        Process p;
        Process *pp;
        p.process_id = -1;
        LinkedQueue *pt = TempQueue;
        while(pt->next){
            pt = pt->next;
            if (pt->proc.process_id > p.process_id){
                p = pt->proc;
                p.service_time = 0;
                pp = &pt->proc;
                flag = 1;//flag = 1 means there is a swap
            }
        }
        if (!flag) break;// No more swaps
        ProcessQueue[highest_queue] = AddTail(ProcessQueue[highest_queue], p);// EnQueue the highest priority process
        pp->process_id = -1;// Set the process id to -1 to indicate that it has been scheduled
        //printf("After S %d\n", p.process_id);// Debug
    }
    //ProcessQueue[highest_queue]  = sortQueue(TempQueue);
    my_free(TempQueue);
}

void scheduler(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int queue_num, int period){
    printf("Process number: %d\n", proc_num);
    for (int i = 0;i < proc_num; i++)printf("%d %d %d\n", proc[i].process_id, proc[i].arrival_time, proc[i].execution_time);

    printf("\nQueue number: %d\n", queue_num);
    printf("Period: %d\n", period);
    for (int i = 0;i < queue_num; i++)printf("%d %d %d\n", i, ProcessQueue[i]->time_slice, ProcessQueue[i]->allotment_time);
    
    int current_time = 0;
    int next_process_id = 0;
    int ready_queue = 0;
    int highest_queue = queue_num - 1;
    // int time_slice = ProcessQueue[ready_queue]->time_slice;
    while(1){
        printf("\n");
        if (next_process_id >= 0)// If there are still processes to be scheduled
            next_process_id = get_new_job(proc, ProcessQueue, proc_num, highest_queue, next_process_id, current_time);// Get new processes

        if (current_time && current_time % period == 0){// If it's time to boost
            //printf("Priority Boost.\n");
            priority_boost(ProcessQueue, highest_queue);
        }

        // Find Process to Run
        ready_queue = highest_queue;
        while(ready_queue >= 0 && IsEmptyQueue(ProcessQueue[ready_queue]))ready_queue--;// Find the highest queue that has processes

        if (ready_queue < 0 && next_process_id < 0)break;

        if (ready_queue >= 0){// If there are processes to run
            printf("Ready Queue:");
            QueuePrint(ProcessQueue[ready_queue]);
            Process current_proc = DeQueue(ProcessQueue[ready_queue]);
            int current_pid = current_proc.process_id;
            int real_slice = min(min(current_proc.execution_time, ProcessQueue[ready_queue]->time_slice), (period - (current_time % period)));
            int next_time = current_time + real_slice;
            current_proc.execution_time -= real_slice;
            current_proc.service_time += real_slice;        
            printf("Process %d found in ready queue %d.\n",current_pid ,ready_queue);
            printf("Current Time: %d\n, Next Time: %d\n", current_time, next_time);
            outprint(current_time, next_time, current_pid, current_proc.arrival_time, current_proc.execution_time);
            current_time = next_time;

            if (current_proc.execution_time == 0){
                printf("Process %d finished.\n", current_pid);
            } else if (current_proc.service_time >= ProcessQueue[ready_queue]->allotment_time){
                printf("Process %d downgraded, serviced time %d excceds allotment %d.\n", current_pid, current_proc.service_time, ProcessQueue[ready_queue]->allotment_time);
                ready_queue = ready_queue == 0 ? 0 : ready_queue - 1;
                current_proc.service_time = 0;
                ProcessQueue[ready_queue] = EnQueue(ProcessQueue[ready_queue], current_proc);
            } else{
                printf("Process %d remains in ready queue with serviced time %d.\n", current_pid, current_proc.service_time);
                ProcessQueue[ready_queue] = EnQueue(ProcessQueue[ready_queue], current_proc);
            }
        } else{
            current_time = proc[next_process_id].arrival_time;
        }// If there are no processes to run, skip to the next process
    }// End of while loop
}