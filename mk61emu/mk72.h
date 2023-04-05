#ifndef MK72_H_INCLUDED
#define MK72_H_INCLUDED

#if defined(ENERGIA)
#define TARGET_CALC
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#ifdef TARGET_WINDOWS
#include <conio.h>
#endif // TARGET_WINDOWS

// assert.h отсутствует в msp430
#ifdef MSP430
#   define assert(expr)
#else
#   include <assert.h>
#endif

// Целевая платформа компиляции
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
#define MAX_CHARS_FOR_UTF8_SYMBOL   3  // Все используемые русские символы кодируются в utf-8 максимум 3 байтами
#define MAX_LINE_LEN_UTF8           MAX_LINE_LEN * MAX_CHARS_FOR_UTF8_SYMBOL
#define MAX_FILE_NAME               64

/**
 * Используемая таблица символов - Windows-1251
 * http://en.wikipedia.org/wiki/Windows-1251
 */
#define L_A    0xC0  // А
#define L_B    0xC1  // Б
#define L_V    0xC2  // В
#define L_G    0xC3  // Г
#define L_D    0xC4  // Д
#define L_E    0xC5  // Е
#define L_ZH   0xC6  // Ж
#define L_Z    0xC7  // З
#define L_I    0xC8  // И
#define L_J    0xC9  // Й
#define L_K    0xCA  // К
#define L_L    0xCB  // Л
#define L_M    0xCC  // М
#define L_N    0xCD  // Н
#define L_O    0xCE  // О
#define L_P    0xCF  // П
#define L_R    0xD0  // Р
#define L_S    0xD1  // С
#define L_T    0xD2  // Т
#define L_U    0xD3  // У
#define L_F    0xD4  // Ф
#define L_H    0xD5  // Х
#define L_C    0xD6  // Ц
#define L_CH   0xD7  // Ч
#define L_SH   0xD8  // Ш
#define L_SCH  0xD9  // Щ
#define L_TZ   0xDA  // Ъ
#define L_Y    0xDB  // Ы
#define L_MZ   0xDC  // Ь
#define L_EH   0xDD  // Э
#define L_JU   0xDE  // Ю
#define L_JA   0xDF  // Я

#define LL_A    "\xC0"
#define LL_B    "\xC1"
#define LL_V    "\xC2"
#define LL_G    "\xC3"
#define LL_D    "\xC4"
#define LL_E    "\xC5"
#define LL_ZH   "\xC6"
#define LL_Z    "\xC7"
#define LL_I    "\xC8"
#define LL_J    "\xC9"
#define LL_K    "\xCA"
#define LL_L    "\xCB"
#define LL_M    "\xCC"
#define LL_N    "\xCD"
#define LL_O    "\xCE"
#define LL_P    "\xCF"
#define LL_R    "\xD0"
#define LL_S    "\xD1"
#define LL_T    "\xD2"
#define LL_U    "\xD3"
#define LL_F    "\xD4"
#define LL_H    "\xD5"
#define LL_C    "\xD6"
#define LL_CH   "\xD7"
#define LL_SH   "\xD8"
#define LL_SCH  "\xD9"
#define LL_TZ   "\xDA"
#define LL_Y    "\xDB"
#define LL_MZ   "\xDC"
#define LL_EH   "\xDD"
#define LL_JU   "\xDE"
#define LL_JA   "\xDF"

#define CL_A   (char)L_A
#define CL_B   (char)L_B
#define CL_V   (char)L_V
#define CL_G   (char)L_G
#define CL_D   (char)L_D
#define CL_E   (char)L_E
#define CL_ZH  (char)L_ZH
#define CL_Z   (char)L_Z
#define CL_I   (char)L_I
#define CL_J   (char)L_J
#define CL_K   (char)L_K
#define CL_L   (char)L_L
#define CL_M   (char)L_M
#define CL_N   (char)L_N
#define CL_O   (char)L_O
#define CL_P   (char)L_P
#define CL_R   (char)L_R
#define CL_S   (char)L_S
#define CL_T   (char)L_T
#define CL_U   (char)L_U
#define CL_F   (char)L_F
#define CL_H   (char)L_H
#define CL_C   (char)L_C
#define CL_CH  (char)L_CH
#define CL_SH  (char)L_SH
#define CL_SCH (char)L_SCH
#define CL_TZ  (char)L_TZ
#define CL_Y   (char)L_Y
#define CL_MZ  (char)L_MZ
#define CL_EH  (char)L_EH
#define CL_JU  (char)L_JU
#define CL_JA  (char)L_JA

typedef unsigned char byte;

// Абстрактный класс для реализации режима использования в устройстве МК72 (МК61, Рапира, Форт...)
enum MK72Result
{
    MK_OK = 0,
    MK_ERROR
};

enum MK72EnginePowerState
{
    MK72Engine_Off = 0,
    MK72Engine_On = 1
};

class MK72Engine
{
public:
    MK72Engine();
    virtual ~MK72Engine();
    virtual bool IsOutputRequired();
    virtual MK72Result EndOutput();
    virtual MK72Result DoStep() = 0;
    virtual MK72Result DoInput(const char* buf, size_t length) = 0;
    virtual MK72Result DoKeyPress(const int key1, const int key2) = 0;
    virtual MK72EnginePowerState GetPowerState();
    virtual MK72Result SetPowerState(const MK72EnginePowerState value);
protected:
    bool m_outputRequired;
private:
    MK72EnginePowerState m_powerState;
};

// Адаптеры управления памятью
EXTERN_C void *mk_calloc (size_t n, size_t size);
EXTERN_C void *mk_malloc (size_t size);
EXTERN_C void mk_free(void *p);

// Ввод-вывод для целевой платформы (калькулятор, консоль Linux, консоль Windows...)
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
#define MSG_MK_NOT_ENOUGH_MEMORY  LL_N LL_E LL_D LL_O LL_S LL_T LL_A LL_T LL_O LL_N LL_O " " LL_P LL_A LL_M LL_JA LL_T LL_I // Недостаточно памяти
#define MSG_MK_UNKNOWN_RESULT  LL_N LL_E LL_I LL_Z LL_V LL_E LL_S LL_T LL_N LL_Y LL_J " " LL_R LL_E LL_Z LL_U LL_L LL_T LL_A LL_T // Неизвестный результат
#define MSG_MK_FILE_ERROR  LL_O LL_SH LL_I LL_B LL_K LL_A " " LL_O LL_T LL_K LL_R LL_Y LL_T LL_I LL_JA // Ошибка открытия. Файл не существует или нет прав
#define MSG_MK_FILE_READ_ERROR LL_O LL_SH LL_I LL_B LL_K LL_A " " LL_CH LL_T LL_E LL_N LL_I LL_JA // Ошибка чтения
#define MSG_MK_FILE_WRITE_ERROR LL_O LL_SH LL_I LL_B LL_K LL_A " " LL_Z LL_A LL_P LL_I LL_S LL_I // Ошибка записи
#define MSG_MK_DEVICE_IS_OFF  LL_U LL_S LL_T LL_R LL_O LL_J LL_S LL_T LL_O " " LL_V LL_Y LL_K LL_L LL_JU LL_CH LL_E LL_N LL_O // Устройство выключено
// Операции со строками
EXTERN_C int mk_strcasecmp(char const *str1, char const *str2);

// Тайминг
EXTERN_C int mk_sleep(const int milliseconds);

// Полезные функции
EXTERN_C uint8_t ctoui8(const char c);

#endif // MK72_H_INCLUDED
