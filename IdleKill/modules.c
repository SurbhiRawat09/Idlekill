// modules.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <sys/select.h>
#include "modules.h"
#include "hashmap.h"
#include "doubly_linked_list.h"
#include "priority_queue.h"
#include "graph.h"
#include "circular_queue.h"

HashMap *app_activity_map = NULL;
DoublyLinkedList *activity_log = NULL;
PriorityQueue *app_priority_queue = NULL;
Graph *app_dependency_graph = NULL;
CircularQueue *termination_queue = NULL;

bool printed_once = false;
int visible_pids[MAX_VISIBLE_PIDS];
int visible_pid_count = 0;
int last_focused_pid = -1;
int last_displayed_lines = 0;

int get_focused_window_pid() {
    FILE *fp = popen("hyprctl activewindow -j | jq '.pid'", "r");
    if (!fp) return -1;

    char pid_str[16];
    if (fgets(pid_str, sizeof(pid_str), fp)) {
        pclose(fp);
        int pid = atoi(pid_str);
        last_focused_pid = pid;
        return pid;
    }
    pclose(fp);
    return -1;
}

bool is_visible_pid(int pid) {
    for (int i = 0; i < visible_pid_count; i++) {
        if (visible_pids[i] == pid) return true;
    }
    return false;
}

void fetch_visible_pids() {
    visible_pid_count = 0;
    FILE *fp = popen("hyprctl clients -j | jq '.[].pid'", "r");
    if (!fp) return;

    char line[64];
    while (fgets(line, sizeof(line), fp)) {
        if (visible_pid_count >= MAX_VISIBLE_PIDS) break;
        int pid = atoi(line);
        if (pid > 0) visible_pids[visible_pid_count++] = pid;
    }
    pclose(fp);
}

void clear_priority_queue(PriorityQueue *pq) {
    if (!pq) return;

    while (!pq_is_empty(pq)) {
        AppMetrics app = pq_pop(pq);
        if (app.pid) free(app.pid);
        if (app.name) free(app.name);
    }
}

void send_notification(const char *app_name, const char *pid_str, time_t idle_time, int threshold) {
    if (!app_name || !pid_str) return;

    char command[512];
    int minutes = idle_time / 60;
    int threshold_min = threshold / 60;

    snprintf(command, sizeof(command),
        "notify-send 'Idle Application' '%s has been idle for %d minutes (Threshold: %d mins)' "
        "-t 0 "
        "--action=kill='Terminate Now' "
        "--action=ignore='Ignore'",
        app_name, minutes, threshold_min);

    FILE *fp = popen(command, "r");
    if (fp) {
        char response[16];
        if (fgets(response, sizeof(response), fp)) {
            if (strstr(response, "kill")) {
                terminate_process(pid_str, app_name);
            }
        }
        pclose(fp);
    }
}

void terminate_process(const char *pid_str, const char *app_name) {
    if (!pid_str || !app_name) return;

    int pid = atoi(pid_str);
    if (pid <= 0) return;

    if (kill(pid, SIGTERM) == -1) {
        perror("Failed to terminate process");
        return;
    }

    if (app_activity_map) {
        hashmap_remove(app_activity_map, pid_str);
    }

    char confirm[256];
    snprintf(confirm, sizeof(confirm),
        "notify-send 'Terminated' '%s (PID: %s) has been terminated' -t 3000",
        app_name, pid_str);
    system(confirm);

    printf("\033[1;31mTerminated idle process: %s (%s)\033[0m\n", pid_str, app_name);
}

void process_and_store_app(const char *pid_str, const char *app_name, const char *cpu_str, const char *mem_kb_str, int threshold) {
    if (!pid_str || !app_name || !cpu_str || !mem_kb_str) return;

    int pid = atoi(pid_str);
    if (!is_visible_pid(pid)) return;

    float cpu = atof(cpu_str);
    float mem = atof(mem_kb_str) / 1024.0;
    int focused_pid = get_focused_window_pid();

    if (!app_activity_map) return;

    time_t *last_active = hashmap_get_ptr(app_activity_map, pid_str);
    if (pid == focused_pid) {
        if (last_active) {
            *last_active = time(NULL);
        } else {
            hashmap_put(app_activity_map, pid_str, time(NULL));
        }
    } else if (!last_active) {
        hashmap_put(app_activity_map, pid_str, time(NULL));
    }

    time_t idle_time = hashmap_get_idle_time(app_activity_map, pid_str);

    if (idle_time > threshold && idle_time <= threshold + 5) {
        send_notification(app_name, pid_str, idle_time, threshold);
    }

    AppMetrics app = {
        .pid = strdup(pid_str),
        .name = strdup(app_name),
        .cpu = cpu,
        .mem = mem,
        .idle_time = idle_time
    };

    if (app_priority_queue) {
        pq_push(app_priority_queue, app);
    }

    if (termination_queue) {
        cq_enqueue(termination_queue, strdup(pid_str));
    }

    if (app_dependency_graph) {
        if (graph_size(app_dependency_graph) >= MAX_GRAPH_NODES) {
            graph_remove_least_important(app_dependency_graph);
        }
        graph_add_node(app_dependency_graph, pid_str);
    }
}

void clear_displayed_lines() {
    // Clear previously displayed lines
    for (int i = 0; i < last_displayed_lines; i++) {
        printf("\033[K\n");  // Clear line and move down
    }
    printf("\033[%dA", last_displayed_lines);  // Move cursor back up
}

