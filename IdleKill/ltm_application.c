#include <stdio.h>
#include "ltm_application.h"
#include "modules.h"

void start_ltm_application() {
    printf("[LTM] Initializing modules...\n");

    init_process_panel();
    init_services_panel();
    init_startup_panel();
    init_users_panel();
    init_performance_panel();
    init_app_history();
    init_details_panel();

    printf("[LTM] All modules initialized.\n");
}

