/*
 * This is a template main.c file for a Cinnamon Bun Node Application
 */
#incldue "es_lib/app/app.h"
#include "es_lib/os/os.h"

/*
 * These Strings identify your application. It is important that the first
 * author string is set to something as on power up the Cinnamon Bun's OS will
 * check that this string has a length greater then 0. If the string is not set
 * the the OS assumes that there is no valid Application and will ignore your
 * applicaiton, failing to run any of your code.
 */
DEF_APP_AUTHOR_40("me@mail.com")
DEF_APP_DESCRIPTION_50("My ES CAN Node App")
DEF_APP_VERSION_10("v1.0")
DEF_APP_URL_50("www.test.com")

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
