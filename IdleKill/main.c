// main.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "ltm_application.h"
#include "modules.h"

#define IDLE_NOTIFICATION_THRESHOLD 120  // Default 2 minutes

void display_splash() {
    printf("\033[1;36m");
    printf("  _____  _____  _____  _____  _  __ \n");
    printf(" |_   _||_   _||_   _||_   _|| |/ / \n");
    printf("   | |    | |    | |    | |  | ' /  \n");
    printf("   | |    | |    | |    | |  | . \\  \n");
    printf("  _| |_  _| |_  _| |_  _| |_ |_|\\_\\ \n");
    printf(" |_____||_____||_____||_____|(_)(_) \n");
    printf("\033[0m");
    printf("\n\033[1;33m[ IdleKill - Process Monitor ]\033[0m\n");
    printf("\033[1;32m--------------------------------\033[0m\n");
}

int get_user_threshold() {
    int threshold = IDLE_NOTIFICATION_THRESHOLD;
    char input[10];

    printf("\n\033[1;34mConfigure Idle Time Threshold\033[0m\n");
    printf("\033[1;37mCurrent default: %d seconds (%.1f minutes)\033[0m\n",
           IDLE_NOTIFICATION_THRESHOLD, IDLE_NOTIFICATION_THRESHOLD / 60.0);
    printf("\033[1;36mEnter custom threshold in seconds (0 for default): \033[0m");

    fgets(input, sizeof(input), stdin);
    int custom = atoi(input);
    if (custom > 0) {
        threshold = custom;
    }

    return threshold;
}

int main() {
    display_splash();
    int threshold = get_user_threshold();
    init_process_panel();

    // Clear screen before entering main loop
    printf("\033[2J\033[H");
    
    while (1) {
        printf("\033[H");  // Move cursor to top-left
        fetch_and_process_apps(threshold);
        usleep(500000); // Sleep for 0.5s between refreshes
    }

    return 0;
}
