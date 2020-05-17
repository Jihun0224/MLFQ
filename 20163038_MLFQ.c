#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
#pragma warning(disable: 4996)

int now_time = 0; //현재 시간
int completed_process = 0;//완료된 process
int generated_procees = 0; // 생성된 process
int context_switches = 0; //context switch 횟수
int idle_time = 0;
typedef struct process {
    int pid, execution_t, arr_t, pri, wait_t, turnaround_t, rem_t, aging;
    //프로세스 id, 실행시간, 도착시간, 우선순위, 대기시간, 전체 걸린시간, 남은 실행시간 , 해당 큐에서 wait 타임 담을 aging
    struct process* next;
    struct process* previous;
}process;
typedef struct Queue {
    process* front;
    process* rear;
    int jobs;
    int tau;
}Queue;
Queue completed; //완료된 process 담을 큐
void print_state(Queue* fifo_q, Queue* pq_q, Queue* sjf_q, Queue* rr_q) {

    printf("\n# of jobs in RR = %d\n", rr_q->jobs);
    printf("# of jobs in SJF = %d\n", sjf_q->jobs);
    printf("# of jobs in PRQ = %d\n", pq_q->jobs);
    printf("# of jobs in FIFO = %d\n", fifo_q->jobs);
    printf("#context switches = %d\n", context_switches);
    printf("#Total processes generated = %d\n", generated_procees);
    printf("#Total processes completed = %d\n\n", completed_process);
    printf("----------------------------------------------\n");
}
void initQueue(Queue* queue) {
    queue->front = queue->rear = NULL;
    queue->jobs = 0;
    queue->tau = 0;
}
int isEmpty(Queue* queue) {
    if (queue->front == NULL)
        return 1;
    else
        return 0;
}
void Enqueue(Queue* queue, process* new) {
    process* now;
    now = new;
    now->next = NULL;
    if (isEmpty(queue)) {
        queue->front = now;
        queue->rear = now;
    }
    else {
        now->previous = queue->rear;
        queue->rear->next = now;
        queue->rear = now;
    }
    queue->jobs++;
}
//pq_q에 들어갈때 process의 우선순위를 비교해서 Enqueue
void pq_Enqueue(Queue* queue, process* new) {
    process* now = (process*)malloc(sizeof(process));
    now = new;
    now->next = NULL;
    if (isEmpty(queue)) {
        queue->front = now;
        queue->rear = now;
    }
    else {
        process* current = queue->front;
        while (current != NULL) {
            if (now->pri < current->pri) {
                if (current == queue->front) {
                    now->next = current;
                    current->previous = now;
                    queue->front = now;
                    break;
                }
                //current가 q 중간일 경우
                now->previous = current->previous;
                now->next = current;
                current->previous->next = now;
                current->previous = now;
                break;
            }
            current = current->next;
        }
        if (current == NULL) {
            queue->rear->next = now;
            now->previous = queue->rear;
            queue->rear = now;
        }
    }
    queue->jobs++;
}
//sjf_q에 들어갈때 process의 남은 시간을 비교해서 Enqueue
void sjf_Enqueue(Queue* queue, process* new) {
    process* now = new;
    now->next = NULL;
    if (isEmpty(queue)) {
        queue->front = now;
        queue->rear = now;
    }
    else {
        process* current = queue->front;
        while (current != NULL) {
            if (now->rem_t < current->rem_t) {//남은시간이 더 낮으면 currnent 앞으로 enqueue
                if (current == queue->front) { //새로 들어온 process의 남은 시간이 큐의 젤 앞 process보다 낮을 경우
                    now->next = current;
                    current->previous = now;
                    queue->front = now;
                    break;
                }
                //current가 q 중간일 경우
                now->previous = current->previous;
                now->next = current;
                current->previous->next = now;
                current->previous = now;
                break;
            }
            current = current->next;
        }
        if (current == NULL) { //now가 우선순위가 제일 낮은 경우
            queue->rear->next = now;
            now->previous = queue->rear;
            queue->rear = now;
        }
    }
    queue->jobs++;
}
process* Dequeue(Queue* queue) {
    process* de_process;
    if (isEmpty(queue)) {
        return NULL;
    }
    de_process = queue->front;
    if (queue->front == queue->rear) {
        queue->front = NULL;
        queue->rear = NULL;
        queue->jobs--;
        return de_process;
    }
    queue->front = de_process->next;
    queue->jobs--;
    return de_process;
}
//입력받은 aging에 도달한 process를 받아서 pid로 process위치를 확인하고 Dequeue
process* feedback_Dequeue(Queue* queue, process* move) {
    process* temp;
    process* rear;
    temp = queue->front;
    rear = queue->rear;
    if (temp->pid == move->pid && rear->pid == move->pid) { //큐에 유일하게 있는 process
        queue->front = NULL;
        queue->rear = NULL;
        queue->jobs--;
        return temp;
    }
    else if (temp->pid == move->pid) { //move가 q의 front일때 
        queue->front = temp->next;
        queue->front->previous = NULL;
        queue->jobs--;
        return temp;
    }
    else if (rear->pid == move->pid) {//move가 q의 rear일때 
        queue->rear = rear->previous;
        queue->rear->next = NULL;
        queue->jobs--;
        return rear;
    }
    else {
        while (temp != NULL) {
            if (temp->pid == move->pid) {
                temp->previous->next = temp->next;
                temp->next->previous = temp->previous;
                queue->jobs--;
                return temp;
            }
            temp = temp->next;
        }
    }
    printf("error\n");
    return NULL;
}
void Initprocess(process* process) {

    process->aging = 0;
    process->arr_t = now_time;
    process->execution_t = rand() % 20 + 1;
    process->next = NULL;
    process->pid = generated_procees;
    process->previous = NULL;
    process->pri = rand() % 20 + 1;
    process->rem_t = process->execution_t;
    process->turnaround_t = 0;
    process->wait_t = 0;
}
//기다리는 process aging_time을 증가시켜줌
void up_aging(Queue* fifo_q, Queue* pq_q, Queue* sjf_q) {

    process* fifo_current = fifo_q->front;
    while (fifo_current != NULL) {
        fifo_current->aging++;
        fifo_current = fifo_current->next;
    }
    process* pq_current = pq_q->front;
    while (pq_current != NULL) {
        pq_current->aging++;
        pq_current = pq_current->next;
    }
    process* sjf_current = sjf_q->front;
    while (sjf_current != NULL) {
        sjf_current->aging++;
        sjf_current = sjf_current->next;
    }
}
//process의 aging_time이 입력받은 aging과 같아지면 process 이동
void feedback_process(Queue* fifo_q, Queue* pq_q, Queue* sjf_q, Queue* rr_q, int fifo_aging, int pq_aging, int sjf_aging) {

    process* fifo_move = fifo_q->front;
    while (fifo_move != NULL) {
        if (fifo_aging == fifo_move->aging) {
            process* up = fifo_move;
            fifo_move = fifo_move->next;
            up->aging = 0; //pq로 올라가므로 aging 값 0으로
            pq_Enqueue(pq_q, feedback_Dequeue(fifo_q, up));// aging값에 도달한 process를 fifo_q에서 찾고 빼내서 pq_q에 enqueue
            print_state(fifo_q, pq_q, sjf_q, rr_q);
        }
        else {
            fifo_move = fifo_move->next;
        }
    }
    process* pq_move = pq_q->front;
    while (pq_move != NULL) {
        if (pq_aging == pq_move->aging) {
            process* up = pq_move;
            pq_move = pq_move->next;
            up->aging = 0;
            sjf_Enqueue(sjf_q, feedback_Dequeue(pq_q, up));
            print_state(fifo_q, pq_q, sjf_q, rr_q);
        }
        else {
            pq_move = pq_move->next;
        }
    }
    process* sjf_move = sjf_q->front;
    while (sjf_move != NULL) {
        if (sjf_aging == sjf_move->aging) {
            process* up = sjf_move;
            sjf_move = sjf_move->next;
            up->aging = 0;
            Enqueue(rr_q, feedback_Dequeue(sjf_q, up));
            print_state(fifo_q, pq_q, sjf_q, rr_q);
        }
        else {
            sjf_move = sjf_move->next;
        }
    }
}
void process_generate(Queue* fifo_q, Queue* pq_q, Queue* sjf_q, Queue* rr_q, int fifo_iet, int pq_iet, int sjf_iet, int rr_iet, int context_switching_time, int fifo_ace, int pq_ace, int sjf_ace, int rr_ace) {

    double co_eff = 0;
    int choose_q = rand() % 4;

    process* generated_process = (process*)malloc(sizeof(process));
    Initprocess(generated_process);
    generated_procees++;
    int actual_burst = generated_process->execution_t; //난수로 설정된 실행시간을 저장
    //처음 들어올때 
    if (now_time == 0) {
        context_switches++;
        now_time = now_time + context_switching_time;
    }
    if (choose_q == 0) {
        co_eff = (double)rr_ace / 100;           //0<= co_eff <= 1 이므로 /100
        if (rr_q->tau == 0) {    //처음 큐에 들어왔을때
            generated_process->execution_t = rr_iet;  //입력받음 초기 예측값을 실행 시간으로
            generated_process->rem_t = rr_iet;        //갓 생성 되었으므로 남은시간에도 초기 예측값으로
            rr_q->tau = (int)(co_eff * actual_burst + (1 - co_eff) * rr_iet) + 0.9; //다음 생성될 프로세스의 실행 시간을 위해 다음 tau값 구하는 공식에 의해 구함. +0.9는 올림하기 위해
        }
        else {
            generated_process->execution_t = rr_q->tau;  //전에 구한 tau값을 넣어줌
            generated_process->rem_t = rr_q->tau;
            rr_q->tau = (int)(co_eff * actual_burst + (1 - co_eff) * rr_q->tau) + 0.9; // 다음 프로세스 실행시간을 결정하기위해 tau값 구함
        }
        Enqueue(rr_q, generated_process);
        printf("Process appended to RR queue.\n");
        printf("Process execution time = %d\n", generated_process->execution_t);
        print_state(fifo_q, pq_q, sjf_q, rr_q);
    }
    else if (choose_q == 1) {
        co_eff = (double)sjf_ace / 100;
        if (sjf_q->tau == 0) {
            generated_process->execution_t = sjf_iet;
            generated_process->rem_t = sjf_iet;
            sjf_q->tau = (int)(co_eff * actual_burst + (1 - co_eff) * sjf_iet) + 0.9;
        }
        else {
            generated_process->execution_t = sjf_q->tau;  //전에 구한 tau값을 넣어줌
            generated_process->rem_t = sjf_q->tau;
            sjf_q->tau = (int)(co_eff * actual_burst + (1 - co_eff) * sjf_q->tau) + 0.9;
        }
        process* temp = generated_process;
        sjf_Enqueue(sjf_q, temp);
        printf("Process appended to SJF queue.\n");
        printf("Process execution time = %d\n", generated_process->execution_t);
        print_state(fifo_q, pq_q, sjf_q, rr_q);
    }
    else if (choose_q == 2) {
        co_eff = (double)pq_ace / 100;
        if (pq_q->tau == 0) {
            generated_process->execution_t = pq_iet;
            generated_process->rem_t = pq_iet;
            pq_q->tau = (int)(co_eff * actual_burst + (1 - co_eff) * pq_q->tau) + 0.9;
        }
        else {
            generated_process->execution_t = pq_q->tau;  //전에 구한 tau값을 넣어줌
            generated_process->rem_t = pq_q->tau;
            pq_q->tau = (int)(co_eff * actual_burst + (1 - co_eff) * pq_q->tau) + 0.9; // 다음 프로세스 실행시간을 결정하기위해 tau값 구함
        }
        pq_Enqueue(pq_q, generated_process);
        printf("Process appended to PRQ queue.\n");
        printf("Process execution time = %d\n", generated_process->execution_t);
        print_state(fifo_q, pq_q, sjf_q, rr_q);
    }
    else {
        co_eff = (double)fifo_ace / 100;
        if (fifo_q->tau == 0) {
            generated_process->execution_t = fifo_iet;
            generated_process->rem_t = fifo_iet;
            fifo_q->tau = (int)(co_eff * actual_burst + (1 - co_eff) * fifo_q->tau) + 0.9;
        }
        else {
            generated_process->execution_t = fifo_q->tau;  //전에 구한 tau값을 넣어줌
            generated_process->rem_t = fifo_q->tau;
            fifo_q->tau = (int)(co_eff * actual_burst + (1 - co_eff) * fifo_q->tau) + 0.9; // 다음 프로세스 실행시간을 결정하기위해 tau값 구함
        }
        Enqueue(fifo_q, generated_process);
        printf("Process appended to FIFO queue.\n");
        printf("Process execution time = %d\n", generated_process->execution_t);
        print_state(fifo_q, pq_q, sjf_q, rr_q);
    }

}
void fifo(Queue* fifo_q, Queue* pq_q, Queue* sjf_q, Queue* rr_q, int fifo_iet, int pq_iet, int sjf_iet, int rr_iet, int context_switching_time, int fifo_aging, int pq_aging, int sjf_aging, int fifo_ace, int pq_ace, int sjf_ace, int rr_ace, int cycles) {
    int i;
    process* temp = Dequeue(fifo_q);
    for (i = 0;i < temp->rem_t;i++) {
        fifo_q->jobs++; //현재 진행 중인 process는 앞에서 Dequeue할때 jobs--을 해줘서 빠져있으므로 그 process가 진행중일때 다른 프로세스가 생성될때 실행중인 현재 process까지 jobs에 
                        //포함하기위해 ++해주고 적절한 위치에서 --해줘서 문제없도록 진행해줍니다. 다른 Q에서도 이처럼 해줍니다.
        if (cycles <= now_time) {
            fifo_q->jobs--;
            Enqueue(fifo_q, temp);
            print_state(fifo_q, pq_q, sjf_q, rr_q);
            return;
        }
        int random_num = rand();
        //랜덤으로 프로세스 생성
        if (random_num % 4 == 1) {
            process_generate(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_ace, pq_ace, sjf_ace, rr_ace);
        }
        //상위 큐에 process 있으면 context switching 발생
        if (pq_q->front != NULL || sjf_q->front != NULL || rr_q->front != NULL) {
            fifo_q->jobs--;
            Enqueue(fifo_q, temp);
            context_switches++;
            now_time = now_time + context_switching_time;
            return;
        }
        now_time++;
        temp->rem_t--;
        up_aging(fifo_q, pq_q, sjf_q);
        feedback_process(fifo_q, pq_q, sjf_q, rr_q, fifo_aging, pq_aging, sjf_aging);
        fifo_q->jobs--;
    }
    completed_process++;
    temp->wait_t = now_time - temp->arr_t;
    temp->turnaround_t = temp->wait_t + temp->execution_t;
    Enqueue(&completed, temp);
    print_state(fifo_q, pq_q, sjf_q, rr_q);
}
void pq(Queue* fifo_q, Queue* pq_q, Queue* sjf_q, Queue* rr_q, int fifo_iet, int pq_iet, int sjf_iet, int rr_iet, int context_switching_time, int fifo_aging, int pq_aging, int sjf_aging, int pq_preemptive, int fifo_ace, int pq_ace, int sjf_ace, int rr_ace, int cycles) {

    int i;
    process* temp = Dequeue(pq_q);

    for (i = 0;i < temp->rem_t;i++) {
        pq_q->jobs++;
        if (cycles <= now_time) {
            pq_q->jobs--;
            pq_Enqueue(pq_q, temp);
            print_state(fifo_q, pq_q, sjf_q, rr_q);
            return;
        }
        int random_num = rand();
        //랜덤으로 프로세스 생성
        if (random_num % 4 == 1) {
            process_generate(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_ace, pq_ace, sjf_ace, rr_ace);
        }
        //상위 큐에 process 있으면 context switching 발생
        if (sjf_q->front != NULL || rr_q->front != NULL) {
            pq_q->jobs--;
            pq_Enqueue(pq_q, temp);
            context_switches++;
            now_time = now_time + context_switching_time;
            return;
        }
        //preemptive
        if (pq_preemptive == 1) {
            if (pq_q->front != NULL) {
                int check_pri = pq_q->front->pri;
                if (check_pri < temp->pri) { //pq_q에서 front에 있는 Process가 우선순위가 더 높을때 context switching 발생
                    pq_q->jobs--;
                    pq_Enqueue(pq_q, temp);
                    context_switches++;
                    now_time = now_time + context_switching_time;
                    return;
                }
            }
        }

        now_time++;
        temp->rem_t--;
        up_aging(fifo_q, pq_q, sjf_q);
        feedback_process(fifo_q, pq_q, sjf_q, rr_q, fifo_aging, pq_aging, sjf_aging);
        pq_q->jobs--;
    }

    completed_process++;
    temp->wait_t = now_time - temp->arr_t;
    temp->turnaround_t = temp->wait_t + temp->execution_t;
    Enqueue(&completed, temp);
    print_state(fifo_q, pq_q, sjf_q, rr_q);

}
void sjf(Queue* fifo_q, Queue* pq_q, Queue* sjf_q, Queue* rr_q, int fifo_iet, int pq_iet, int sjf_iet, int rr_iet, int context_switching_time, int fifo_aging, int pq_aging, int sjf_aging, int sjf_preemptive, int fifo_ace, int pq_ace, int sjf_ace, int rr_ace, int cycles) {
    int i;
    process* temp = Dequeue(sjf_q);
    for (i = 0;i < temp->rem_t;i++) {
        sjf_q->jobs++;
        if (cycles <= now_time) {
            sjf_q->jobs--;
            sjf_Enqueue(sjf_q, temp);
            print_state(fifo_q, pq_q, sjf_q, rr_q);
            return;
        }
        int random_num = rand();
        //랜덤으로 프로세스 생성
        if (random_num % 4 == 1) {
            process_generate(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_ace, pq_ace, sjf_ace, rr_ace);
        }
        //rr_q에 process가 있으면 context switching 발생
        if (rr_q->front != NULL) {
            sjf_q->jobs--;
            sjf_Enqueue(sjf_q, temp);
            context_switches++;
            now_time = now_time + context_switching_time;
            return;
        }
        //preemptive 일때
        if (sjf_preemptive == 1) {
            if (sjf_q->front != NULL) {
                int check_rem_t = sjf_q->front->rem_t; //큐 젤 앞의 process 의 remain time 과 비교
                if (check_rem_t < temp->rem_t) {
                    sjf_q->jobs--;
                    sjf_Enqueue(sjf_q, temp);
                    context_switches++;             //context switching 이 발생했고 
                    now_time = now_time + context_switching_time;
                    return;         //스케줄러로 이동
                }
            }
        }

        now_time++;
        temp->rem_t--;
        up_aging(fifo_q, pq_q, sjf_q);
        feedback_process(fifo_q, pq_q, sjf_q, rr_q, fifo_aging, pq_aging, sjf_aging);
        sjf_q->jobs--;
    }

    completed_process++;
    temp->wait_t = now_time - temp->arr_t;
    temp->turnaround_t = temp->wait_t + temp->execution_t;
    Enqueue(&completed, temp);
    print_state(fifo_q, pq_q, sjf_q, rr_q);
}
void rr(Queue* fifo_q, Queue* pq_q, Queue* sjf_q, Queue* rr_q, int fifo_iet, int pq_iet, int sjf_iet, int rr_iet, int rr_time_slot, int context_switching_time, int fifo_aging, int pq_aging, int sjf_aging, int fifo_ace, int pq_ace, int sjf_ace, int rr_ace, int cycles) {
    int i;
    process* temp = Dequeue(rr_q);
    for (i = 0;i < rr_time_slot; i++) {
        rr_q->jobs++;
        if (cycles <= now_time) {
            rr_q->jobs--;
            Enqueue(rr_q, temp);
            print_state(fifo_q, pq_q, sjf_q, rr_q);
            return;
        }
        int random_num = rand();
        //랜덤으로 프로세스 생성
        if (random_num % 4 == 1) {
            process_generate(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_ace, pq_ace, sjf_ace, rr_ace);
        }

        now_time++;
        temp->rem_t--;
        up_aging(fifo_q, pq_q, sjf_q);
        feedback_process(fifo_q, pq_q, sjf_q, rr_q, fifo_aging, pq_aging, sjf_aging);
        rr_q->jobs--;
        //퀀텀만큼 돌기전에 프로세스가 끝났을 경우
        if (temp->rem_t == 0) {
            completed_process++;
            temp->wait_t = now_time - temp->arr_t - temp->execution_t;
            temp->turnaround_t = temp->wait_t + temp->execution_t;
            Enqueue(&completed, temp);
            print_state(fifo_q, pq_q, sjf_q, rr_q);
            break;
        }

    }

    //퀀텀만큼 실행했는데 안 끝났을 경우 sjf로 이동
    if (temp->rem_t != 0) {
        sjf_Enqueue(sjf_q, temp);
        context_switches++;
        now_time = now_time + context_switching_time;
        print_state(fifo_q, pq_q, sjf_q, rr_q);
    }

}
void Scheduler(Queue* fifo_q, Queue* pq_q, Queue* sjf_q, Queue* rr_q, int cycles, int fifo_iet, int pq_iet, int sjf_iet, int rr_iet, int fifo_aging, int pq_aging, int sjf_aging, int rr_aging, int rr_time_slot, int context_switching_time, int pq_preemptive, int sjf_preemptive, int fifo_ace, int pq_ace, int sjf_ace, int rr_ace) {

    process_generate(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_ace, pq_ace, sjf_ace, rr_ace);

    while (cycles > now_time) {
        int random_num = rand();
        //랜덤으로 프로세스 생성
        if (random_num % 4 == 1) {
            process_generate(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_ace, pq_ace, sjf_ace, rr_ace);
        }

        if (rr_q->front != NULL) {
            rr(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, rr_time_slot, context_switching_time, fifo_aging, pq_aging, sjf_aging, fifo_ace, pq_ace, sjf_ace, rr_ace, cycles);
        }
        else if (rr_q->front == NULL && sjf_q->front != NULL) {
            sjf(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_aging, pq_aging, sjf_aging, sjf_preemptive, fifo_ace, pq_ace, sjf_ace, rr_ace, cycles);
        }
        else if (rr_q->front == NULL && sjf_q->front == NULL && pq_q->front != NULL) {
            pq(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_aging, pq_aging, sjf_aging, pq_preemptive, fifo_ace, pq_ace, sjf_ace, rr_ace, cycles);
        }
        else if (rr_q->front == NULL && sjf_q->front == NULL && pq_q->front == NULL && fifo_q->front != NULL) {
            fifo(fifo_q, pq_q, sjf_q, rr_q, fifo_iet, pq_iet, sjf_iet, rr_iet, context_switching_time, fifo_aging, pq_aging, sjf_aging, fifo_ace, pq_ace, sjf_ace, rr_ace, cycles);
        }
        else {
            idle_time++;
            now_time++;
        }
    }

}
void Summary(int cycles) {
    double AWT = 0;
    double ATT = 0;
    double ALL_TT;
    int  MAX_WT, MAX_TT;
    double Throughput;
    double d_cycles = (double)cycles;
    double d_idle_t = (double)idle_time;
    double d_completed_process = (double)completed_process;;
    double utilization = d_cycles / (d_cycles + d_idle_t) * 100;
    process* temp = completed.front;
    MAX_WT = temp->wait_t;
    MAX_TT = temp->turnaround_t;

    while (temp != NULL) {
        AWT = AWT + temp->wait_t;
        ATT = ATT + temp->turnaround_t;
        if (MAX_WT < temp->wait_t) {
            MAX_WT = temp->wait_t;
        }
        if (MAX_TT < temp->turnaround_t) {
            MAX_TT = temp->turnaround_t;
        }
        temp = temp->next;
    }
    ALL_TT = ATT;
    Throughput = (d_completed_process / d_cycles) / ALL_TT;
    AWT = AWT / completed.jobs;
    ATT = ATT / completed.jobs;
    printf("----------------------------------------------\n\n");
    printf("\tSUMMARY\t\n");
    printf("----------------------------------------------\n");
    printf("AVERAGE WAITING TIME\t= %.4f time units\n", AWT);
    printf("AVERAGE TURNAROUND TIME = %.4f time units\n", ATT);
    printf("CPU UTILIZATION\t= %.0f %%\n", utilization);
    printf("MAXIMUM TURNAROUND TIME = %d\n", MAX_TT);
    printf("MAXIMUM WAIT TIME\t= %d\n", MAX_WT);
    printf("CPU THROUGHPUT\t= %.0f %%\n", Throughput);
    printf("----------------------------------------------\n");
}
int main(void) {
    //iet = initial estimated time , ace =  alpha co-eff 
    int cycles, fifo_iet, pq_iet, sjf_iet, rr_iet, fifo_aging, pq_aging, sjf_aging, rr_aging, rr_time_slot, context_switching_time, pq_preemptive, sjf_preemptive, fifo_ace, pq_ace, sjf_ace, rr_ace;
    Queue fifo_q, pq_q, sjf_q, rr_q;
    initQueue(&fifo_q);
    initQueue(&pq_q);
    initQueue(&sjf_q);
    initQueue(&rr_q);
    initQueue(&completed);
    printf("Enter the time Enter the time cycles     : ");
    scanf("%d", &cycles);
    printf("Enter the value of time slot for RR      : ");
    scanf("%d", &rr_time_slot);
    printf("Enter the context switching time          : ");
    scanf("%d", &context_switching_time);
    printf("SJF with pre-emption (1-yes/0-no)         : ");
    scanf("%d", &sjf_preemptive);
    printf("PQ with pre-emption (1-yes/0-no)          : ");
    scanf("%d", &pq_preemptive);
    printf("Enter the alpha co-eff for RR             : ");
    scanf("%d", &rr_ace);
    printf("Enter the alpha co-eff for SJF           : ");
    scanf("%d", &sjf_ace);
    printf("Enter the alpha co-eff for PQ            : ");
    scanf("%d", &pq_ace);
    printf("Enter the alpha co-eff for FIFO          : ");
    scanf("%d", &fifo_ace);
    printf("Enter the aging time for RR              : ");
    scanf("%d", &rr_aging);
    printf("Enter the aging time for SJF             : ");
    scanf("%d", &sjf_aging);
    printf("Enter the aging time for PQ              : ");
    scanf("%d", &pq_aging);
    printf("Enter the aging time for FIFO            : ");
    scanf("%d", &fifo_aging);
    printf("Enter the initial estimated time for RR : ");
    scanf("%d", &rr_iet);
    printf("Enter the initial estimated time for SJF : ");
    scanf("%d", &sjf_iet);
    printf("Enter the initial estimated time for PQ : ");
    scanf("%d", &pq_iet);
    printf("Enter the initial estimated time for FIFO : ");
    scanf("%d", &fifo_iet);


    Scheduler(&fifo_q, &pq_q, &sjf_q, &rr_q, cycles, fifo_iet, pq_iet, sjf_iet, rr_iet, fifo_aging, pq_aging, sjf_aging, rr_aging, rr_time_slot, context_switching_time, pq_preemptive, sjf_preemptive, fifo_ace, pq_ace, sjf_ace, rr_ace);
    Summary(cycles);

    return 0;

}