void display_top_processes(int threshold) {
    if (!app_priority_queue) return;

    AppMetrics new_top[TOP_N_PROCESSES];
    time_t now = time(NULL);
    
    // Clear previous display
    clear_displayed_lines();

    printf("\033[1;36m┌───────────────────────────────────────────────────────────────────────┐\033[0m\n");
    printf("\033[1;36m│\033[1;35m Process Monitoring \033[0m\033[1;36m(Threshold: \033[1;33m%d\033[1;36ms) \033[0m\033[1;36m│ \033[1;35mFocused: \033[1;33m%d\033[0m \033[1;36m│\033[0m\n",
           threshold, last_focused_pid);
    printf("\033[1;36m├─────────┬──────────────────────┬─────────┬─────────────┬──────────────┤\033[0m\n");
    printf("\033[1;36m│\033[1;34m PID     \033[0m\033[1;36m│\033[1;34m App Name           \033[0m\033[1;36m│\033[1;34m CPU%%    \033[0m\033[1;36m│\033[1;34m Memory (MB)\033[0m\033[1;36m│\033[1;34m Idle Time  \033[0m\033[1;36m│\033[0m\n");
    printf("\033[1;36m├─────────┼──────────────────────┼─────────┼─────────────┼──────────────┤\033[0m\n");

    int lines_printed = 6;  // Count of header lines

    for (int i = 0; i < TOP_N_PROCESSES; i++) {
        if (!pq_is_empty(app_priority_queue)) {
            new_top[i] = pq_pop(app_priority_queue);
        } else {
            new_top[i] = (AppMetrics){
                .pid = strdup("-"),
                .name = strdup("-"),
                .cpu = 0,
                .mem = 0,
                .idle_time = 0
            };
        }

        int idle_min = new_top[i].idle_time / 60;
        int idle_sec = new_top[i].idle_time % 60;

        const char *color_code = "";
        if (new_top[i].idle_time > threshold * 0.75) color_code = "\033[1;31m";
        else if (new_top[i].idle_time > threshold * 0.5) color_code = "\033[1;33m";
        else color_code = "\033[1;32m";

        printf("\033[1;36m│\033[0m %-7s \033[1;36m│\033[0m %-20s \033[1;36m│\033[0m %-7.2f \033[1;36m│\033[0m %-11.1f \033[1;36m│ %s%02d:%02d\033[0m \033[1;36m│\033[0m\n",
               new_top[i].pid,
               new_top[i].name,
               new_top[i].cpu,
               new_top[i].mem,
               color_code,
               idle_min,
               idle_sec);

        free(new_top[i].pid);
        free(new_top[i].name);
        lines_printed++;
    }

    printf("\033[1;36m└─────────┴──────────────────────┴─────────┴─────────────┴──────────────┘\033[0m\n");
    printf("\033[1;33mPress 'q' to quit | 'r' to refresh | 't' to change threshold\033[0m\n");
    lines_printed += 2;
    
    last_displayed_lines = lines_printed;
}

void fetch_and_process_apps(int threshold) {
    if (!app_priority_queue || !app_activity_map || !termination_queue || !app_dependency_graph) {
        return;
    }

    fetch_visible_pids();
    clear_priority_queue(app_priority_queue);

    FILE *fp = popen("ps -eo pid,comm,%cpu,rss --no-headers", "r");
    if (!fp) {
        perror("ps failed");
        return;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp)) {
        char pid[16], name[64], cpu[16], mem[32];
        if (sscanf(line, "%15s %63s %15s %31s", pid, name, cpu, mem) == 4) {
            process_and_store_app(pid, name, cpu, mem, threshold);
        }
    }

    pclose(fp);
    display_top_processes(threshold);

    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    if (select(1, &fds, NULL, NULL, &tv) > 0) {
        char c = getchar();
        if (c == 'q') {
            printf("\033[1;31mExiting...\033[0m\n");
            exit(0);
        } else if (c == 'r') {
            printf("\033[1;32mRefreshing...\033[0m\n");
        } else if (c == 't') {
            printf("\033[1;34mEnter new threshold in seconds: \033[0m");
            char input[10];
            fgets(input, sizeof(input), stdin);
            int new_threshold = atoi(input);
            if (new_threshold > 0) {
                threshold = new_threshold;
                printf("\033[1;32mThreshold updated to %d seconds\033[0m\n", threshold);
            } else {
                printf("\033[1;31mInvalid threshold value\033[0m\n");
            }
        }
    }
}

void cleanup_process_panel() {
    if (app_activity_map) {
        hashmap_free(app_activity_map);
        app_activity_map = NULL;
    }

    if (activity_log) {
        dll_free(activity_log);
        activity_log = NULL;
    }

    if (app_priority_queue) {
        clear_priority_queue(app_priority_queue);
        pq_free(app_priority_queue);
        app_priority_queue = NULL;
    }

    if (app_dependency_graph) {
        graph_free(app_dependency_graph);
        app_dependency_graph = NULL;
    }

    if (termination_queue) {
        while (!cq_is_empty(termination_queue)) {
            char *pid = (char *)cq_dequeue(termination_queue);
            if (pid) free(pid);
        }
        free(termination_queue->pids);
        free(termination_queue);
        termination_queue = NULL;
    }
}

void init_process_panel() {
    cleanup_process_panel();

    app_activity_map = hashmap_create(100);
    activity_log = dll_create(100);
    app_priority_queue = pq_create(100);
    app_dependency_graph = graph_create(100);
    termination_queue = cq_create(100);

    atexit(cleanup_process_panel);
}

void init_services_panel() {}
void init_startup_panel() {}
void init_users_panel() {}
void init_performance_panel() {}
void init_app_history() {}
void init_details_panel() {}
