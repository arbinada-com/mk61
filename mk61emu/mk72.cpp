#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>

/* mk_getch implementation */
#ifdef TARGET_CALC
#   include "msp.h"
#else
#   ifdef _WIN32
#       include <windows.h>
#       include <conio.h>
#   else
#       include <time.h>
#   endif
#   ifdef __unix__
#       include <termios.h>
#       include <unistd.h>
#       include <stdio.h>
#   endif
#endif

#ifndef TARGET_CALC
#   include "char_conv.h"
#endif

#include "mk72.h"

/**
 * MK72Engine
 */

MK72Engine::MK72Engine()
{
    m_outputRequired = false;
    m_powerState = MK72Engine_Off;
}

MK72Engine::~MK72Engine()
{

}

bool MK72Engine::IsOutputRequired()
{
    return m_outputRequired;
}

MK72Result MK72Engine::EndOutput()
{
    m_outputRequired = false;
    return MK_OK;
}


MK72EnginePowerState MK72Engine::GetPowerState()
{
    return m_powerState;
}

MK72Result MK72Engine::SetPowerState(const MK72EnginePowerState value)
{
    if (m_powerState != value)
    {
        m_powerState = value;
    }
    return MK_OK;
}

/**
 * Адаптеры управления памятью
 */

void *mk_calloc (size_t n, size_t size)
{
    return calloc(n, size);
}

void *mk_malloc (size_t size)
{
	return malloc(size);
}

void mk_free(void *p)
{
    free(p);
}

/**
 * Ввод-вывод для целевой платформы (калькулятор, консоль Linux, консоль Windows...)
 */

static size_t utf8_length(const char lead)
{
    unsigned char x = ~((unsigned char) lead);
    unsigned int b = 0;
    if (x & 0xf0)
        x >>= 4;
    else
        b += 4;
    if (x & 0x0c)
        x >>= 2;
    else
        b += 2;
    if (! (x & 0x02) )
        b++;
    return b;
}


int mk_kbhit(void)
{
#ifdef TARGET_CALC
	// TODO
	return 0;
#else
#ifdef __linux__
    struct timeval tv;
    fd_set rdfs;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rdfs);
    FD_SET (STDIN_FILENO, &rdfs);
    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
#else
    return kbhit();
#endif
#endif
}

/*
 * Возвращает однобайтный код символа для ASCII (<128)
 * или 1..4-байтный код Utf-8
 */
int mk_getchar()
{
#   ifdef __linux__
#define UTF8_ERROR  ((int)(-2))
    const int c = getchar();
    if (c < 0)
        return EOF;
    const size_t len = utf8_length(c);
    if (len < 2) // ASCII
        return c;

    unsigned int uc = c & ( (1 << (7 - len)) - 1 );
    size_t i;
    for(i = 0 ; i + 1 < len ; i++)
    {
        const int c = getchar();
        if (c != -1 && ( c >> 6 ) == 0x2)
        {
            uc <<= 6;
            uc |= (c & 0x3f);
        }
        else if (c == -1)
        {
            return EOF;
        }
        else
        {
            return UTF8_ERROR;
        }
    }
    return (int) uc;
#   else
    return getchar();
#   endif
}

/*
 * Чтение символа, если была нажата клавиша
 */
