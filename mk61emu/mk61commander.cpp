#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <ctime>

#ifdef _WIN32
#    include <windows.h>
#    include <conio.h>
#endif
#ifdef __unix__
#    include <termios.h>
#    include <unistd.h>
#    include <stdio.h>
#endif

#include "mk_common.h"
#include "mk61commander.h"

mk61_commander::mk61_commander()
    : m_emu(std::make_unique<mk61_emu>())
{}

void mk61_commander::run()
{
    const std::string file_ext = ".mk61";
    std::string cmd;
    std::time_t now, last_time;
    std::time(&last_time);
    bool quit = false;
    bool is_first_time = true;
    while (!quit)
    {
        std::time(&now);
        if (is_first_time || m_emu->is_output_required())
        {
            is_first_time = false;
            output_state();
            std::cout << "# " << cmd << std::endl;
            m_emu->end_output();
        }
        get_command(cmd);
        m_last_parse_result = parse_input(cmd);
        if (m_last_parse_result.parsed == true)
        {
            mk61emu_result result;
            switch (m_last_parse_result.cmd_kind)
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
            {
                std::cout << "File name: ";
                std::string filename;
                std::cin >> filename;
                // TODO check filename
                filename += file_ext;
                try
                {
                    if (m_last_parse_result.cmd_kind == mk_cmd_kind_t::cmd_save)
                    {
                        save_state(filename);
                        mk_show_message(mk_message_t::msg_info, "State saved");
                    }
                    else
                    {
                        load_state(filename);
                        mk_show_message(mk_message_t::msg_info, "State loaded");
                    }
                }
                catch(...)
                {
                    mk_show_message(mk_message_t::msg_error, result.message);
                }
                break;
            }
            case mk_cmd_kind_t::cmd_keys:
            case mk_cmd_kind_t::cmd_unknown:
            case mk_cmd_kind_t::cmd_empty:
                break;
            }
            cmd.clear();
        }
        // Make a step when calculator in the running mode
        if (m_emu->get_power_state() == engine_power_state_t::engine_on)
            m_emu->do_step();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate a delay between steps (macro ticks)
    }
    std::cout << std::endl;
}

void mk61_commander::load_state(const std::string& filename)
{
    std::ifstream data(filename, std::ifstream::binary);
    m_emu->set_state(data);
}

void mk61_commander::save_state(const std::string& filename)
{
    std::ofstream data(filename, std::ofstream::binary);
    m_emu->get_state(data);
}

void mk61_commander::mk_show_message(const mk_message_t message_type, const std::string message)
{
    switch (message_type)
    {
    case mk_message_t::msg_info:
        break;
    case mk_message_t::msg_warn:
        std::cout << "Warning: ";
        break;
    case mk_message_t::msg_error:
        std::cout << "Error: ";
        break;
    }
    std::cout << message << std::endl;
}

void mk61_commander::clear_display()
{
#ifdef __linux__
    const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
    mk_printf(CLEAR_SCREE_ANSI);
#else
#   ifdef _WIN32
    system("cls");
#   else
    clrscr();
#   endif // _WIN32
#endif
}

void mk61_commander::output_display()
{
    std::cout << m_emu->get_indicator_str() << std::endl;
}

void mk61_commander::output_state()
{
    clear_display();
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
    std::cout << "\n"
        << "R0: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R0)
        << " | R1: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R1)
        << " | R2: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R2)
        << " | R3: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R3)
        << "\n"
        << "R4: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R4)
        << " | R5: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R5)
        << " | R6: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R6)
        << "\n"
        << "R7: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R7)
        << " | R8: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R8)
        << " | R9: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R9)
        << " | RA: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RA)
        << "\n"
        << "RB: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RB)
        << " | RC: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RC)
        << " | RD: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RD)
        << " | RE: " << m_emu->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RE)
        << "\n\n"
        << " T: '" << m_emu->get_reg_stack_str(mk61emu_reg_stack_t::RT)
        << "'\tCounter: " << m_emu->get_prog_counter_str()
        << "\tRunning: " << (m_emu->is_running() ? "yes" : "no")
        << "\n"
        << " Z: '" << m_emu->get_reg_stack_str(mk61emu_reg_stack_t::RZ) << "'\n"
        << " Y: '" << m_emu->get_reg_stack_str(mk61emu_reg_stack_t::RY) << "'\n"
        << " X: '" << m_emu->get_reg_stack_str(mk61emu_reg_stack_t::RX) << "'\n"
        << "X1: '" << m_emu->get_reg_stack_str(mk61emu_reg_stack_t::RX1) << "'\n"
        << "\n"
        << "DISPLAY: '" << m_emu->get_indicator_str() << "'"
        << std::endl;
}

