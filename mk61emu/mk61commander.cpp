#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef TARGET_CALC
#include <time.h>
#endif

#include "mk_common.h"
#include "mk61commander.h"

mk61_commander::mk61_commander()
    : m_emu(std::make_unique<mk61_emu>())
{}

void mk61_commander::run()
{
    const char file_ext[] = ".mk61";
    char cmd[MAX_LINE_LEN + 1];
    memset(cmd, 0, MAX_LINE_LEN + 1);
    time_t now, last_time;
    time(&last_time);
    bool quit = false;
    bool is_first_time = true;
    while (!quit)
    {
        time(&now);
        if (is_first_time || m_emu->is_output_required())
        {
            is_first_time = false;
            output_state();
            mk_printf("# %s\n", cmd);
            m_emu->end_output();
        }
        get_command(cmd, MAX_LINE_LEN);
        this->m_last_parse_result = parse_input(cmd);
        if (this->m_last_parse_result.parsed == true)
        {
            mk61emu_result_t result;
            char name_raw[MAX_FILE_NAME + 1];
            char name[MAX_FILE_NAME + sizeof(file_ext) + 1];
            switch (this->m_last_parse_result.cmd_kind)
            {
            case mk_cmd_kind_t::cmd_quit:
                quit = true;
                break;
            case mk_cmd_kind_t::cmd_off:
                m_emu->set_power_state(engine_power_state_t::engine_off);
                break;
            case mk_cmd_kind_t::cmd_on:
                m_emu->set_power_state(engine_power_state_t::engine_on);
                break;
            case mk_cmd_kind_t::cmd_load:
            case mk_cmd_kind_t::cmd_save:
                memset(name_raw, 0, MAX_FILE_NAME + 1);
                mk_printf("File name: ");
                if (input_name(name_raw, MAX_FILE_NAME) == false)
                {
                    mk_show_message(mk_message_t::msg_error, "Invalid name. Only uppercase letters and '_' allowed.");
                    break;
                }
                memset(name, 0, MAX_FILE_NAME + sizeof(file_ext) + 1);
                strcat(name, file_ext);
                if (this->m_last_parse_result.cmd_kind == mk_cmd_kind_t::cmd_save)
                    m_emu->save_state(name, &result);
                else
                    m_emu->load_state(name, &result);
                if (result.succeeded == true)
                    mk_show_message(mk_message_t::msg_info,
                                    this->m_last_parse_result.cmd_kind == mk_cmd_kind_t::cmd_save ?
                                    "State saved" : "State loaded");
                else
                    mk_show_message(mk_message_t::msg_error, result.message);
                break;
            case mk_cmd_kind_t::cmd_keys:
            case mk_cmd_kind_t::cmd_unknown:
                break;
            case mk_cmd_kind_t::cmd_empty:
                break;
            }
            memset(cmd, 0, MAX_LINE_LEN + 1);
        }
        // Make a step when calculator in the running mode
        if (m_emu->get_power_state() == engine_power_state_t::engine_on)
            m_emu->do_step();
        mk_sleep(100); // Simulate a delay between steps (macro ticks)
    }
    puts("\n");
}

int mk61_commander::mk_show_message(const mk_message_t message_type, const char *format, ... )
{
    va_list args;
    va_start(args, format);
    int result = -1;
    switch (message_type)
    {
    case mk_message_t::msg_info:
        break;
    case mk_message_t::msg_warn:
        mk_printf("Warning: ");
        break;
    case mk_message_t::msg_error:
        mk_printf("Error: ");
        break;
    }
    char buf[MAX_LINE_LEN + 1];
    memset(buf, 0, sizeof(buf));
    vsnprintf(buf, MAX_LINE_LEN, format, args);
    mk_printf("%s\n", buf);
    mk_printf("Press <ENTER>\n");
    mk_getchar();
    va_end(args);
    return result;
}

