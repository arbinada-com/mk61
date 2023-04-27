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
* mk_instruction_keys
*/
std::string mk_instruction_keys::keys_to_string() const
{
    std::stringstream ss;
    for (const auto& key : m_keys)
    {
        ss << "{" << key.key1() << "," << key.key2() << "{";
    }
    return ss.str();
}


/*
* instruction_index
*/
std::string instruction_index::make_key(const std::string& mnemonics)
{
    return strutils::to_upper(mnemonics);
}

void instruction_index::add_instr(
    int32_t code,
    const std::string& mnemonics,
    const std::string& caption,
    std::vector<mk_key_coord> keys,
    std::vector<std::string> mnemonics_synonyms
)
{
    auto instr = mk_instruction(code, mnemonics, caption);
    auto instr_keys = std::make_shared<mk_instruction_keys>(instr, keys);
    check_mnemonics_not_exists(instr.mnemonics());
    check_keys_not_exist(keys);
    m_data.push_back(instr_keys);
    m_index.insert(std::make_pair(make_key(instr.mnemonics()), instr_keys));
    for (const auto& synonym : mnemonics_synonyms)
    {
        check_mnemonics_not_exists(synonym);
        m_index.insert(std::make_pair(make_key(synonym), instr_keys));
    }
}

void instruction_index::check_mnemonics_not_exists(const std::string& mnemonics)
{
    std::string key = make_key(mnemonics);
    if (m_index.find(key) != m_index.cend())
        throw std::logic_error("Mnemonics already exists: " + key);
}

void instruction_index::check_keys_not_exist(std::vector<mk_key_coord> keys)
{
    for (const auto& instr_keys : m_data)
    {
        if (instr_keys->keys().size() == keys.size())
        {
            int match_count = 0;
            for (size_t i = 0; i != keys.size(); ++i)
            {
                if (instr_keys->keys()[i] == keys[i])
                    ++match_count;
                else
                    break;
            }
            if (match_count == keys.size())
                throw std::logic_error("Key sequence already exists: " + instr_keys->keys_to_string());
        }
    }
}


