/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #2
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of WORDs)       */

#define          TASK_START_ID       0 
#define          TASK_1_ID           3
#define          TASK_2_ID           4
#define          TASK_3_ID           5


#define          TASK_START_PRIO    0




/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/
OS_STK        TaskStartStk[TASK_STK_SIZE];
OS_STK        Task1Stk[TASK_STK_SIZE];                /* Task #1    task stack                         */
OS_STK        Task2Stk[TASK_STK_SIZE];                /* Task #2    task stack                         */
OS_STK        Task3Stk[TASK_STK_SIZE];                /* Task #3    task stack     */       


#define          TASK_NUM            3           
INT8U         TASK_INFO[TASK_NUM][2] = {{6,30},{6,30},{9,30}};
INT8U         TASK_PRIO[TASK_NUM] = {3, 4, 5};
INT32U        TASK_CNT[TASK_NUM] = {0, 0, 0};

OS_EVENT      *r1, *r2;
INT8U         err1, err2;
INT32U        LAB3_MODE_PEND = 2;
INT32U        LAB3_MODE_POST = 3;

// #define          TASK_NUM            2          
// INT8U         TASK_INFO[TASK_NUM][2] = {{1,3},{3,6}};
// INT8U         TASK_PRIO[TASK_NUM] = {1, 2};
// INT32U        TASK_CNT[TASK_NUM] = {0, 0};

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks(void);
        void  TaskStart(void *data);
        void  print_buffer(void);
        void  Task1(void *data);
        void  Task2(void *data);
        void  Task3(void *data);   

void print_buffer(void)
{
    
    INT16U i;
    OS_ENTER_CRITICAL();
    for(i = 0;i < lab1_pos;i++){
        if(lab1_output[i][1] == COMPLETE){
            printf("%8ld    complete               %ld            %ld\n",lab1_output[i][0],lab1_output[i][2],lab1_output[i][3]);
        }
        else if(lab1_output[i][1] == PREEMPT){
            if(lab1_output[i][2] != (INT32U)1 && lab1_output[i][3] != (INT32U)1 &&
            lab1_output[i][2] != (INT32U)2 && lab1_output[i][3] != (INT32U)2){
                printf("%8ld    preempt                %ld            %ld\n",lab1_output[i][0],lab1_output[i][2],lab1_output[i][3]);
            }
        }
        else if(lab1_output[i][1] == LAB3_MODE_PEND){
            printf("%8ld    lock      R%ld   (Prio=%ld changes to=%ld)\n",lab1_output[i][0],lab1_output[i][4],lab1_output[i][2],lab1_output[i][3]);
        }
        else if(lab1_output[i][1] == LAB3_MODE_POST){
            printf("%8ld    unlock    R%ld   (Prio=%ld changes to=%ld)\n",lab1_output[i][0],lab1_output[i][4],lab1_output[i][2],lab1_output[i][3]);
        }
        else{}
    }
    lab1_pos = 0;
    OS_EXIT_CRITICAL();
}

