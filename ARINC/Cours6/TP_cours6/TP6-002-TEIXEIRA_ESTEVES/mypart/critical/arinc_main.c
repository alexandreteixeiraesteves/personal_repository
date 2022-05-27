/* Code for Processor:0  Partition:1*/
/* Includes: */
/* Include files for RPi653 */
#include <arinc653/error.h>
#include <arinc653/partition.h>
#include <arinc653/process.h>
#include <arinc653/queuing.h>
#include <arinc653/time.h>
#include <libc/string.h>

#define RPi653
#include <libpartition/lopht-debug.h>

/* External function definitions */
#include <applicative/definitions.h>

/* Definition of boolean type */
typedef int bool ;

/* Queuing port configuration */
int number_of_ports = 3;
struct queuing_port_data {
  QUEUING_PORT_NAME_TYPE name ;
  MESSAGE_SIZE_TYPE max_msg_size ;
  PORT_DIRECTION_TYPE direction ;
} port_data[3] = 
{
  {"port_myrpi_critical_VL1000",sizeof(int),DESTINATION},
  {"port_myrpi_critical_VL1001",sizeof(int),DESTINATION},
  {"port_myrpi_critical_VL1002",sizeof(bool),SOURCE},
};
QUEUING_PORT_ID_TYPE port_id[3]={0,1,2};

/* Task configuration */
int number_of_tasks = 6;
const NAME_TYPE task_names[6] = {
  "task_blk_0",
  "task_blk_1",
  "task_blk_2",
  "task_blk_6",
  "task_blk_10",
  "task_blk_11",
};
PROCESS_ATTRIBUTE_TYPE pat[6];
PROCESS_ID_TYPE pid[6];
int task_running[6];
RETURN_CODE_TYPE rc[6];

/* Constant declarations */
int Engine__fs_sensor_addr;
int Engine__hs_sensor_addr;

/* Partition variable declaration
   (including rotating registers) */
/* variable 0*/
bool var_0 ;
/* variable 1*/
bool var_1[2] ;
int var_1_cnt ;
/* variable 6*/
int var_6 ;
/* variable 11*/
int var_11 ;

/* Task code declarations */
MESSAGE_SIZE_TYPE recv_size_3port_myrpi_critical_VL1000;
char recv_buf_3port_myrpi_critical_VL1000[sizeof(int)];
MESSAGE_SIZE_TYPE recv_size_4port_myrpi_critical_VL1001;
char recv_buf_4port_myrpi_critical_VL1001[sizeof(int)];
int aux_block_11_i ;

/* Task code. */
void task_blk_0(){
  for(;;){
      task_running[0] = 1 ;
      Externc__read_bool_sensor_step(&Engine__fs_sensor_addr,&var_0);
      task_running[0] = 0 ;
    SUSPEND_SELF(INFINITE_TIME_VALUE,&rc[0]) ;
    console_perror(rc[0],"critical","task_0_SUSPEND_SELF");
  }
}
void task_blk_1(){
  for(;;){
    var_1_cnt = (var_1_cnt + 1)%2;
      task_running[1] = 1 ;
      Externc__read_bool_sensor_step(&Engine__hs_sensor_addr,&var_1[var_1_cnt]);
      task_running[1] = 0 ;
    SUSPEND_SELF(INFINITE_TIME_VALUE,&rc[1]) ;
    console_perror(rc[1],"critical","task_1_SUSPEND_SELF");
  }
}
void task_blk_2(){
  for(;;){
      task_running[2] = 1 ;
      SEND_QUEUING_MESSAGE(
          port_id[2],
          (APEX_BYTE*)&var_1[var_1_cnt],
          sizeof(bool),
          0 /*no timeout*/ ,
          &rc[2]
        );
      console_perror(rc[2],"critical","task_2_SEND");
      task_running[2] = 0 ;
    SUSPEND_SELF(INFINITE_TIME_VALUE,&rc[2]) ;
    console_perror(rc[2],"critical","task_2_SUSPEND_SELF");
  }
}
void task_blk_6(){
  for(;;){
    if(var_1[var_1_cnt]){
      task_running[3] = 1 ;
      RECEIVE_QUEUING_MESSAGE(
          port_id[0],
          0 /*no timeout*/ ,
          (APEX_BYTE*)recv_buf_3port_myrpi_critical_VL1000,
          &recv_size_3port_myrpi_critical_VL1000,
          &rc[3]
        );
      console_perror(rc[3],"critical","task_3_RECV");
      memcpy((char*)&(var_6),recv_buf_3port_myrpi_critical_VL1000,sizeof(int));
      task_running[3] = 0 ;
    }
    SUSPEND_SELF(INFINITE_TIME_VALUE,&rc[3]) ;
    console_perror(rc[3],"critical","task_3_SUSPEND_SELF");
  }
}
void task_blk_10(){
  for(;;){
    if(!(var_1[(var_1_cnt+1)%2])){
      task_running[4] = 1 ;
      RECEIVE_QUEUING_MESSAGE(
          port_id[1],
          0 /*no timeout*/ ,
          (APEX_BYTE*)recv_buf_4port_myrpi_critical_VL1001,
          &recv_size_4port_myrpi_critical_VL1001,
          &rc[4]
        );
      console_perror(rc[4],"critical","task_4_RECV");
      memcpy((char*)&(var_11),recv_buf_4port_myrpi_critical_VL1001,sizeof(int));
      task_running[4] = 0 ;
    }
    SUSPEND_SELF(INFINITE_TIME_VALUE,&rc[4]) ;
    console_perror(rc[4],"critical","task_4_SUSPEND_SELF");
  }
}
void task_blk_11(){
  for(;;){
      task_running[5] = 1 ;
      if(var_0)aux_block_11_i=12345;
      else if((!(var_0))&&(var_1[(var_1_cnt+1)%2]))aux_block_11_i=var_6;
      else if((!(var_0))&&(!(var_1[(var_1_cnt+1)%2])))aux_block_11_i=var_11;
      Externc__act_step(&aux_block_11_i);
      task_running[5] = 0 ;
    SUSPEND_SELF(INFINITE_TIME_VALUE,&rc[5]) ;
    console_perror(rc[5],"critical","task_5_SUSPEND_SELF");
  }
}