void instruction_index::init()
{
    add_instr(0x00, "0", "digit 0", { {0, 1} });
    add_instr(0x01, "1", "digit 1", { {1, 1} });
    add_instr(0x02, "2", "digit 2", { {2, 1} });
    add_instr(0x03, "3", "digit 3", { {3, 1} });
    add_instr(0x04, "4", "digit 4", { {4, 1} });
    add_instr(0x05, "5", "digit 5", { {5, 1} });
    add_instr(0x06, "6", "digit 6", { {6, 1} });
    add_instr(0x07, "7", "digit 7", { {7, 1} });
    add_instr(0x08, "8", "digit 8", { {8, 1} });
    add_instr(0x09, "9", "digit 9", { {9, 1} });
    add_instr(0x0A, ",", "decimal point", { {7, 8} }, {"."});
    add_instr(0x0B, "+/-", "changes the sign of a number", { {8, 8} }, {"/-/"});
    add_instr(0x0C, "E", "enter powers of ten", { {9, 8} }, { "EE" });
    add_instr(0x0D, "Cx", "clear display (RX)", { {10, 8} });
    add_instr(0x0E, "ENT", "enter", { {11, 8} });
    add_instr(0x0F, "LASTx", "last value of RX", { {11, 9}, {11, 8} }, { "FBx", "FANS"});
    add_instr(0x10, "+", "addition", { {2, 8} });
    add_instr(0x11, "-", "substraction", { {3, 8} });
    add_instr(0x12, "*", "multiplication", { {4, 8} }, {"x"});
    add_instr(0x13, "/", "division", { {5, 8} }, { ":" });
    add_instr(0x14, "<->", "swap RX with RY", { {6, 8} }, { "XY" });
    add_instr(0x15, "10^x", "power of ten", { {11, 9}, {0, 1} });
    add_instr(0x16, "EXP", "power of e", { {11, 9}, {1, 1} });
    add_instr(0x17, "LG", "decimal logarithm", { {11, 9}, {2, 1} });
    add_instr(0x18, "LN", "natural logarithm", { {11, 9}, {3, 1} });
    add_instr(0x19, "ASIN", "arc sine", { {11, 9}, {4, 1} }, { "ARCSIN" });
    add_instr(0x1A, "ACOS", "arc cosine", { {11, 9}, {5, 1} }, { "ARCCOS" });
    add_instr(0x1B, "ATAN", "arc tangent", { {11, 9}, {6, 1} }, { "ARCTG" });
    add_instr(0x1C, "SIN", "sine", { {11, 9}, {7, 1} });
    add_instr(0x1D, "COS", "cosine", { {11, 9}, {8, 1} });
    add_instr(0x1E, "TAN", "tangent", { {11, 9}, {9, 1} }, { "TG" });
    // 0x1F
    add_instr(0x20, "PI", "pi constant", { {11, 9}, {2, 8} });
    add_instr(0x21, "SQRT", "square root", { {11, 9}, {3, 8} });
    add_instr(0x22, "x^2", "square of X", { {11, 9}, {4, 8} });
    add_instr(0x23, "1/x", "inversion of X", { {11, 9}, {5, 8} }, { "INV" });
    add_instr(0x24, "X^Y", "power of X", { {11, 9}, {6, 8} });
    add_instr(0x25, "R", "Roll down stack", { {11, 9}, {7, 8} });
    add_instr(0x26, "M-D", "HM to degrees", { {10, 9}, {6, 1} });
    // Skip 0x27..29
    add_instr(0x2A, "MS-D", "MS to degree", { {10, 9}, {3, 1} });
    // Skip 0x2B..2F
    add_instr(0x30, "D-MS", "degrees to MS", { {10, 9}, {6, 8} });
    add_instr(0x31, "ABS", "absolute value", { {10, 9}, {4, 1} }, { "|x|" });
    add_instr(0x32, "SGN", "sign of X", { {10, 9}, {5, 1} });
    add_instr(0x33, "D-M", "degrees to M", { {10, 9}, {2, 8} });
    add_instr(0x34, "INT", "integer part", { {10, 9}, {7, 1} }, { "[x]" });
    add_instr(0x35, "FRAC", "fractional part", { {10, 9}, {8, 1} }, { "{x}" });
    add_instr(0x36, "MAX", "max of X and Y", { {10, 9}, {9, 1} });
    add_instr(0x37, "AND", "logical AND", { {10, 9}, {7, 8} });
    add_instr(0x38, "OR", "logical OR", { {10, 9}, {8, 8} });
    add_instr(0x39, "XOR", "logical XOR", { {10, 9}, {9, 8} });
    add_instr(0x3A, "NOT", "logical NOT", { {10, 9}, {10, 8} });
    add_instr(0x3B, "RND", "random number", { {10, 9}, {11, 8} });
    // Skip 0x3C..3F
    add_instr(0x40, "M0", "store RX to memory register R0", { {6, 9}, {0, 1} }, { "MS0", "STO0" });
    add_instr(0x41, "M1", "store RX to memory register R1", { {6, 9}, {1, 1} }, { "MS1", "STO1" });
    add_instr(0x42, "M2", "store RX to memory register R2", { {6, 9}, {2, 1} }, { "MS2", "STO2" });
    add_instr(0x43, "M3", "store RX to memory register R3", { {6, 9}, {3, 1} }, { "MS3", "STO3" });
    add_instr(0x44, "M4", "store RX to memory register R4", { {6, 9}, {4, 1} }, { "MS4", "STO4" });
    add_instr(0x45, "M5", "store RX to memory register R5", { {6, 9}, {5, 1} }, { "MS5", "STO5" });
    add_instr(0x46, "M6", "store RX to memory register R6", { {6, 9}, {6, 1} }, { "MS6", "STO6" });
    add_instr(0x47, "M7", "store RX to memory register R7", { {6, 9}, {7, 1} }, { "MS7", "STO7" });
    add_instr(0x48, "M8", "store RX to memory register R8", { {6, 9}, {8, 1} }, { "MS8", "STO8" });
    add_instr(0x49, "M9", "store RX to memory register R9", { {6, 9}, {9, 1} }, { "MS9", "STO9" });
    add_instr(0x4A, "MA", "store RX to memory register RA", { {6, 9}, {7, 8} }, { "MSA", "STOA" });
    add_instr(0x4B, "MB", "store RX to memory register RB", { {6, 9}, {8, 8} }, { "MSB", "STOB" });
    add_instr(0x4C, "MC", "store RX to memory register RC", { {6, 9}, {9, 8} }, { "MSC", "STOC" });
    add_instr(0x4D, "MD", "store RX to memory register RD", { {6, 9}, {10, 8} }, { "MSD", "STOD" });
    add_instr(0x4E, "ME", "store RX to memory register RE", { {6, 9}, {11, 8} }, { "MSE", "STOE" });
    // Skip 0x4F
    add_instr(0x50, "R/S", "run/stop", { {2, 9} }, { "RS" });
    add_instr(0x51, "GTO", "go to instruction", { {3, 9} }, { "GOTO" });
    add_instr(0x52, "RTN", "return from subroutine", { {4, 9} }, { "RETURN" });
    add_instr(0x53, "GSB", "go to subroutine", { {5, 9} }, { "GOSUB" });
    add_instr(0x54, "NOP", "no operation", { {10, 9}, {0, 1} });
    // Skip 0x55..56
    add_instr(0x57, "x!=0", "check RX not equal to 0", { {11, 9}, {2, 9} }, { "x<>0", "xNE0"});

    //
    add_instr(0x60, "MR0", "recall memory register R0 to RX", { {8, 9}, {0, 1} }, { "RCL0" });
    add_instr(0x61, "MR1", "recall memory register R1 to RX", { {8, 9}, {1, 1} }, { "RCL1" });
    add_instr(0x62, "MR2", "recall memory register R2 to RX", { {8, 9}, {2, 1} }, { "RCL2" });
    add_instr(0x63, "MR3", "recall memory register R3 to RX", { {8, 9}, {3, 1} }, { "RCL3" });
    add_instr(0x64, "MR4", "recall memory register R4 to RX", { {8, 9}, {4, 1} }, { "RCL4" });
    add_instr(0x65, "MR5", "recall memory register R5 to RX", { {8, 9}, {5, 1} }, { "RCL5" });
    add_instr(0x66, "MR6", "recall memory register R6 to RX", { {8, 9}, {6, 1} }, { "RCL6" });
    add_instr(0x67, "MR7", "recall memory register R7 to RX", { {8, 9}, {7, 1} }, { "RCL7" });
    add_instr(0x68, "MR8", "recall memory register R8 to RX", { {8, 9}, {8, 1} }, { "RCL8" });
    add_instr(0x69, "MR9", "recall memory register R9 to RX", { {8, 9}, {9, 1} }, { "RCL9" });
    add_instr(0x6A, "MRA", "recall memory register RA to RX", { {8, 9}, {7, 8} }, { "RCLA" });
    add_instr(0x6B, "MRB", "recall memory register RB to RX", { {8, 9}, {8, 8} }, { "RCLB" });
    add_instr(0x6C, "MRC", "recall memory register RC to RX", { {8, 9}, {9, 8} }, { "RCLC" });
    add_instr(0x6D, "MRD", "recall memory register RD to RX", { {8, 9}, {10, 8} }, { "RCLD" });
    add_instr(0x6E, "MRE", "recall memory register RE to RX", { {8, 9}, {11, 8} }, { "RCLE" });

    // Modes
    add_instr(mk_instruction::no_code, "AUT", "calculation mode", { {11, 9}, {8, 8} });
    add_instr(mk_instruction::no_code, "PRG", "programming mode", { {11, 9}, {9, 8} });
    // Setting the angular mode
    // DEG Sets degree mode, which uses decimal degrees rather than hexagesimal degrees (degrees, minutes, seconds)
    // RAD Sets radian mode
    // GRAD Sets gradient mode
}


/*
* mk61_commander
*/
mk61_commander::mk61_commander()
    : m_runner(std::make_unique<emu_runner>())
{
    m_instructions.init();
}

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
    std::string cmd_up = strutils::to_upper(cmd);
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
