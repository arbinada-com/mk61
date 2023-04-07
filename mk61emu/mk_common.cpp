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
#endif

#include "mk_common.h"

/**
 * MK72Engine
 */

mk_engine::mk_engine()
{
    m_outputRequired = false;
    m_powerState = engine_power_state_t::engine_off;
}

mk_engine::~mk_engine()
{

}

bool mk_engine::is_output_required()
{
    return m_outputRequired;
}

mk_result_t mk_engine::end_output()
{
    m_outputRequired = false;
    return mk_result_t::mk_ok;
}


engine_power_state_t mk_engine::get_power_state()
{
    return m_powerState;
}

mk_result_t mk_engine::set_power_state(const engine_power_state_t value)
{
    if (m_powerState != value)
    {
        m_powerState = value;
    }
    return mk_result_t::mk_ok;
}

/**
 * Memory managment adapters
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
 * Platform I/O
 */

int mk_kbhit(void)
{
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
}

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
 * Read character if key pressed
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
    printf(format, args);
    va_end(args);
    return result;
}

char *mk_gets(char *str, int num)
{
    char *result = str;
    memset(result, 0, num);
    gets(str);
    return result;
}

int mk_puts(const char *str)
{
    return puts(str);
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
 * String ops
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
 * Timing
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