/* Task entry points, needed for init */
const void* task_entry_points[6];

/* Deadline miss handler. Very simple, for now. */
RETURN_CODE_TYPE main_rc ;
void deadline_miss(int task_id) {
  debug_printf("Deadline miss on task %d.",task_id);
  STOP(pid[task_id],&main_rc);
  console_perror(main_rc,"critical","deadline_miss_STOP");
}

/* Sequencer is considered separately from the normal tasks */
const NAME_TYPE sequencer_name = "sequencer" ;
PROCESS_ATTRIBUTE_TYPE sequencer_pat;
PROCESS_ID_TYPE sequencer_pid;
int init_cycle_counter ;
int cycle_counter ;
void sequencer() { 
  init_cycle_counter = 0 ;
  cycle_counter = 0 ;
  /* Sync with start of first window in the second MTF */
  TIMED_WAIT(557056,&main_rc);
  console_perror(main_rc,"critical","seq_MTF_sync_TIMED_WAIT");
  for(;;) {
    debug_printf("Cycle:%d.\n",cycle_counter);

    /* Actions scheduled at date 131072 */
    {
      /* Start instance of task 1*/
      RESUME(pid[1],&main_rc);
      console_perror(main_rc,"critical","seq_0_START_1");
    }
    TIMED_WAIT(16384,&main_rc);
    console_perror(main_rc,"critical","seq_0_TIMED_WAIT");
    /* Actions scheduled at date 163840 */
    {
      /* Start instance of task 2*/
      RESUME(pid[2],&main_rc);
      console_perror(main_rc,"critical","seq_1_START_2");
    }
    {
      /* Deadline task 1*/
      if(task_running[1])deadline_miss(1) ;
    }
    TIMED_WAIT(16384,&main_rc);
    console_perror(main_rc,"critical","seq_1_TIMED_WAIT");
    /* Actions scheduled at date 196608 */
    if(init_cycle_counter >= 1) {
      /* Start instance of task 4*/
      RESUME(pid[4],&main_rc);
      console_perror(main_rc,"critical","seq_2_START_4");
    }
    {
      /* Deadline task 2*/
      if(task_running[2])deadline_miss(2) ;
    }
    TIMED_WAIT(16384,&main_rc);
    console_perror(main_rc,"critical","seq_2_TIMED_WAIT");
    /* Actions scheduled at date 229376 */
    if(init_cycle_counter >= 1) {
      /* Start instance of task 5*/
      RESUME(pid[5],&main_rc);
      console_perror(main_rc,"critical","seq_3_START_5");
    }
    if(init_cycle_counter >= 1) {
      /* Deadline task 4*/
      if(task_running[4])deadline_miss(4) ;
    }
    TIMED_WAIT(147455,&main_rc);
    console_perror(main_rc,"critical","seq_3_TIMED_WAIT");
    /* Actions scheduled at date 393216 */
    if(init_cycle_counter >= 1) {
      /* Deadline task 5*/
      if(task_running[5])deadline_miss(5) ;
    }
    {
      /* Start instance of task 3*/
      RESUME(pid[3],&main_rc);
      console_perror(main_rc,"critical","seq_4_START_3");
    }
    TIMED_WAIT(16384,&main_rc);
    console_perror(main_rc,"critical","seq_4_TIMED_WAIT");
    /* Actions scheduled at date 425984 */
    {
      /* Deadline task 3*/
      if(task_running[3])deadline_miss(3) ;
    }
    TIMED_WAIT(81920,&main_rc);
    console_perror(main_rc,"critical","seq_5_TIMED_WAIT");
    /* Actions scheduled at date 524288 */
    {
      /* Start instance of task 0*/
      RESUME(pid[0],&main_rc);
      console_perror(main_rc,"critical","seq_6_START_0");
    }
    TIMED_WAIT(16384,&main_rc);
    console_perror(main_rc,"critical","seq_6_TIMED_WAIT");
    /* Actions scheduled at date 557056 */
    {
      /* Deadline task 0*/
      if(task_running[0])deadline_miss(0) ;
    }
    TIMED_WAIT(540671,&main_rc);
    console_perror(main_rc,"critical","seq_7_TIMED_WAIT");
    /* Update cycle counters */
    cycle_counter += 1 ;
    if(init_cycle_counter<=1) init_cycle_counter++;
  }
}

