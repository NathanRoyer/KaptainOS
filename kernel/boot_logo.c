#include "boot_logo.h"
#include "../services/drivers/cpu_timer.h"
#include "../services/drivers/teletype.h"

char * bootlogo = "\n\n\n\n\n\n\n"
"  #    ##                                                    ######## ########\n"
"  #   #                                                      #      # #\n"
"  #   #                       #                #             #      # #\n"
"  #   #                       #                              #      # #\n"
"  ####       #### #   #####   #####   #### #   #    ####     #      # ########\n"
"  #   #     #    ##  #     #  #      #    ##  ##   #    #    #      #        #\n"
"  #   #     #     #  #     #  #      #     #   #   #    #    #      #        #\n"
"  #   #     #    ##  #     #  #      #    ##   #   #    #    #      #        #\n"
"  #    ##    #### ## ######    ##     #### ##   #  #    #    ######## ########\n"
"                     #\n"
"                     #\n"
"                     #";

void show_boot_logo(u32 secs){
	clear_screen();
	kprint_color(bootlogo, 0b00000100);
	sleep(secs);
	clear_screen();
}