void mk61_commander::output_state()
{
    mk_clear_display();
    std::cout << "MK61 emulator v" << MK61EMU_VERSION_MAJOR << "." << MK61EMU_VERSION_MINOR << "\n"
        << "QUIT quit the program\n"
        << "ON OFF calculator on or off\n"
        << "SAVE LOAD save or load state (program, memory...)\n"
        << "Keys: F STPR STPL RTN RS\n"
        << "    K STO RCL GTO GSB\n"
        << "    0..9 . SGN EXP XY + - * / A B C D E\n"
        << "    CX Enter ENT\n"
        << "Modes: PRG AUT RAD GRAD DEG\n"
        << std::endl;

    if (m_emu->get_power_state() == engine_power_state_t::engine_on)
    {
        std::cout << "ON\t\tAngle: " << m_emu->get_angle_unit_str() << std::endl;
    }
    else
    {
        std::cout << "OFF" << std::endl;
    }
    printf("\n");
    printf("R0: %s | R1: %s | R2: %s | R3: %s\n",
           m_emu->get_reg_mem_str(mk61emu_R0),
           m_emu->get_reg_mem_str(mk61emu_R1),
           m_emu->get_reg_mem_str(mk61emu_R2),
           m_emu->get_reg_mem_str(mk61emu_R3));
    printf("R4: %s | R5: %s | R6: %s\n",
           m_emu->get_reg_mem_str(mk61emu_R4),
           m_emu->get_reg_mem_str(mk61emu_R5),
           m_emu->get_reg_mem_str(mk61emu_R6));
    printf("R7: %s | R8: %s | R9: %s | RA: %s\n",
           m_emu->get_reg_mem_str(mk61emu_R7),
           m_emu->get_reg_mem_str(mk61emu_R8),
           m_emu->get_reg_mem_str(mk61emu_R9),
           m_emu->get_reg_mem_str(mk61emu_RA));
    printf("RB: %s | RC: %s | RD: %s | RE: %s\n",
           m_emu->get_reg_mem_str(mk61emu_RB),
           m_emu->get_reg_mem_str(mk61emu_RC),
           m_emu->get_reg_mem_str(mk61emu_RD),
           m_emu->get_reg_mem_str(mk61emu_RE));
    printf("\n");
    printf(" T: '%s'\tCounter: %s\tRunning: %s\n",
           m_emu->get_reg_stack_str(mk61emu_RT),
           m_emu->get_prog_counter_str(),
           m_emu->is_running() ? "yes" : "no");
    printf(" Z: '%s'\n", m_emu->get_reg_stack_str(mk61emu_RZ));
    printf(" Y: '%s'\n", m_emu->get_reg_stack_str(mk61emu_RY));
    printf(" X: '%s'\n", m_emu->get_reg_stack_str(mk61emu_RX));
    printf("X1: '%s'\n", m_emu->get_reg_stack_str(mk61emu_RX1));
    printf("\n");
    printf("DISPLAY: '%s'\n", m_emu->get_indicator_str());
}

bool mk61_commander::get_command(char *cmdstr, int num)
{
    bool result = false;
    char curr[2];
    int i = 0;
    int c = mk_getch(true, false);
    switch(c)
    {
    case '\b':
        // Delete last character
        i = strlen(cmdstr);
        if (i > 0 && i < num)
            cmdstr[i - 1] = 0;
        break;
    case 27:
        // Clear the whole input character sequence
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
            curr[0] = c;
            strcat(cmdstr, curr);
        }
    }
    return result;
}

