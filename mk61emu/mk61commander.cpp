#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef TARGET_CALC
#include <time.h>
#endif

#include "mk72.h"
#include "char_conv.h"
#include "mk61commander.h"

MK61Commander::MK61Commander()
{
    m_emu = new MK61Emu();
    m_last_parse_result.parsed = false;
    m_last_parse_result.cmd_kind = mk61cmd_unknown;
}

MK61Commander::~MK61Commander()
{
    delete m_emu;
}

void MK61Commander::Run()
{
    const char file_ext[] = ".mk61";
    char cmd[MAX_LINE_LEN_UTF8 + 1];
    memset(cmd, 0, MAX_LINE_LEN_UTF8 + 1);
    time_t now, last_time;
    time(&last_time);
    bool quit = false;
    bool firstTime = true;
    while (!quit)
    {
        time(&now);
        if (firstTime || m_emu->IsOutputRequired())
        {
            firstTime = false;
            OutputState();
            mk_printf("# %s\n", cmd);
            m_emu->EndOutput();
        }
        GetCommand(cmd, MAX_LINE_LEN_UTF8);
        this->m_last_parse_result = ParseInput(cmd);
        if (this->m_last_parse_result.parsed == true)
        {
            mk61emu_result_t result;
            char name_raw[MAX_FILE_NAME + 1];
            char name[MAX_FILE_NAME + sizeof(file_ext) + 1];
            switch (this->m_last_parse_result.cmd_kind)
            {
                case mk61cmd_quit:
                    quit = true;
                    break;
                case mk61cmd_off:
                    m_emu->SetPowerState(MK72Engine_Off);
                    break;
                case mk61cmd_on:
                    m_emu->SetPowerState(MK72Engine_On);
                    break;
                case mk61cmd_load:
                case mk61cmd_save:
                    memset(name_raw, 0, MAX_FILE_NAME + 1);
                    mk_printf("Имя файла: ");
                    if (InputName(name_raw, MAX_FILE_NAME) == false)
                    {
                        mk_show_message(mk_message_error, "Неправильное имя. Допустимы только заглавные буквы, цифры и _.");
                        break;
                    }
                    memset(name, 0, MAX_FILE_NAME + sizeof(file_ext) + 1);
                    transliterate_from_windows1251(name_raw, name, MAX_FILE_NAME);
                    strcat(name, file_ext);
                    if (this->m_last_parse_result.cmd_kind == mk61cmd_save)
                        m_emu->SaveState(name, &result);
                    else
                        m_emu->LoadState(name, &result);
                    if (result.succeeded == true)
                        mk_show_message(mk_message_info,
                                        this->m_last_parse_result.cmd_kind == mk61cmd_save ?
                                        "Состояние сохранено" : "Состояние загружено");
                    else
                        mk_show_message(mk_message_error, result.message);
                    break;
                case mk61cmd_keys:
                case mk61cmd_unknown:
                    break;
                case mk61cmd_empty:
                    break;
            }
            memset(cmd, 0, MAX_LINE_LEN_UTF8 + 1);
        }
        // Если калькулятор в режиме счета по программе, то делаем шаг
        if (m_emu->GetPowerState() == MK72Engine_On)
            m_emu->DoStep();
        mk_sleep(100); // Симуляция задержки между шагами (макротактами)
    }
    puts("\n");
}

int MK61Commander::mk_show_message(const mk_message_t message_type, const char *format, ... )
{
    va_list args;
    va_start(args, format);
    int result = -1;
    switch (message_type)
    {
        case mk_message_info:
            break;
        case mk_message_warn:
            mk_printf(LL_P LL_R LL_E LL_D LL_U LL_P LL_R LL_E LL_ZH LL_D LL_E LL_N LL_I LL_E ": ");
            break;
        case mk_message_error:
            mk_printf(LL_O LL_SH LL_I LL_B LL_K LL_A ": ");
            break;
    }
    char buf[MAX_LINE_LEN + 1];
    memset(buf, 0, sizeof(buf));
    vsnprintf(buf, MAX_LINE_LEN, format, args);
    mk_printf("%s\n", buf);
    mk_printf(LL_N LL_A LL_ZH LL_M LL_I LL_T LL_E " " LL_V LL_V LL_O LL_D "\n");
    mk_getchar();
    va_end(args);
    return result;
}