int mk_getch(const bool nowait, const bool echo)
{
    int ch = EOF;
#ifdef TARGET_CALC
    // TODO
#else
#ifdef __unix__
    struct termios oldattr, newattr;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~ICANON;
    if (echo == true)
        newattr.c_lflag &= ~ECHO;
    if (nowait == true)
    {
        newattr.c_cc[VMIN] = 1;
        newattr.c_cc[VTIME] = 0;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    if (!nowait || mk_kbhit() != 0)
        ch = mk_getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
#else
    if (nowait == false || kbhit())
    {
        if (echo == true)
            ch = getche();
        else
            ch = getch();
    }
#endif
#endif
    return ch;
}


void mk_clear_display()
{
#ifdef TARGET_CALC
// TODO
#else
#   ifdef __linux__
    const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
    mk_printf(CLEAR_SCREE_ANSI);
#   else
#       ifdef _WIN32
    system("cls");
#       else
    clrscr();
#       endif // _WIN32
#   endif
#endif
}

int mk_printf(const char *format, ... )
{
    va_list args;
    va_start(args, format);
    int result = -1;
#ifdef TARGET_CALC
    // TODO
    printf(format, args);
#else
    char buf_cp1251[MAX_LINE_LEN + 1];
    memset(buf_cp1251, 0, sizeof(buf_cp1251));
    if ((result = vsnprintf(buf_cp1251, MAX_LINE_LEN, format, args)) > 0)
    {
        char buf_utf8[MAX_LINE_LEN_UTF8 + 1];
        memset(buf_utf8, 0, sizeof(buf_utf8));
        if (convert_windows1251_to_utf8(buf_cp1251, buf_utf8, MAX_LINE_LEN_UTF8))
            result = printf("%s", buf_utf8);
        else
            result =  -1;
    }
#endif
    va_end(args);
    return result;
}

char *mk_gets(char *str, int num)
{
    char *result = str;
    memset(result, 0, num);
#ifdef TARGET_CALC
    gets(str);
#else
    char buf_utf8[MAX_LINE_LEN_UTF8 + 1];
    memset(buf_utf8, 0, MAX_LINE_LEN_UTF8 + 1);
    if ((result = fgets(buf_utf8, MAX_LINE_LEN_UTF8, stdin)) != NULL)
    {
        if (convert_utf8_to_windows1251(buf_utf8, str, num))
            result = str;
    }
#endif
    return result;
}

int mk_puts(const char *str)
{
#ifdef TARGET_CALC
// TODO
    return -1;
#else
    char buf_utf8[MAX_LINE_LEN_UTF8 + 1];
    memset(buf_utf8, 0, sizeof(buf_utf8));
    if (convert_windows1251_to_utf8(str, buf_utf8, MAX_LINE_LEN_UTF8))
        return puts(buf_utf8);
    return -1;
#endif
}

mk_file_result_t mk_load_file(const char *name, char *buffer, const uint32_t size)
{
    mk_file_result_t result = mk_file_ok;
#ifdef TARGET_CALC
// TODO
#else
    FILE * f;
    f = fopen(name, "r");
    if (f != NULL)
    {
        size_t count = fread(buffer, sizeof(char), size, f);
        if (count != size)
            result = mk_file_read_error;
        fclose (f);
    }
    else
        result = mk_file_open_error;
#endif
    return result;
}

mk_file_result_t mk_save_file(const char *name, const char *buffer, const uint32_t size)
{
    mk_file_result_t result = mk_file_ok;
#ifdef TARGET_CALC
// TODO
#else
    FILE * f;
    f = fopen(name, "w");
    if (f != NULL)
    {
        size_t count = fwrite(buffer, sizeof(char), size, f);
        if (count != size)
            result = mk_file_write_error;
        fclose (f);
    }
    else
        result = mk_file_open_error;
#endif
    return result;
}

/**
 * Операции со строками
 */
int mk_strcasecmp(char const *str1, char const *str2)
{
    for (;; str1++, str2++)
    {
        int d = tolower(*str1) - tolower(*str2);
        if (d != 0 || !*str1)
            return d;
    }
}


/**
 * Тайминг
 */
int mk_sleep(const int milliseconds)
{
#ifdef TARGET_CALC

// TODO
    return 0;
#else
#ifdef _WIN32
SetLastError(0);
Sleep(milliseconds);
return GetLastError() ? -1 : 0;
#else
struct timespec req, rem;
req.tv_sec = 0;
req.tv_nsec = milliseconds * 1000000;
return nanosleep(&req, &rem);
#endif
#endif
}

/**
 * Полезные функции
 */

uint8_t ctoui8(const char c)
{
    uint8_t result = (uint8_t)c - (uint8_t)('0');
    return result;
}
