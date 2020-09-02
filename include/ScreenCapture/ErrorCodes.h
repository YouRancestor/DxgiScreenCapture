#ifndef ERRORCODES_H
#define ERRORCODES_H


/**
 * Return values:
 * E_INVALID_ARGUMENTS - Atleast one of the arguments is invalid.
 * E_INVALID_DISPLAY_INDEX - The VideoOutput device's index is invalid.
 * E_INVALID_ADAPTER_INDEX - The VideoAdapter device's index is invalid.
 * E_DEVICE_NOT_SURPPORT - This program is not available on the current device.
 * E_UNKNOWN - Unknown error.
 * E_OK - Success.
 * E_TIMEOUT - Time out.
 * E_NOCHANGE - The screen image has no change.
 * E_AGAIN - Some error occured, but not serious, try again may fix the problem.
 */
#define E_INVALID_ARGUMENTS -5
#define E_INVALID_DISPLAY_INDEX -4
#define E_INVALID_ADAPTER_INDEX -3
#define E_DEVICE_NOT_SURPPORT -2
#define E_UNKNOWN -1
#define E_OK 0
#define E_TIMEOUT 1
#define E_NO_CHANGE 2
#define E_AGAIN 3

#endif // ERRORCODES_H
