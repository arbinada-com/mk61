#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <vector>

#include "mk_common.h"
#include "mk61commander.h"

/*
* emu_runner
*/
emu_runner::emu_runner()
    : m_emu(std::make_unique<mk61_emu>())
{}

emu_runner::~emu_runner()
{
    try
    {
        terminate();
    }
    catch(...)
    { }
}


void emu_runner::start()
{
    m_emu_thread = std::make_unique<std::thread>(&emu_runner::internal_run, this);
}

void emu_runner::terminate()
{
    m_sig_term = true;
    if (m_emu_thread->joinable())
        m_emu_thread->join();
}

void emu_runner::do_step_unsafe()
{
    if (m_emu->get_power_state() == engine_power_state_t::engine_on)
        for (int i = 0; i < 10; ++i) // Step up keys
            m_emu->do_step();
}


mk_result_t emu_runner::do_key_press(const uint8_t key1, const uint8_t key2)
{
    std::lock_guard lock(m_lock);
    auto result = m_emu->do_key_press(key1, key2);
    do_step_unsafe();
    return result;
}

angle_unit_t emu_runner::get_angle_unit()
{
    std::lock_guard lock(m_lock);
    return m_emu->get_angle_unit();
}

engine_power_state_t emu_runner::get_power_state()
{
    std::lock_guard lock(m_lock);
    return m_emu->get_power_state();
}

std::string emu_runner::get_prog_counter_str()
{
    std::lock_guard lock(m_lock);
    return m_emu->get_prog_counter_str();
}

std::string emu_runner::get_reg_mem_str(const mk61emu_reg_mem_t reg)
{
    std::lock_guard lock(m_lock);
    return m_emu->get_reg_mem_str(reg);
}

std::string emu_runner::get_reg_stack_str(const mk61emu_reg_stack_t reg)
{
    std::lock_guard lock(m_lock);
    return m_emu->get_reg_stack_str(reg);
}

bool emu_runner::is_emu_running()
{
    std::lock_guard lock(m_lock);
    return m_emu->is_running();
}

void emu_runner::set_angle_unit(angle_unit_t value)
{
    std::lock_guard lock(m_lock);
    m_emu->set_angle_unit(value);
}

void emu_runner::set_power_state(engine_power_state_t value)
{
    std::lock_guard lock(m_lock);
    m_emu->set_power_state(value);
}


void emu_runner::internal_run()
{
    while (!m_sig_term)
    {
        // Make a step when calculator in the running mode
        if (m_emu->get_power_state() == engine_power_state_t::engine_on && m_emu->is_running())
        {
            std::lock_guard lock(m_lock);
            do_step_unsafe();
        }
        if (m_simulate_delay)
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate a delay between steps (macro ticks)
    }
}

/*
* mk61_commander
*/
mk61_commander::mk61_commander()
    : m_runner(std::make_unique<emu_runner>())
{}

strings_t mk61_commander::parse_cmdline(const std::string& cmdline)
{
    std::istringstream ss(cmdline);
    std::string token;
    strings_t result;
    while (std::getline(ss, token, ' '))
    {
        result.push_back(token);
    }
    return result;
}

void mk61_commander::run()
{
    const std::string default_file_ext = ".mk61";
    bool quit = false;
    show_help();
    m_runner->start();
    while (!quit)
    {
        output_display();
        //output_state();
        std::string cmdline;
        std::getline(std::cin, cmdline);
        strings_t commands = parse_cmdline(cmdline);
        for (size_t i = 0; i < commands.size(); ++i)
        {
            std::string cmd = commands[i];
            mk_parse_result parse_result = parse_input(cmd);
            if (parse_result.parsed == true)
            {
                mk61emu_result result;
                switch (parse_result.cmd_kind)
                {
                case mk_cmd_kind_t::cmd_quit:
                    quit = true;
                    break;
                case mk_cmd_kind_t::cmd_off:
                    m_runner->set_power_state(engine_power_state_t::engine_off);
                    break;
                case mk_cmd_kind_t::cmd_on:
                    m_runner->set_power_state(engine_power_state_t::engine_on);
                    break;
                case mk_cmd_kind_t::cmd_output_state:
                    output_state();
                    break;
                case mk_cmd_kind_t::cmd_help:
                    show_help();
                    break;
                case mk_cmd_kind_t::cmd_load:
                case mk_cmd_kind_t::cmd_save:
                {
                    if (i == commands.size() - 1)
                    {
                        show_message(mk_message_t::msg_error, "Filename expected");
                        break;
                    }
                    std::string filename = commands[++i];
                    // TODO check filename
                    filename += default_file_ext;
                    try
                    {
                        if (parse_result.cmd_kind == mk_cmd_kind_t::cmd_save)
                        {
                            save_state(filename);
                            show_message(mk_message_t::msg_info, "State saved");
                        }
                        else
                        {
                            load_state(filename);
                            show_message(mk_message_t::msg_info, "State loaded");
                        }
                    }
                    catch (std::exception& e)
                    {
                        show_message(mk_message_t::msg_error, e.what());
                    }
                    break;
                }
                case mk_cmd_kind_t::cmd_keys:
                case mk_cmd_kind_t::cmd_unknown:
                case mk_cmd_kind_t::cmd_empty:
                    break;
                }
            }
            if (quit)
                break;
        }
    }
    std::cout << std::endl;
}

