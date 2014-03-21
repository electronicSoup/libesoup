#ifndef SEND_ANDROID_H
#define SEND_ANDROID_H

extern BOOL androidReceive(BYTE *buffer, UINT16 *size, BYTE *errorCode);
extern BYTE androidTransmit(BYTE *buffer, BYTE Size);
extern BYTE androidTasks(void* device_handle);


#endif /* SEND_ANDROID_H */
