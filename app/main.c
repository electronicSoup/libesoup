/*
 * This is a template main.c file for a Cinnamon Bun Node Application
 */
#include "es_lib/os/os.h"

/*
 * These Strings identify your application. It is important that the first
 * author string is set to something as on power up the Cinnamon Bun's OS will
 * check that this string has a length greater then 0. If the string is not set
 * the the OS assumes that there is no valid Application and will ignore your
 * applicaiton, failing to run any of your code.
 */
#define APP_STRINGS_BASE 0x18000
__prog__ char app_author[40] __attribute__((space(prog), address(APP_STRINGS_BASE))) = "me@bogus.com";
__prog__ char app_software[50] __attribute__((space(prog), address(APP_STRINGS_BASE + 40))) = "My Supper cool App";
__prog__ char app_version[10] __attribute__((space(prog), address(APP_STRINGS_BASE + 40 + 50))) = "1.0";
__prog__ char app_uri[50] __attribute__((space(prog), address(APP_STRINGS_BASE + 40 + 50 + 10))) = "http:/www.bogus.com";

#define TAG "MyAPP"

/*
 * Like the Arduino sketche's setup() function this is the initialisation code
 * which will be called by the OS on power up and immediately after the
 * Application has been installed on the Cinnamon Bun Device.
 * This routine must return in a timely fashion! If the watch dog timer kicks
 * in resetting the device before this funciton returns the application will
 * be marked as invalid by the OS.
 */
void app_init(void)
{
    /*
     * This funciton call to initialise the OS API Functions is very important!
     * If you are going to call OS API functons you must first have called
     * this function. Otherwise your code will jump to random locations in
     * program memory.
     */
    os_init();

    os_serial_log(Debug, TAG, "app_init()\n\r");
}


void app_main(void)
{
}
