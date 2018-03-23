

#include "libesoup_config.h"

int main()
{
	result_t  rc;
	
	rc = libesoup_init();
	if(rc < 0) {
		// ERROR
	}
	
	while(1) {
		Nop();
	}
}