void MK61Commander::OutputState()
{
    mk_clear_display();
    printf("Эмулятор МК61 версия %d.%d\n", MK61EMU_VERSION_MAJOR, MK61EMU_VERSION_MINOR);
    printf("QUIT или ВЫХ - выход из программы\n");
    printf("ON/OFF или ВКЛ/ВЫКЛ - включение/выключение\n");
    printf("SAVE/LOAD или ЗАГР/СОХР - сохранить/загрузить состояние (программу, регистры...)\n");
    printf("Клавиши: Ф(F) ШГП(STPR) ШГЛ(STPL) В0(RTN) СП(RS)\n");
    printf("К(K) ХП(STO) ИП(RCL) БП(GTO) ПП(GSB)\n");
    printf("0..9 ,(.) ЗН(SGN) ВП(E) ХУ(XY) + - * / а б с д е\n");
    printf("CX(СХ) клавиша Enter или ВВОД(ENT)\n");
    printf("Команды: ПРГ(PRG) АВТ(AUT) РАД ГРАД ГРД\n\n");

    if (m_emu->GetPowerState() == MK72Engine_On)
    {
        printf("ВКЛЮЧЕН\t\tУгол: ");
        mk_printf("%s\n", m_emu->GetAngleUnitStr());
    }
    else
    {
        printf("ВЫКЛЮЧЕН\n");
    }
    printf("\n");
    printf("R0: %s | R1: %s | R2: %s | R3: %s\n",
           m_emu->GetRegMemStr(mk61emu_R0),
           m_emu->GetRegMemStr(mk61emu_R1),
           m_emu->GetRegMemStr(mk61emu_R2),
           m_emu->GetRegMemStr(mk61emu_R3));
    printf("R4: %s | R5: %s | R6: %s\n",
           m_emu->GetRegMemStr(mk61emu_R4),
           m_emu->GetRegMemStr(mk61emu_R5),
           m_emu->GetRegMemStr(mk61emu_R6));
    printf("R7: %s | R8: %s | R9: %s | RA: %s\n",
           m_emu->GetRegMemStr(mk61emu_R7),
           m_emu->GetRegMemStr(mk61emu_R8),
           m_emu->GetRegMemStr(mk61emu_R9),
           m_emu->GetRegMemStr(mk61emu_RA));
    printf("RB: %s | RC: %s | RD: %s | RE: %s\n",
           m_emu->GetRegMemStr(mk61emu_RB),
           m_emu->GetRegMemStr(mk61emu_RC),
           m_emu->GetRegMemStr(mk61emu_RD),
           m_emu->GetRegMemStr(mk61emu_RE));
    printf("\n");
    printf(" T: '%s'\tСчётчик: %s\tСчёт: %s\n",
           m_emu->GetRegStackStr(mk61emu_RT),
           m_emu->GetProgCounterStr(),
           m_emu->IsRunning() ? "да" : "нет");
    printf(" Z: '%s'\n", m_emu->GetRegStackStr(mk61emu_RZ));
    printf(" Y: '%s'\n", m_emu->GetRegStackStr(mk61emu_RY));
    printf(" X: '%s'\n", m_emu->GetRegStackStr(mk61emu_RX));
    printf("X1: '%s'\n", m_emu->GetRegStackStr(mk61emu_RX1));
    printf("\n");
    printf("ИНДИКАТОР: '%s'\n", m_emu->GetIndicatorStr());
}

bool MK61Commander::GetCommand(char *cmdstr, int num)
{
    bool result = false;
    char curr[2];
    int i = 0;
    int c = mk_getch(true, false);
    switch(c)
    {
        case '\b':
            // Удаление последнего введенного символа
            i = strlen(cmdstr);
            if (i > 0 && i < num)
                cmdstr[i - 1] = 0;
            break;
        case 27:
            // Очистка всей введенной последовательности
            memset(cmdstr, 0, num);
        default:
            if (c == '\n')
                result = true;
            memset(curr, 0, 2);
            if ((c > 31 && c < 127) || c == '\n')
            {
                curr[0] = c;
                strcat(cmdstr, curr);
            }
            else if (c > 127)
            {
#ifdef __unix__
                curr[0] = convert_char_utf8_to_windows1251(c);
#else
                curr[0] = c;
#endif
                strcat(cmdstr, curr);
            }
    }
    return result;
}