void mk61_commander::load_state(const std::string& filename)
{
    std::ifstream data(filename, std::ifstream::binary);
    //m_emu->set_state(data);
}

void mk61_commander::save_state(const std::string& filename)
{
    std::ofstream data(filename, std::ofstream::binary);
    //m_emu->get_state(data);
}

void mk61_commander::show_message(const mk_message_t message_type, const std::string message)
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

void mk61_commander::clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    clrscr();
#endif
}

void mk61_commander::output_display()
{
    std::cout << "RX: " << m_runner->get_reg_stack_str(mk61emu_reg_stack_t::RX) << std::endl;
}

void mk61_commander::show_help()
{
    clear_screen();
    std::cout << "MK61 emulator v" << MK61EMU_VERSION_MAJOR << "." << MK61EMU_VERSION_MINOR << "\n"
        << "Commands:\n"
        << "    HELP to show this help\n"
        << "    QUIT or EXIT to quit the program\n"
        << "    ON to switch calculator on\n"
        << "    OFF to switch calculator off\n"
        << "    SAVE <filename> to save calculator state (program, memory...) to file\n"
        << "    LOAD <filename> to restore calculator state from file\n"
        << "    STATE to show calculator state\n"
        << "Modes:\n"
        << "    PRG AUT RAD GRAD DEG\n"
        << "Keys:\n"
        << "    F STPR STPL RTN RS\n"
        << "    K STO RCL GTO GSB\n"
        << "    0..9 . SGN EXP XY + - * / A B C D E\n"
        << "    CX Enter ENT\n"
        << std::endl;
}

void mk61_commander::output_state()
{
    clear_screen();
    if (m_runner->get_power_state() == engine_power_state_t::engine_on)
    {
        std::cout << "ON\t\tAngle: ";
        switch (m_runner->get_angle_unit())
        {
        case angle_unit_t::radian:
            std::cout << "RAD";
            break;
        case angle_unit_t::grade:
            std::cout << "GRD";
            break;
        case angle_unit_t::degree:
            std::cout << "DEG";
            break;
        default:
            std::cout << "?";
            break;
        }
        std::cout << std::endl;

    }
    else
    {
        std::cout << "OFF" << std::endl;
    }
    std::cout << "\n"
        << "R0: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R0)
        << " | R1: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R1)
        << " | R2: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R2)
        << " | R3: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R3)
        << "\n"
        << "R4: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R4)
        << " | R5: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R5)
        << " | R6: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R6)
        << "\n"
        << "R7: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R7)
        << " | R8: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R8)
        << " | R9: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_R9)
        << " | RA: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RA)
        << "\n"
        << "RB: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RB)
        << " | RC: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RC)
        << " | RD: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RD)
        << " | RE: " << m_runner->get_reg_mem_str(mk61emu_reg_mem_t::mk61emu_RE)
        << "\n\n"
        << " T: '" << m_runner->get_reg_stack_str(mk61emu_reg_stack_t::RT)
        << "'\tCounter: " << m_runner->get_prog_counter_str()
        << "\tRunning: " << (m_runner->is_emu_running() ? "yes" : "no")
        << "\n"
        << " Z: '" << m_runner->get_reg_stack_str(mk61emu_reg_stack_t::RZ) << "'\n"
        << " Y: '" << m_runner->get_reg_stack_str(mk61emu_reg_stack_t::RY) << "'\n"
        << " X: '" << m_runner->get_reg_stack_str(mk61emu_reg_stack_t::RX) << "'\n"
        << "X1: '" << m_runner->get_reg_stack_str(mk61emu_reg_stack_t::RX1) << "'\n"
        << "\n"
        << "DISPLAY: '" << m_runner->get_reg_stack_str(mk61emu_reg_stack_t::RX) << "'"
        << std::endl;
}