void log_buffer(INT32U mode, INT32U prev_prio, INT32U cur_prio, INT32U resource){
    lab1_output[lab1_pos][0] = OSTimeGet();
    lab1_output[lab1_pos][1] = (INT32U)mode;
    lab1_output[lab1_pos][2] = (INT32U)prev_prio;
    lab1_output[lab1_pos][3] = (INT32U)cur_prio;
    lab1_output[lab1_pos][4] = (INT32U)resource;
    lab1_pos = (lab1_pos + 1)% OUTPUT_ROW_N;
}
/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main (void)
{
    OS_STK *ptos;
    OS_STK *pbos;
    INT32U  size;


    PC_DispClrScr(DISP_FGND_WHITE);                        /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    PC_ElapsedInit();                                      /* Initialized elapsed time measurement     */

    ptos        = &TaskStartStk[TASK_STK_SIZE - 1];        /* TaskStart() will use Floating-Point      */
    pbos        = &TaskStartStk[0];
    size        = TASK_STK_SIZE;
    OSTaskStkInit_FPE_x86(&ptos, &pbos, &size);            
    OSTaskCreateExt(TaskStart,
                   (void *)0,
                   ptos,
                   TASK_START_PRIO,
                   TASK_START_ID,
                   pbos,
                   size,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR,
                   1,
                   99998);
    
    r1 = OSMutexCreate(1, &err1);                           /* Lab3: resource r1 */
    r2 = OSMutexCreate(2, &err2);                           /* Lab3: resource r2 */

    OSStart();                                             /* Start multitasking                       */
}
void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */


    OS_ENTER_CRITICAL();                                   /* Install uC/OS-II's clock tick ISR        */
    PC_VectSet(0x08, OSTickISR);
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    lab1_pos = 0;                                          /* lab1                                     */

    TaskStartCreateTasks();                                /* Create all other tasks                   */
    OSTimeSet(1);
    OSTaskDel(0);
}
static  void  TaskStartCreateTasks (void)
{
    OSTaskCreateExt(Task1,
                   (void *)0,
                   &Task1Stk[TASK_STK_SIZE - 1],
                   TASK_PRIO[0],
                   TASK_1_ID,
                   &Task1Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR,
                   TASK_INFO[0][0],
                   TASK_INFO[0][1]);

    OSTaskCreateExt(Task2,
                   (void *)0,
                   &Task2Stk[TASK_STK_SIZE - 1],
                   TASK_PRIO[1],
                   TASK_2_ID,
                   &Task2Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR,
                   TASK_INFO[1][0],
                   TASK_INFO[1][1]);

    OSTaskCreateExt(Task3,
                   (void *)0,
                   &Task3Stk[TASK_STK_SIZE - 1],
                   TASK_PRIO[2],
                   TASK_3_ID,
                   &Task3Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR,
                   TASK_INFO[2][0],
                   TASK_INFO[2][1]);
}

void  Task1 (void *pdata)
{
    int start;
    int end;
    int toDelay;
    INT16S  key;

    int r1_used, r2_used;
    INT8U prev_prio, cur_prio;
    INT32U mode, resource;
    pdata = pdata;
    
    while(1){
        OSTimeDly(8); // Lab3: simulate start time
        start = OSTimeGet(); 
        r1_used = 0;
        r2_used = 0;
        while(OSTCBCur->compTime > 0){
            if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
                if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                    PC_DOSReturn();                            /* Yes, return to DOS                       */
                }
            }
            print_buffer();

            if(OSTCBCur->compTime == 4){                        /* Lab3: Lock R1 */
                if(!r1_used){
                    prev_prio = OSTCBCur->OSTCBPrio;
                    OSMutexPend(r1, 5, &err1);
                    OS_ENTER_CRITICAL();
                    log_buffer(LAB3_MODE_PEND,prev_prio,OSTCBCur->OSTCBPrio,(INT32U)1);
                    r1_used = 1;
                    OS_EXIT_CRITICAL();
                }
                
            }
            else if(OSTCBCur->compTime == 2){                   /* Lab3: Lock R2 */
                if(!r2_used){
                    prev_prio = OSTCBCur->OSTCBPrio;
                    OSMutexPend(r2, 5, &err2);
                    OS_ENTER_CRITICAL();
                    log_buffer(LAB3_MODE_PEND,prev_prio,OSTCBCur->OSTCBPrio,(INT32U)2);
                    r2_used = 1;
                    OS_EXIT_CRITICAL();
                }
            }
        }
        prev_prio = OSTCBCur->OSTCBPrio;
        OSMutexPost(r1);                                        /* Lab3: Release R1 */
        OS_ENTER_CRITICAL();
        log_buffer(LAB3_MODE_POST,prev_prio,OSTCBCur->OSTCBPrio,(INT32U)1);
        OS_EXIT_CRITICAL();
        
        prev_prio = OSTCBCur->OSTCBPrio;
        OSMutexPost(r2);                                        /* Lab3: Release R2 */
        OS_ENTER_CRITICAL();
        log_buffer(LAB3_MODE_POST,prev_prio,OSTCBCur->OSTCBPrio,(INT32U)2);
        OS_EXIT_CRITICAL();
        
        end = OSTimeGet();
        toDelay = (int)(OSTCBCur->period) - (end-start);
        start = start + (OSTCBCur->period);
        if(toDelay < 0)
            toDelay = 0;
        OS_ENTER_CRITICAL();
            OSTCBCur->compTime = TASK_INFO[0][0];
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}