int mk61_commander::mk_kbhit(void)
{
#ifdef __linux__
    struct timeval tv;
    fd_set rdfs;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);
    select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
#else
    return kbhit();
#endif
}


int mk61_commander::mk_getch(const bool nowait, const bool echo)
{
    int ch = EOF;
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
    return ch;
}

bool mk61_commander::get_command(std::string& cmd)
{
    bool result = false;
    int ch = mk_getch(true, false);
    if (ch == EOF || ch > 127)
        return result;
    char c = static_cast<char>(ch);
    switch(c)
    {
    case '\b':
        // Delete last character
        if (cmd.size() > 0)
            cmd.resize(cmd.size() - 1);
        break;
    case 27:
        // Clear the whole input character sequence
        cmd.clear();
    default:
        if (c == '\n')
            result = true;
        cmd += c;
    }
    return result;
}

mk_parse_result mk61_commander::parse_input(const std::string& cmd)
{
    mk_parse_result result;
    result.parsed = true;
    result.cmd_kind = mk_cmd_kind_t::cmd_empty;
    if (cmd.size() == 0)
        return result;
    result.cmd_kind = mk_cmd_kind_t::cmd_keys;
    std::string cmd_up(cmd);
    std::transform(cmd_up.begin(), cmd_up.end(), cmd_up.begin(), ::toupper);
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
    else if (cmd_up == "STPL")
        m_emu->do_key_press(7, 9);
    else if (cmd_up == "STPR")
        m_emu->do_key_press(9, 9);
    else if (cmd_up == "RTN")
        m_emu->do_key_press(4, 9);
    else if (cmd_up == "RS")
        m_emu->do_key_press(2, 9);
    else if (cmd_up == "STO")
        m_emu->do_key_press(6, 9);
    else if (cmd_up == "RCL")
        m_emu->do_key_press(8, 9);
    else if (cmd_up == "GTO")
        m_emu->do_key_press(3, 9);
    else if (cmd_up == "GSB")
        m_emu->do_key_press(5, 9);
    else if (cmd_up == "XY")
        m_emu->do_key_press(6, 8);
    else if (cmd_up == "SGN") // /-/
        m_emu->do_key_press(8, 8);
    else if (cmd_up == "EXP")
        m_emu->do_key_press(9, 8);
    else if (cmd_up == "CX")
        m_emu->do_key_press(10, 8);
    else if (cmd_up == "ENT" || cmd == "\n")
        m_emu->do_key_press(11, 8);
    // Modes
    else if (cmd_up == "PRG")
    {
        m_emu->do_key_press(11, 9); // F
        m_emu->do_step();
        m_emu->do_key_press(9, 8); // EXP key
    }
    else if (cmd_up == "AUT")
    {
        m_emu->do_key_press(11, 9); // F
        m_emu->do_step();
        m_emu->do_key_press(8, 8); // /-/ key
    }
    else if (cmd_up == "SAVE")
        result.cmd_kind = mk_cmd_kind_t::cmd_save;
    else if (cmd_up == "LOAD")
        result.cmd_kind = mk_cmd_kind_t::cmd_load;
    // Modes
    else if (cmd_up == "RAD")
        m_emu->set_angle_unit(angle_unit_t::radian);
    else if (cmd_up == "GRAD")
        m_emu->set_angle_unit(angle_unit_t::grade);
    else if (cmd_up == "DEG")
        m_emu->set_angle_unit(angle_unit_t::degree);
    else if (cmd_up == "QUIT")
        result.cmd_kind = mk_cmd_kind_t::cmd_quit;
    else if (cmd_up == "OFF")
        result.cmd_kind = mk_cmd_kind_t::cmd_off;
    else if (cmd_up == "ON")
        result.cmd_kind = mk_cmd_kind_t::cmd_on;
    else
    {
        result.parsed = false;
        result.cmd_kind = mk_cmd_kind_t::cmd_unknown;
    }
    return result;
}