mk_parse_result mk61_commander::parse_input(const std::string& cmd)
{
    mk_parse_result result;
    result.parsed = true;
    result.cmd_kind = mk_cmd_kind_t::cmd_empty;
    if (cmd.size() == 0)
        return result;
    result.cmd_kind = mk_cmd_kind_t::cmd_keys;
    std::string cmd_up;
    for (auto& c : cmd)
        cmd_up += (char)toupper(c);
    //std::transform(cmd_up.begin(), cmd_up.end(), cmd_up.begin(), ::toupper);
    // Keys
    if (cmd_up.size() == 1)
    {
        char c = cmd_up.back();
        if (static_cast<int>(c) > 47 && static_cast<int>(c) < 58)
            m_runner->do_key_press(static_cast<int>(c) - 46, 1); // Digits 0..9
        else if (c > 95 && c < 106) // Characters `abcdefghi
            m_runner->do_key_press(static_cast<uint8_t>(c) - 94, 1);
        else if (c == 'F')
            m_runner->do_key_press(11, 9);
        else if (c == 'K')
            m_runner->do_key_press(10, 9);
        else if (c == '*')
            m_runner->do_key_press(4, 8);
        else if (c == '+')
            m_runner->do_key_press(2, 8);
        else if (c == '-')
            m_runner->do_key_press(3, 8);
        else if (c == '/')
            m_runner->do_key_press(5, 8);
        else if (c == ',' || c == '.' || c == 'A' || c == 'a')
            m_runner->do_key_press(7, 8);
    }
    else if (cmd_up == "STPL")
        m_runner->do_key_press(7, 9);
    else if (cmd_up == "STPR")
        m_runner->do_key_press(9, 9);
    else if (cmd_up == "RTN")
        m_runner->do_key_press(4, 9);
    else if (cmd_up == "RS")
        m_runner->do_key_press(2, 9);
    else if (cmd_up == "STO")
        m_runner->do_key_press(6, 9);
    else if (cmd_up == "RCL")
        m_runner->do_key_press(8, 9);
    else if (cmd_up == "GTO")
        m_runner->do_key_press(3, 9);
    else if (cmd_up == "GSB")
        m_runner->do_key_press(5, 9);
    else if (cmd_up == "XY")
        m_runner->do_key_press(6, 8);
    else if (cmd_up == "SGN") // /-/
        m_runner->do_key_press(8, 8);
    else if (cmd_up == "EXP")
        m_runner->do_key_press(9, 8);
    else if (cmd_up == "CX")
        m_runner->do_key_press(10, 8);
    else if (cmd_up == "ENT" || cmd == "\n")
        m_runner->do_key_press(11, 8);
    // Modes
    else if (cmd_up == "PRG")
    {
        m_runner->do_key_press(11, 9); // F
        m_runner->do_key_press(9, 8);  // EXP key
    }
    else if (cmd_up == "AUT")
    {
        m_runner->do_key_press(11, 9); // F
        m_runner->do_key_press(8, 8);  // /-/ key
    }
    else if (cmd_up == "RAD")
        m_runner->set_angle_unit(angle_unit_t::radian);
    else if (cmd_up == "GRAD")
        m_runner->set_angle_unit(angle_unit_t::grade);
    else if (cmd_up == "DEG")
        m_runner->set_angle_unit(angle_unit_t::degree);
    // Commands
    else if (cmd_up == "SAVE")
        result.cmd_kind = mk_cmd_kind_t::cmd_save;
    else if (cmd_up == "LOAD")
        result.cmd_kind = mk_cmd_kind_t::cmd_load;
    else if (cmd_up == "QUIT" || cmd_up == "EXIT")
        result.cmd_kind = mk_cmd_kind_t::cmd_quit;
    else if (cmd_up == "OFF")
        result.cmd_kind = mk_cmd_kind_t::cmd_off;
    else if (cmd_up == "ON")
        result.cmd_kind = mk_cmd_kind_t::cmd_on;
    else if (cmd_up == "HELP")
        result.cmd_kind = mk_cmd_kind_t::cmd_help;
    else if (cmd_up == "STATE")
        result.cmd_kind = mk_cmd_kind_t::cmd_output_state;
    else
    {
        result.parsed = false;
        result.cmd_kind = mk_cmd_kind_t::cmd_unknown;
    }
    return result;
}