void  Task2 (void *pdata)
{
    int start;
    int end;
    int toDelay;
    INT16S  key;

    int r2_used;
    INT8U prev_prio, cur_prio;
    INT32U mode, resource;
    pdata = pdata;
    
    while(1){
        OSTimeDly(4); // Lab3: simulate start time
        start = OSTimeGet(); 
        r2_used = 0;
        while(OSTCBCur->compTime > 0){
            if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
                if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                    PC_DOSReturn();                            /* Yes, return to DOS                       */
                }
            }
            print_buffer();

            if(OSTCBCur->compTime == 4){                        /* Lab3: Lock R2 */
                if(!r2_used){
                    prev_prio = OSTCBCur->OSTCBPrio;
                    OSMutexPend(r2, 5, &err2);
                    OS_ENTER_CRITICAL();
                    log_buffer(LAB3_MODE_PEND,prev_prio,OSTCBCur->OSTCBPrio,(INT32U)2);
                    r2_used = 1;
                    OS_EXIT_CRITICAL();
                }
                
            }
        }
        prev_prio = OSTCBCur->OSTCBPrio;
        OSMutexPost(r2);                                        /* Lab3: Release R2 */
        OS_ENTER_CRITICAL();
        log_buffer(LAB3_MODE_POST,prev_prio,OSTCBCur->OSTCBPrio,(INT32U)2);
        OS_EXIT_CRITICAL();
        
        end = OSTimeGet();
        toDelay = (int)(OSTCBCur->period) - (end-start);
        start = start + (OSTCBCur->period);
        if(toDelay < 0)
            toDelay = 0;
        OS_ENTER_CRITICAL();
            OSTCBCur->compTime = TASK_INFO[1][0];
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}
void  Task3 (void *pdata)
{
    int start;
    int end;
    int toDelay;
    INT16S  key;

    int r1_used;
    INT8U prev_prio, cur_prio;
    INT32U mode, resource;
    pdata = pdata;
    
    // initialize output
    lab1_pos = 0;
    OSTimeSet(0);
    while(1){
        start = OSTimeGet(); 
        r1_used = 0;
        while(OSTCBCur->compTime > 0){
            if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
                if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                    PC_DOSReturn();                            /* Yes, return to DOS                       */
                }
            }
            print_buffer();

            if(OSTCBCur->compTime == 7){                        /* Lab3: Lock R2 */
                if(!r1_used){
                    prev_prio = OSTCBCur->OSTCBPrio;
                    OSMutexPend(r1, 5, &err1);
                    OS_ENTER_CRITICAL();
                    log_buffer(LAB3_MODE_PEND,prev_prio,OSTCBCur->OSTCBPrio,(INT32U)1);
                    r1_used = 1;
                    OS_EXIT_CRITICAL();
                }
                
            }
        }
        prev_prio = OSTCBCur->OSTCBPrio;
        OSMutexPost(r1);                                        /* Lab3: Release R1 */
        OS_ENTER_CRITICAL();
        log_buffer(LAB3_MODE_POST,prev_prio,OSTCBCur->OSTCBPrio,(INT32U)1);
        OS_EXIT_CRITICAL();
        
        end = OSTimeGet();
        toDelay = (int)(OSTCBCur->period) - (end-start);
        start = start + (OSTCBCur->period);
        if(toDelay < 0)
            toDelay = 0;
        OS_ENTER_CRITICAL();
            OSTCBCur->compTime = TASK_INFO[2][0];
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}
