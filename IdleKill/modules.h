#ifndef MODULES_H
#define MODULES_H

#include <time.h>

// Constants
#define MAX_LINE_LEN 512
#define TOP_N_PROCESSES 5
#define MAX_VISIBLE_PIDS 128
#define MAX_GRAPH_NODES 30
#define DEFAULT_IDLE_THRESHOLD 300 // 5 minutes in seconds

// Function declarations
int get_focused_window_pid();
void terminate_process(const char *pid_str, const char *app_name);
void send_notification(const char *app_name, const char *pid_str, time_t idle_time, int threshold);

void init_process_panel();
void init_services_panel();
void init_startup_panel();
void init_users_panel();
void init_performance_panel();
void init_app_history();
void init_details_panel();
void fetch_and_process_apps(int threshold);

#endif

