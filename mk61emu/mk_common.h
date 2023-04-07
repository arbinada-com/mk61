#ifndef MK_COMMON_INCLUDED
#define MK_COMMON_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#ifdef TARGET_WINDOWS
#include <conio.h>
#endif // TARGET_WINDOWS

#include <assert.h>

// Targets
#ifndef TARGET_CALC
#   ifndef TARGET_WIN
#       ifndef TARGET_LINUX
#           ifdef __unix__
#               define TARGET_LINUX
#           else
#               define TARGET_WIN
#           endif
#       endif
#   endif
#endif

#ifndef TARGET_CALC
#ifndef TARGET_WIN
#ifndef TARGET_LINUX
#error Target not defined
#endif
#endif
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define MAX_LINE_LEN                255
#define MAX_FILE_NAME               64


typedef unsigned char byte;

enum class mk_result_t
{
    mk_ok = 0,
    mk_error
};

enum class engine_power_state_t
{
    engine_off = 0,
    engine_on = 1
};

class mk_engine
{
public:
    mk_engine();
    virtual ~mk_engine();
    virtual bool is_output_required();
    virtual mk_result_t end_output();
    virtual mk_result_t do_step() = 0;
    virtual mk_result_t do_input(const char* buf, size_t length) = 0;
    virtual mk_result_t do_key_press(const int key1, const int key2) = 0;
    virtual engine_power_state_t get_power_state();
    virtual mk_result_t set_power_state(const engine_power_state_t value);
protected:
    bool m_outputRequired;
private:
    engine_power_state_t m_powerState;
};

// Memory managing adapters
EXTERN_C void *mk_calloc (size_t n, size_t size);
EXTERN_C void *mk_malloc (size_t size);
EXTERN_C void mk_free(void *p);

// Platform specific I/O (calc, Linux, Windows...)
EXTERN_C void mk_clear_display();
EXTERN_C char *mk_gets(char *str, int num);
EXTERN_C int mk_printf(const char *format, ... );
EXTERN_C int mk_puts(const char *str);
EXTERN_C int mk_getch(const bool nowait, const bool echo);
EXTERN_C int mk_getchar();

typedef enum
{
    mk_file_ok = 0,
    mk_file_open_error,
    mk_file_read_error,
    mk_file_write_error
} mk_file_result_t;

EXTERN_C mk_file_result_t mk_load_file(const char *name, char *buffer, const uint32_t size);
EXTERN_C mk_file_result_t mk_save_file(const char *name, const char *buffer, const uint32_t size);

#define MSG_MK_OK "OK"
#define MSG_MK_NOT_ENOUGH_MEMORY "Not enough memory"
#define MSG_MK_UNKNOWN_RESULT  "Unknown result"
#define MSG_MK_FILE_ERROR  "File error"
#define MSG_MK_FILE_READ_ERROR "File read error"
#define MSG_MK_FILE_WRITE_ERROR "File write error"
#define MSG_MK_DEVICE_IS_OFF  "Device is off"
// String ops
EXTERN_C int mk_strcasecmp(char const *str1, char const *str2);

// Timing
EXTERN_C int mk_sleep(const int milliseconds);

#endif // MK_COMMON_INCLUDED