mk61commander_parse_result_t MK61Commander::ParseInput(const char *cmd)
{
    mk61commander_parse_result_t result;
    result.parsed = true;
    result.cmd_kind = mk61cmd_empty;
    if (strlen(cmd) == 0)
        return result;

    result.cmd_kind = mk61cmd_keys;
    // Клавиши
    if (cmd[0] > 47 && cmd[0] < 58)
        m_emu->DoKeyPress(((uint8_t)cmd[0]) - 46, 1); // Цифры 0..9
    else if (cmd[0] > 95 && cmd[0] < 106) // Символы `abcdefghi
        m_emu->DoKeyPress(((uint8_t)cmd[0]) - 94, 1);
    else if (cmd[0] == 'F' || cmd[0] == CL_F)
        m_emu->DoKeyPress(11, 9);
    else if (cmd[0] == 'K' || cmd[0] == CL_K)
        m_emu->DoKeyPress(10, 9);
    else if (cmd[0] == '*')
        m_emu->DoKeyPress(4, 8);
    else if (cmd[0] == '+')
        m_emu->DoKeyPress(2, 8);
    else if (cmd[0] == '-')
        m_emu->DoKeyPress(3, 8);
    else if (cmd[0] == '/')
        m_emu->DoKeyPress(5, 8);
    else if (cmd[0] == ',' || cmd[0] == '.' || cmd[0] == 'A' || cmd[0] == 'a')
        m_emu->DoKeyPress(7, 8);
    else if (strcmp(cmd, LL_SH LL_G LL_L) == 0 || strcmp(cmd, "STPL") == 0) // "ШГЛ"
        m_emu->DoKeyPress(7, 9);
    else if (strcmp(cmd, LL_SH LL_G LL_P) == 0 || strcmp(cmd, "STPR") == 0) // ШГП
        m_emu->DoKeyPress(9, 9);
    else if (strcmp(cmd, "RTN") == 0 || strcmp(cmd, LL_V "0") == 0)
        m_emu->DoKeyPress(4, 9);
    else if (strcmp(cmd, "RS") == 0 || strcmp(cmd, LL_S LL_P) == 0)
        m_emu->DoKeyPress(2, 9);
    else if (strcmp(cmd, "STO") == 0 || strcmp(cmd, LL_H LL_P) == 0)
        m_emu->DoKeyPress(6, 9);
    else if (strcmp(cmd, "RCL") == 0 || strcmp(cmd, LL_I LL_P) == 0)
        m_emu->DoKeyPress(8, 9);
    else if (strcmp(cmd, "GTO") == 0 || strcmp(cmd, LL_B LL_P) == 0)
        m_emu->DoKeyPress(3, 9);
    else if (strcmp(cmd, "GSB") == 0 || strcmp(cmd, LL_P LL_P) == 0)
        m_emu->DoKeyPress(5, 9);
    else if (strcmp(cmd, "XY") == 0 || strcmp(cmd, LL_H LL_U) == 0)
        m_emu->DoKeyPress(6, 8);
    else if (strcmp(cmd, "SGN") == 0 || strcmp(cmd, LL_Z LL_N) == 0) // /-/
        m_emu->DoKeyPress(8, 8);
    else if (strcmp(cmd, "E") == 0 || strcmp(cmd, LL_V LL_P) == 0)
        m_emu->DoKeyPress(9, 8);
    else if (strcmp(cmd, "СХ") == 0 || strcmp(cmd, LL_C LL_H) == 0)
        m_emu->DoKeyPress(10, 8);
    else if (strcmp(cmd, "\n") == 0 || strcmp(cmd, "ENT") == 0 || strcmp(cmd, LL_V LL_V LL_O LL_D) == 0)
        m_emu->DoKeyPress(11, 8);
    // Команды
    else if (strcmp(cmd, LL_P LL_R LL_G) == 0 || strcmp(cmd, "PRG") == 0)
    {
        m_emu->DoKeyPress(11, 9); // F
        m_emu->DoStep();
        m_emu->DoKeyPress(9, 8); // клавиша ВП
    }
    else if (strcmp(cmd, LL_A LL_V LL_T) == 0 || strcmp(cmd, "AUT") == 0)
    {
        m_emu->DoKeyPress(11, 9); // F
        m_emu->DoStep();
        m_emu->DoKeyPress(8, 8); // клавиша /-/
    }
    else if (strcmp(cmd, "SAVE") == 0 || strcmp(cmd, LL_S LL_O LL_H LL_R) == 0)
        result.cmd_kind = mk61cmd_save;
    else if (strcmp(cmd, "LOAD") == 0 || strcmp(cmd, LL_Z LL_A LL_G LL_R) == 0)
        result.cmd_kind = mk61cmd_load;
    // Команды режимов
    else if (strcmp(cmd, "RAD") == 0 || strcmp(cmd, LL_R LL_A LL_D) == 0)
        m_emu->SetAngleUnit(angle_unit_radian);
    else if (strcmp(cmd, "GRAD") == 0 || strcmp(cmd, LL_G LL_R LL_A LL_D) == 0)
        m_emu->SetAngleUnit(angle_unit_grade);
    else if (strcmp(cmd, "DEG") == 0 || strcmp(cmd, LL_G LL_R LL_D) == 0)
        m_emu->SetAngleUnit(angle_unit_degree);
    else if (strcmp(cmd, "QUIT") == 0 || strcmp(cmd, LL_V LL_Y LL_H) == 0)
        result.cmd_kind = mk61cmd_quit;
    else if (strcmp(cmd, "OFF") == 0 || strcmp(cmd, LL_V LL_Y LL_K LL_L) == 0)
        result.cmd_kind = mk61cmd_off;
    else if (strcmp(cmd, "ON") == 0 || strcmp(cmd, LL_V LL_K LL_L) == 0)
        result.cmd_kind = mk61cmd_on;
    else
    {
        result.parsed = false;
        result.cmd_kind = mk61cmd_unknown;
    }
    return result;
}

bool MK61Commander::InputName(char *str, size_t max_size)
{
    bool result = false;
    mk_gets(str, max_size);
    int i = 0, len = strlen(str);
    if (len > 0)
        str[--len] = 0; // Удаляем '\n'
    for (i = 0; i < len; i++)
    {
        unsigned char c = str[i];
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c == '_') || (c >= L_A  && c <= L_JA))
            result = true;
        else
        {
            result = false;
            break;
        }
    }
    return result;
}