mk_parse_result mk61_commander::parse_input(const char *cmd)
{
    mk_parse_result result;
    result.parsed = true;
    result.cmd_kind = mk_cmd_kind_t::cmd_empty;
    if (strlen(cmd) == 0)
        return result;

    result.cmd_kind = mk_cmd_kind_t::cmd_keys;
    // Keys
    if (cmd[0] > 47 && cmd[0] < 58)
        m_emu->do_key_press(((uint8_t)cmd[0]) - 46, 1); // Digits 0..9
    else if (cmd[0] > 95 && cmd[0] < 106) // Characters `abcdefghi
        m_emu->do_key_press(((uint8_t)cmd[0]) - 94, 1);
    else if (cmd[0] == 'F')
        m_emu->do_key_press(11, 9);
    else if (cmd[0] == 'K')
        m_emu->do_key_press(10, 9);
    else if (cmd[0] == '*')
        m_emu->do_key_press(4, 8);
    else if (cmd[0] == '+')
        m_emu->do_key_press(2, 8);
    else if (cmd[0] == '-')
        m_emu->do_key_press(3, 8);
    else if (cmd[0] == '/')
        m_emu->do_key_press(5, 8);
    else if (cmd[0] == ',' || cmd[0] == '.' || cmd[0] == 'A' || cmd[0] == 'a')
        m_emu->do_key_press(7, 8);
    else if (strcmp(cmd, "STPL") == 0)
        m_emu->do_key_press(7, 9);
    else if (strcmp(cmd, "STPR") == 0)
        m_emu->do_key_press(9, 9);
    else if (strcmp(cmd, "RTN") == 0)
        m_emu->do_key_press(4, 9);
    else if (strcmp(cmd, "RS") == 0)
        m_emu->do_key_press(2, 9);
    else if (strcmp(cmd, "STO") == 0)
        m_emu->do_key_press(6, 9);
    else if (strcmp(cmd, "RCL") == 0)
        m_emu->do_key_press(8, 9);
    else if (strcmp(cmd, "GTO") == 0)
        m_emu->do_key_press(3, 9);
    else if (strcmp(cmd, "GSB") == 0)
        m_emu->do_key_press(5, 9);
    else if (strcmp(cmd, "XY") == 0)
        m_emu->do_key_press(6, 8);
    else if (strcmp(cmd, "SGN") == 0) // /-/
        m_emu->do_key_press(8, 8);
    else if (strcmp(cmd, "EXP") == 0)
        m_emu->do_key_press(9, 8);
    else if (strcmp(cmd, "CX") == 0)
        m_emu->do_key_press(10, 8);
    else if (strcmp(cmd, "\n") == 0 || strcmp(cmd, "ENT") == 0)
        m_emu->do_key_press(11, 8);
    // Modes
    else if (strcmp(cmd, "PRG") == 0)
    {
        m_emu->do_key_press(11, 9); // F
        m_emu->do_step();
        m_emu->do_key_press(9, 8); // EXP key
    }
    else if (strcmp(cmd, "AUT") == 0)
    {
        m_emu->do_key_press(11, 9); // F
        m_emu->do_step();
        m_emu->do_key_press(8, 8); // /-/ key
    }
    else if (strcmp(cmd, "SAVE") == 0)
        result.cmd_kind = mk_cmd_kind_t::cmd_save;
    else if (strcmp(cmd, "LOAD") == 0)
        result.cmd_kind = mk_cmd_kind_t::cmd_load;
    // Modes
    else if (strcmp(cmd, "RAD") == 0)
        m_emu->set_angle_unit(angle_unit_radian);
    else if (strcmp(cmd, "GRAD") == 0)
        m_emu->set_angle_unit(angle_unit_grade);
    else if (strcmp(cmd, "DEG") == 0)
        m_emu->set_angle_unit(angle_unit_degree);
    else if (strcmp(cmd, "QUIT") == 0)
        result.cmd_kind = mk_cmd_kind_t::cmd_quit;
    else if (strcmp(cmd, "OFF") == 0)
        result.cmd_kind = mk_cmd_kind_t::cmd_off;
    else if (strcmp(cmd, "ON") == 0)
        result.cmd_kind = mk_cmd_kind_t::cmd_on;
    else
    {
        result.parsed = false;
        result.cmd_kind = mk_cmd_kind_t::cmd_unknown;
    }
    return result;
}

bool mk61_commander::input_name(char *str, size_t max_size)
{
    bool result = false;
    mk_gets(str, max_size);
    int i = 0, len = strlen(str);
    if (len > 0)
        str[--len] = 0; // Delete '\n'
    for (i = 0; i < len; i++)
    {
        unsigned char c = str[i];
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c == '_') || (c >= 'a'  && c <= 'z'))
            result = true;
        else
        {
            result = false;
            break;
        }
    }
    return result;
}
