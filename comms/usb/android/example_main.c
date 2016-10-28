/*
 * Remember to Add Android to your system.h file along with definition for your
 * connected function:
 *
 * 
 * Android Definitions:
 *
 *    #define ANDROID
 *
 *    #ifdef ANDROID
 *
 *    Definition of the function used to change State to the App connected
 *    state. If you are going to use an Android connected device you must
 *    define a function for moving into the App Connected state. See example
 *    below:
 *
 *      extern void example_set_app_connected_state(void);
 *      #define ANDROID_SET_APPLICATION_CONNECTED_STATE example_set_app_connected_state();
 *
 *    extern void set_app_connected_state(void);
 *    #define ANDROID_SET_APPLICATION_CONNECTED_STATE set_app_connected_state();
 *
 *    Android main processing when the Android Application is NOT Connected.
 *    If different processing is required when the Android Applicaiton is not
 *    connected to your device then define the function to perform this
 *    functionality and the MACRO.
 *    This is NOT Mandatory. If you project does not require it then don't
 *    define the MACRO.
 *
 *     extern void example_no_android_app_function(void);
 *     #define NO_ANDROID_APP_FN example_no_android_app_function();
 *
 *    //extern void example_no_android_app_function(void);
 *    //#define NO_ANDROID_APP_FN example_no_android_app_function();
 *
 *  In addition to all this you need to link the Microchip files into your src directoy:
 *
 * lrwxrwxrwx  1 john users   64 Jun 27 19:26 usb_hal_local.h -> /opt/microchip/mla/v2014_07_22/framework/usb/src/usb_hal_local.h
 * lrwxrwxrwx  1 john users   67 Jun 27 19:26 usb_host_android.c -> /opt/microchip/mla/v2014_07_22/framework/usb/src/usb_host_android.c
 * lrwxrwxrwx  1 john users   59 Jun 27 19:26 usb_host.c -> /opt/microchip/mla/v2014_07_22/framework/usb/src/usb_host.c
 * lrwxrwxrwx  1 john users   65 Jun 27 19:26 usb_host_local.h -> /opt/microchip/mla/v2014_07_22/framework/usb/src/usb_host_local.h
 * 
 * there is a bash script to link the files
 * Also need usb_handlers.c from es_lib
 *
 *     Also need a heap size  512
 */
#include "es_lib/usb/android/android_comms.h"
#include "es_lib/usb/android/state_idle.h"

/*
 *  Microchip USB Includes
 *
 * For these two inclues include your mla framework directory in the project
 * properties include path:
 *
 *  /opt/microchip/mla/v2014_07_22/framework
 *
 * You must also include the path:
 *
 * es_lib/usb/android
 *
 * In your project properties for the system_config.h file which the Microchip
 * source files expect to find.
 */
#include "usb/usb.h"
#include "usb/usb_host_android.h"

#include "es_lib/firmware/firmware.h"

DEF_FIRMWARE_AUTHOR_40("me@mail.com")
DEF_FIRMWARE_DESCRIPTION_50("My Project")
DEF_FIRMWARE_VERSION_10("v1.0")
DEF_FIRMWARE_URL_50("http://www.me.com")

int main(void)
{
	char manufacturer[40] = "";
	char model[50] = "";
	char version[10] = "";
	char uri[50] = "";
	u16  length;
	ANDROID_ACCESSORY_INFORMATION android_device_info;

	/**
	 * Initialise the current state of the Android Sate machine to Idle
	 * [The link text](@ref set_idle_state)
	 */
	set_idle_state();

	/*
	 * Set up the details that we're going to pass to a connected Android
	 * Device.
	 */
	length = 40;
	flash_strcpy(manufacturer, firmware_author, &length);

	length = 50;
	flash_strcpy(model, firmware_description, &length);

	length = 10;
	flash_strcpy(version, firmware_version, &length);

	length = 50;
	flash_strcpy(uri, firmware_uri, &length);

	LOG_D("manufacturer - %s\n\r", manufacturer);
	LOG_D("model - %s\n\r", model);
	LOG_D("version - %s\n\r", version);
	LOG_D("uri - %s\n\r", uri);

	android_device_info.manufacturer = manufacturer;
	android_device_info.manufacturer_size = sizeof(manufacturer);
	android_device_info.model = model;
	android_device_info.model_size = sizeof(model);
	android_device_info.description = model;
	android_device_info.description_size = sizeof(model);
	android_device_info.version = version;
	android_device_info.version_size = sizeof(version);
	android_device_info.URI = uri;
	android_device_info.URI_size = sizeof(uri);

	asm ("CLRWDT");

	/*
	 * Turn on the power to the USB Port as we're going to use Host Mode
	 */
	USB_HOST

	/*
	 * These next two lines call the Microchip USB Stack functions to
	 * initialise the USB Host and Android functionality
	 */
	AndroidAppStart(&android_device_info);

	asm ("CLRWDT");

	while(TRUE) {
		asm ("CLRWDT");
		//Keep the USB stack running
		USBTasks();
	}
}
