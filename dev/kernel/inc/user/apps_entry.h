/* User program entry points */
#ifndef _USER_APPS_ENTRY_H_
#define _USER_APPS_ENTRY_H_

void user_init();
void init_user();
void noise();

/* Rock paper scissors */
void rps_game();

/* Send/Receive/Reply benchmark */
void srr_bench();

/* clock server clients */
void lazy_dog();

/* A0 train program */
void train_control();
void train_module();
void train_clock();
void train_sensor();
void train_switches();

#endif /* _USER_APPS_ENTRY_H_ */
