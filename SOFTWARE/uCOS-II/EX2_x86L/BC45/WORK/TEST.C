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

#define          TASK_START_ID       0                /* Application tasks IDs                         */
#define          TASK_PRINT_ID       1
#define          TASK_1_ID           2
#define          TASK_2_ID           3
#define          TASK_3_ID           4

#define          TASK_START_PRIO    0                /* Application tasks priorities                  */
#define          TASK_PRINT_PRIO      1
// #define          TASK_1_PRIO        12
// #define          TASK_2_PRIO        13
// #define          TASK_3_PRIO        14
// #define          TASK_4_PRIO        15
// #define          TASK_5_PRIO        16

// #define          TASK_1_C           1
// #define          TASK_1_P           3

// #define          TASK_2_C           3
// #define          TASK_2_P           6

// #define          TASK_3_C           3
// #define          TASK_3_P           6

#define          TASK_NUM            3





/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStartStk[TASK_STK_SIZE];            /* Startup    task stack                         */
OS_STK        TaskClkStk[TASK_STK_SIZE];              /* Clock      task stack                         */
OS_STK        Task1Stk[TASK_STK_SIZE];                /* Task #1    task stack                         */
OS_STK        Task2Stk[TASK_STK_SIZE];                /* Task #2    task stack                         */
OS_STK        Task3Stk[TASK_STK_SIZE];                /* Task #3    task stack                         */
INT8U         TASK_INFO[TASK_NUM][2] = {{4,8},{3,8},{1,8}};
INT8U         TASK_PRIO[TASK_NUM] = {2, 3, 4};

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  TaskStart(void *data);                  /* Function prototypes of tasks                  */
        void  TaskPrint(void *data);
static  void  TaskStartCreateTasks(void);
        void  Task1(void *data);
        void  Task2(void *data);
        void  Task3(void *data);
        void  print_buffer();
        void  RM_sort();



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
                   2,
                   4);

    OSStart();                                             /* Start multitasking                       */
}

void print_buffer()
{
    if(lab1_output[0] != '\0'){
        OS_ENTER_CRITICAL();
            //PC_DispStr(0, (++lab1_print_cnt)%30, lab1_output, DISP_FGND_WHITE); // for PC
            printf("%s",lab1_output);
            memset(lab1_output, 0, sizeof(lab1_output));
        OS_EXIT_CRITICAL();
    }
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

    memset(lab1_output, 0, sizeof(lab1_output));           /* lab1                                     */
    lab1_print_cnt = 0;                                    /* lab1                                     */
    prev_print_prio = -1;                                  /* lab1                                     */

    TaskStartCreateTasks();                                /* Create all other tasks                   */
    OSTimeSet(0);
    OSTaskDel(TASK_START_PRIO);

    // for (;;) {

    //     if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
    //         if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
    //             PC_DOSReturn();                            /* Yes, return to DOS                       */
    //         }
    //     }

    //     OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
    //     OSTimeDly(OS_TICKS_PER_SEC);                       /* Wait one second                          */
    // }
}

static  void  TaskStartCreateTasks (void)
{
     OSTaskCreateExt(TaskPrint,
                   (void *)0,
                   &TaskClkStk[TASK_STK_SIZE - 1],
                   TASK_PRINT_PRIO,
                   TASK_PRINT_ID,
                   &TaskClkStk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR,
                   0,
                   0);

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
    INT32U start;
    INT32U end;
    INT32U toDelay;
    INT16S  key;
    start = OSTimeGet();
    pdata = pdata;
    
    while(1){
        while(OSTCBCur->compTime > 0){
            if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
                if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                    PC_DOSReturn();                            /* Yes, return to DOS                       */
                }
            }
        }
        //print_buffer();
        end = OSTimeGet();
        toDelay = (OSTCBCur->period) - (end-start);
        start = start + (OSTCBCur->period);
        OS_ENTER_CRITICAL();
            OSTCBCur->compTime = TASK_INFO[0][0];
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}

void  Task2 (void *pdata)
{
    INT32U start;
    INT32U end;
    INT32U toDelay;
    INT16S  key;
    start = OSTimeGet();
    pdata = pdata;
    while(1){
        while(OSTCBCur->compTime > 0){
            if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
                if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                    PC_DOSReturn();                            /* Yes, return to DOS                       */
                }
            }
        }
        //print_buffer();
        end = OSTimeGet();
        toDelay = (OSTCBCur->period) - (end-start);
        start = start + (OSTCBCur->period);
        OS_ENTER_CRITICAL();
            OSTCBCur->compTime = TASK_INFO[1][0];
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}
void  Task3 (void *pdata)
{
    INT32U start;
    INT32U end;
    INT32U toDelay;
    INT16S  key;
    start = OSTimeGet();
    pdata = pdata;
    while(1){
        while(OSTCBCur->compTime > 0){
            if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
                if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                    PC_DOSReturn();                            /* Yes, return to DOS                       */
                }
            }
        }
        //print_buffer();
        end = OSTimeGet();
        toDelay = (OSTCBCur->period) - (end-start);
        start = start + (OSTCBCur->period);
        OS_ENTER_CRITICAL();
            OSTCBCur->compTime = TASK_INFO[2][0];
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}

void  TaskPrint (void *pdata)
{
    INT16S  key;
    INT32U start;
    INT32U end;
    pdata = pdata;
    
    while(1){
        start = OSTimeGet();
        print_buffer();
        if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Yes, return to DOS                       */
            }
        }
        end = OSTimeGet();
        OS_ENTER_CRITICAL();
        OSTime -= (end-start);
        OS_EXIT_CRITICAL();
        OSTimeDly(10);
    }
}

void RM_sort(){
    // DO NOTHING

    // if(TASK_NUM == 2){
    //     if(TASK_info[0][0]/TASK_info[0][1] < TASK_info[1][0]/TASK_info[1][1]){
    //         TASK_prio = {2,1};
    //     }
    //     else{
    //         TASK_prio = {1,2};
    //     }
    // }
    // else if(TASK_NUM == 3){
    //     if(TASK_info[0][0]/TASK_info[0][1] >= TASK_info[1][0]/TASK_info[1][1]){
    //         if(TASK_info[1][0]/TASK_info[1][1] >= TASK_info[2][0]/TASK_info[2][1]){
    //             TASK_prio = {1,2,3};
    //         }
    //         el
    //     }
    // }
}