/* Entry point of the partition. */
int cnt_i ;/* general-purpose counter */
void main_process() {
  task_entry_points[0] = (void*)task_blk_0;
  task_entry_points[1] = (void*)task_blk_1;
  task_entry_points[2] = (void*)task_blk_2;
  task_entry_points[3] = (void*)task_blk_6;
  task_entry_points[4] = (void*)task_blk_10;
  task_entry_points[5] = (void*)task_blk_11;
  /* Constant var initializations. */
Engine__fs_sensor_addr = 4096;
Engine__hs_sensor_addr = 8192;
  /* Variable instance counter initializations */
  var_1_cnt = 1 ;
  /* Delay output initializations */
  /* Under RPi653, there is currently no need for init. */
  /* Task initialization */
  for(cnt_i=0;cnt_i<number_of_tasks;cnt_i++){
    pat[cnt_i].PERIOD = INFINITE_TIME_VALUE ; /* aperiodic process */
    pat[cnt_i].TIME_CAPACITY = INFINITE_TIME_VALUE ;
    pat[cnt_i].ENTRY_POINT = (void*)task_entry_points[cnt_i];
    pat[cnt_i].STACK_SIZE = 0x1000 ;
    pat[cnt_i].BASE_PRIORITY = 0x1 ; /* all tasks have the same */
    pat[cnt_i].DEADLINE = HARD ;
    strcpy(pat[cnt_i].NAME,task_names[cnt_i]) ;
    CREATE_PROCESS(&pat[cnt_i],&pid[cnt_i],&main_rc) ;
    console_perror(main_rc,"critical","main_CREATE_PROCESS");
    START(pid[cnt_i],&main_rc) ;
    console_perror(main_rc,"critical","main_START");
    SUSPEND(pid[cnt_i],&main_rc) ;
    console_perror(main_rc,"critical","main_SUSPEND");
    task_running[cnt_i] = 0 ;
  }
  {
    sequencer_pat.PERIOD = INFINITE_TIME_VALUE ; /* periodic process */ 
    sequencer_pat.TIME_CAPACITY = INFINITE_TIME_VALUE ;
    sequencer_pat.ENTRY_POINT = (void*)sequencer;
    sequencer_pat.STACK_SIZE = 0x1000 ;
    sequencer_pat.BASE_PRIORITY = 0x2 ; /* greater than tasks */
    sequencer_pat.DEADLINE = HARD ;
    strcpy(sequencer_pat.NAME,sequencer_name) ;
    CREATE_PROCESS(&sequencer_pat,&sequencer_pid,&main_rc) ;
    console_perror(main_rc,"critical","main_CREATE_PROCESS_sequencer");
    DELAYED_START(sequencer_pid,0,&main_rc) ;
    console_perror(main_rc,"critical","main_DELAYED_START_sequencer");
  }
  SET_PARTITION_MODE(NORMAL,&main_rc) ;
    console_perror(main_rc,"critical","main_SET_PARTITION_MODE");
  for(;;);
}
