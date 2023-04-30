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
* mk_key_coord
*/
mk_key_coord::mk_key_coord(uint8_t key1, uint8_t key2)
    : m_key1(key1), m_key2(key2)
{}

bool mk_key_coord::operator ==(const mk_key_coord& rhs) const
{
    return (m_key1 == rhs.m_key1) && (m_key2 == rhs.m_key2);
}

std::string mk_key_coord::to_string() const
{
    std::stringstream ss;
    ss << "{" << static_cast<int>(m_key1) << "," << static_cast<int>(m_key2) << "}";
    return ss.str();
}


/*
* mk_instruction_keys
*/
std::string mk_instruction_keys::keys_to_string() const
{
    std::stringstream ss;
    for (const auto& key : m_keys)
    {
        ss << key.to_string();
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

mk_instruction_keys_sptr instruction_index::find(const std::string& mnemonics) const
{
    const std::string key = make_key(mnemonics);
    auto iter = m_index.find(key);
    if (iter != m_index.end())
        return iter->second;
    return mk_instruction_keys_sptr();
}

void instruction_index::add_instr(
    int32_t code,
    const std::string& mnemonics,
    const std::string& caption,
    std::vector<mk_key_coord> keys,
    mk_instruction::synonyms_t mnemonics_synonyms
)
{
    auto instr = mk_instruction(code, mnemonics, caption, mnemonics_synonyms);
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
                throw std::logic_error(std::string("Key sequence already exists: ") + instr_keys->keys_to_string());
        }
    }
}


void instruction_index::init()
{
    add_instr(0x00, "0", "digit 0", { {2, 1} });
    add_instr(0x01, "1", "digit 1", { {3, 1} });
    add_instr(0x02, "2", "digit 2", { {4, 1} });
    add_instr(0x03, "3", "digit 3", { {5, 1} });
    add_instr(0x04, "4", "digit 4", { {6, 1} });
    add_instr(0x05, "5", "digit 5", { {7, 1} });
    add_instr(0x06, "6", "digit 6", { {8, 1} });
    add_instr(0x07, "7", "digit 7", { {9, 1} });
    add_instr(0x08, "8", "digit 8", { {10, 1} });
    add_instr(0x09, "9", "digit 9", { {11, 1} });
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
    add_instr(0x15, "10^x", "power of ten", { {11, 9}, {2, 1} });
    add_instr(0x16, "EXP", "power of e", { {11, 9}, {3, 1} });
    add_instr(0x17, "LG", "decimal logarithm", { {11, 9}, {4, 1} });
    add_instr(0x18, "LN", "natural logarithm", { {11, 9}, {5, 1} });
    add_instr(0x19, "ASIN", "arc sine", { {11, 9}, {6, 1} }, { "ARCSIN" });
    add_instr(0x1A, "ACOS", "arc cosine", { {11, 9}, {7, 1} }, { "ARCCOS" });
    add_instr(0x1B, "ATAN", "arc tangent", { {11, 9}, {8, 1} }, { "ARCTG" });
    add_instr(0x1C, "SIN", "sine", { {11, 9}, {9, 1} });
    add_instr(0x1D, "COS", "cosine", { {11, 9}, {10, 1} });
    add_instr(0x1E, "TAN", "tangent", { {11, 9}, {11, 1} }, { "TG" });
    // 0x1F
    add_instr(0x20, "PI", "pi constant", { {11, 9}, {2, 8} });
    add_instr(0x21, "SQRT", "square root", { {11, 9}, {3, 8} });
    add_instr(0x22, "x^2", "square of X", { {11, 9}, {4, 8} }, { "SQR" });
    add_instr(0x23, "1/x", "inversion of X", { {11, 9}, {5, 8} }, { "INV" });
    add_instr(0x24, "X^Y", "power of X", { {11, 9}, {6, 8} });
    add_instr(0x25, "R", "Roll down stack", { {11, 9}, {7, 8} });
    add_instr(0x26, "M-D", "HM to degrees", { {10, 9}, {8, 1} });
    // Skip 0x27..29
    add_instr(0x2A, "MS-D", "MS to degree", { {10, 9}, {5, 1} });
    // Skip 0x2B..2F
    add_instr(0x30, "D-MS", "degrees to MS", { {10, 9}, {6, 8} });
    add_instr(0x31, "ABS", "absolute value", { {10, 9}, {6, 1} }, { "|x|" });
    add_instr(0x32, "SGN", "sign of X", { {10, 9}, {7, 1} });
    add_instr(0x33, "D-M", "degrees to M", { {10, 9}, {2, 8} });
    add_instr(0x34, "INT", "integer part", { {10, 9}, {9, 1} }, { "[x]" });
    add_instr(0x35, "FRAC", "fractional part", { {10, 9}, {10, 1} }, { "{x}" });
    add_instr(0x36, "MAX", "max of X and Y", { {10, 9}, {11, 1} });
    add_instr(0x37, "AND", "logical AND", { {10, 9}, {7, 8} });
    add_instr(0x38, "OR", "logical OR", { {10, 9}, {8, 8} });
    add_instr(0x39, "XOR", "logical XOR", { {10, 9}, {9, 8} });
    add_instr(0x3A, "NOT", "logical NOT", { {10, 9}, {10, 8} });
    add_instr(0x3B, "RND", "random number", { {10, 9}, {11, 8} });
    // Skip 0x3C..3F
    add_instr(0x40, "M0", "store RX to memory register R0", { {6, 9}, {2, 1} }, { "MS0", "STO0" });
    add_instr(0x41, "M1", "store RX to memory register R1", { {6, 9}, {3, 1} }, { "MS1", "STO1" });
    add_instr(0x42, "M2", "store RX to memory register R2", { {6, 9}, {4, 1} }, { "MS2", "STO2" });
    add_instr(0x43, "M3", "store RX to memory register R3", { {6, 9}, {5, 1} }, { "MS3", "STO3" });
    add_instr(0x44, "M4", "store RX to memory register R4", { {6, 9}, {6, 1} }, { "MS4", "STO4" });
    add_instr(0x45, "M5", "store RX to memory register R5", { {6, 9}, {7, 1} }, { "MS5", "STO5" });
    add_instr(0x46, "M6", "store RX to memory register R6", { {6, 9}, {8, 1} }, { "MS6", "STO6" });
    add_instr(0x47, "M7", "store RX to memory register R7", { {6, 9}, {9, 1} }, { "MS7", "STO7" });
    add_instr(0x48, "M8", "store RX to memory register R8", { {6, 9}, {10, 1} }, { "MS8", "STO8" });
    add_instr(0x49, "M9", "store RX to memory register R9", { {6, 9}, {11, 1} }, { "MS9", "STO9" });
    add_instr(0x4A, "MA", "store RX to memory register RA", { {6, 9}, {7, 8} }, { "MSA", "STOA" });
    add_instr(0x4B, "MB", "store RX to memory register RB", { {6, 9}, {8, 8} }, { "MSB", "STOB" });
    add_instr(0x4C, "MC", "store RX to memory register RC", { {6, 9}, {9, 8} }, { "MSC", "STOC" });
    add_instr(0x4D, "MD", "store RX to memory register RD", { {6, 9}, {10, 8} }, { "MSD", "STOD" });
    add_instr(0x4E, "ME", "store RX to memory register RE", { {6, 9}, {11, 8} }, { "MSE", "STOE" });
    // Skip 0x4F
    add_instr(0x50, "R/S", "run/stop", { {2, 9} }, { "RS" });
    add_instr(0x51, "GTO", "go to instruction", { {3, 9} }, { "GOTO" });
    add_instr(0x52, "RTN", "return from subroutine", { {4, 9} }, { "RET", "RETURN" });
    add_instr(0x53, "GSB", "go to subroutine", { {5, 9} }, { "GOSUB", "CALL" });
    add_instr(0x54, "NOP", "no operation", { {10, 9}, {2, 1} }, { "KNOP" });
    // Skip 0x55..56
    add_instr(0x57, "x!=0", "check RX not equal to 0", { {11, 9}, {2, 9} }, { "x<>0", "xNE0"});
    add_instr(0x58, "L2", "loop on R2", { {11, 9}, {3, 9} });
    add_instr(0x59, "x>=0", "check RX greater or equal to 0", { {11, 9}, {4, 9} }, { "xGE0" });
    add_instr(0x5A, "L3", "loop on R3", { {11, 9}, {5, 9} });
    add_instr(0x58, "L1", "loop on R1", { {11, 9}, {6, 9} });
    add_instr(0x5C, "x<0", "check RX less than 0", { {11, 9}, {9, 9} }, { "xLT0" });
    add_instr(0x5D, "L0", "loop on R0", { {11, 9}, {8, 9} });
    add_instr(0x5E, "x=0", "check RX equal to 0", { {11, 9}, {7, 9} }, { "xEQ0" });
    // Skip 0x5F
    add_instr(0x60, "MR0", "recall memory register R0 to RX", { {8, 9}, {2, 1} }, { "RCL0" });
    add_instr(0x61, "MR1", "recall memory register R1 to RX", { {8, 9}, {3, 1} }, { "RCL1" });
    add_instr(0x62, "MR2", "recall memory register R2 to RX", { {8, 9}, {4, 1} }, { "RCL2" });
    add_instr(0x63, "MR3", "recall memory register R3 to RX", { {8, 9}, {5, 1} }, { "RCL3" });
    add_instr(0x64, "MR4", "recall memory register R4 to RX", { {8, 9}, {6, 1} }, { "RCL4" });
    add_instr(0x65, "MR5", "recall memory register R5 to RX", { {8, 9}, {7, 1} }, { "RCL5" });
    add_instr(0x66, "MR6", "recall memory register R6 to RX", { {8, 9}, {8, 1} }, { "RCL6" });
    add_instr(0x67, "MR7", "recall memory register R7 to RX", { {8, 9}, {9, 1} }, { "RCL7" });
    add_instr(0x68, "MR8", "recall memory register R8 to RX", { {8, 9}, {10, 1} }, { "RCL8" });
    add_instr(0x69, "MR9", "recall memory register R9 to RX", { {8, 9}, {11, 1} }, { "RCL9" });
    add_instr(0x6A, "MRA", "recall memory register RA to RX", { {8, 9}, {7, 8} }, { "RCLA" });
    add_instr(0x6B, "MRB", "recall memory register RB to RX", { {8, 9}, {8, 8} }, { "RCLB" });
    add_instr(0x6C, "MRC", "recall memory register RC to RX", { {8, 9}, {9, 8} }, { "RCLC" });
    add_instr(0x6D, "MRD", "recall memory register RD to RX", { {8, 9}, {10, 8} }, { "RCLD" });
    add_instr(0x6E, "MRE", "recall memory register RE to RX", { {8, 9}, {11, 8} }, { "RCLE" });
    // Skip 0x6F
    add_instr(0x70, "Kx!=00", "check x!=0, indirect jump by R0", { {10, 9}, {2, 9}, {2, 1} });
    add_instr(0x71, "Kx!=01", "check x!=0, indirect jump by R1", { {10, 9}, {2, 9}, {3, 1} });
    add_instr(0x72, "Kx!=02", "check x!=0, indirect jump by R2", { {10, 9}, {2, 9}, {4, 1} });
    add_instr(0x73, "Kx!=03", "check x!=0, indirect jump by R3", { {10, 9}, {2, 9}, {5, 1} });
    add_instr(0x74, "Kx!=04", "check x!=0, indirect jump by R4", { {10, 9}, {2, 9}, {6, 1} });
    add_instr(0x75, "Kx!=05", "check x!=0, indirect jump by R5", { {10, 9}, {2, 9}, {7, 1} });
    add_instr(0x76, "Kx!=06", "check x!=0, indirect jump by R6", { {10, 9}, {2, 9}, {8, 1} });
    add_instr(0x77, "Kx!=07", "check x!=0, indirect jump by R7", { {10, 9}, {2, 9}, {9, 1} });
    add_instr(0x78, "Kx!=08", "check x!=0, indirect jump by R8", { {10, 9}, {2, 9}, {10, 1} });
    add_instr(0x79, "Kx!=09", "check x!=0, indirect jump by R9", { {10, 9}, {2, 9}, {11, 1} });
    add_instr(0x7A, "Kx!=0A", "check x!=0, indirect jump by RA", { {10, 9}, {2, 9}, {7, 8} });
    add_instr(0x7B, "Kx!=0B", "check x!=0, indirect jump by RB", { {10, 9}, {2, 9}, {8, 8} });
    add_instr(0x7C, "Kx!=0C", "check x!=0, indirect jump by RC", { {10, 9}, {2, 9}, {9, 8} });
    add_instr(0x7D, "Kx!=0D", "check x!=0, indirect jump by RD", { {10, 9}, {2, 9}, {10, 8} });
    add_instr(0x7E, "Kx!=0E", "check x!=0, indirect jump by RE", { {10, 9}, {2, 9}, {11, 8} });
    // Skip 0x7F
    add_instr(0x80, "KGTO0", "indirect jump by R0", { {10, 9}, {3, 9}, {2, 1} }, { "KGOTO0" });
    add_instr(0x81, "KGTO1", "indirect jump by R1", { {10, 9}, {3, 9}, {3, 1} }, { "KGOTO1" });
    add_instr(0x82, "KGTO2", "indirect jump by R2", { {10, 9}, {3, 9}, {4, 1} }, { "KGOTO2" });
    add_instr(0x83, "KGTO3", "indirect jump by R3", { {10, 9}, {3, 9}, {5, 1} }, { "KGOTO3" });
    add_instr(0x84, "KGTO4", "indirect jump by R4", { {10, 9}, {3, 9}, {6, 1} }, { "KGOTO4" });
    add_instr(0x85, "KGTO5", "indirect jump by R5", { {10, 9}, {3, 9}, {7, 1} }, { "KGOTO5" });
    add_instr(0x86, "KGTO6", "indirect jump by R6", { {10, 9}, {3, 9}, {8, 1} }, { "KGOTO6" });
    add_instr(0x87, "KGTO7", "indirect jump by R7", { {10, 9}, {3, 9}, {9, 1} }, { "KGOTO7" });
    add_instr(0x88, "KGTO8", "indirect jump by R8", { {10, 9}, {3, 9}, {10, 1} }, { "KGOTO8" });
    add_instr(0x89, "KGTO9", "indirect jump by R9", { {10, 9}, {3, 9}, {11, 1} }, { "KGOTO9" });
    add_instr(0x8A, "KGTOA", "indirect jump by RA", { {10, 9}, {3, 9}, {7, 8} }, { "KGOTOA" });
    add_instr(0x8B, "KGTOB", "indirect jump by RB", { {10, 9}, {3, 9}, {8, 8} }, { "KGOTOB" });
    add_instr(0x8C, "KGTOC", "indirect jump by RC", { {10, 9}, {3, 9}, {9, 8} }, { "KGOTOC" });
    add_instr(0x8D, "KGTOD", "indirect jump by RD", { {10, 9}, {3, 9}, {10, 8} }, { "KGOTOD" });
    add_instr(0x8E, "KGTOE", "indirect jump by RE", { {10, 9}, {3, 9}, {11, 8} }, { "KGOTOE" });
    // Skip 0x8F
    add_instr(0x90, "Kx>=00", "check x>=0, indirect jump by R0", { {10, 9}, {4, 9}, {2, 1} });
    add_instr(0x91, "Kx>=01", "check x>=0, indirect jump by R1", { {10, 9}, {4, 9}, {3, 1} });
    add_instr(0x92, "Kx>=02", "check x>=0, indirect jump by R2", { {10, 9}, {4, 9}, {4, 1} });
    add_instr(0x93, "Kx>=03", "check x>=0, indirect jump by R3", { {10, 9}, {4, 9}, {5, 1} });
    add_instr(0x94, "Kx>=04", "check x>=0, indirect jump by R4", { {10, 9}, {4, 9}, {6, 1} });
    add_instr(0x95, "Kx>=05", "check x>=0, indirect jump by R5", { {10, 9}, {4, 9}, {7, 1} });
    add_instr(0x66, "Kx>=06", "check x>=0, indirect jump by R6", { {10, 9}, {4, 9}, {8, 1} });
    add_instr(0x97, "Kx>=07", "check x>=0, indirect jump by R7", { {10, 9}, {4, 9}, {9, 1} });
    add_instr(0x98, "Kx>=08", "check x>=0, indirect jump by R8", { {10, 9}, {4, 9}, {10, 1} });
    add_instr(0x99, "Kx>=09", "check x>=0, indirect jump by R9", { {10, 9}, {4, 9}, {11, 1} });
    add_instr(0x9A, "Kx>=0A", "check x>=0, indirect jump by RA", { {10, 9}, {4, 9}, {7, 8} });
    add_instr(0x9B, "Kx>=0B", "check x>=0, indirect jump by RB", { {10, 9}, {4, 9}, {8, 8} });
    add_instr(0x9C, "Kx>=0C", "check x>=0, indirect jump by RC", { {10, 9}, {4, 9}, {9, 8} });
    add_instr(0x9D, "Kx>=0D", "check x>=0, indirect jump by RD", { {10, 9}, {4, 9}, {10, 8} });
    add_instr(0x9E, "Kx>=0E", "check x>=0, indirect jump by RE", { {10, 9}, {4, 9}, {11, 8} });
    // Skip 0x9F
    add_instr(0xA0, "KGSB0", "indirect go to subroutine by R0", { {10, 9}, {5, 9}, {2, 1} }, { "KGOSUB0" });
    add_instr(0xA1, "KGSB1", "indirect go to subroutine by R1", { {10, 9}, {5, 9}, {3, 1} }, { "KGOSUB1" });
    add_instr(0xA2, "KGSB2", "indirect go to subroutine by R2", { {10, 9}, {5, 9}, {4, 1} }, { "KGOSUB2" });
    add_instr(0xA3, "KGSB3", "indirect go to subroutine by R3", { {10, 9}, {5, 9}, {5, 1} }, { "KGOSUB3" });
    add_instr(0xA4, "KGSB4", "indirect go to subroutine by R4", { {10, 9}, {5, 9}, {6, 1} }, { "KGOSUB4" });
    add_instr(0xA5, "KGSB5", "indirect go to subroutine by R5", { {10, 9}, {5, 9}, {7, 1} }, { "KGOSUB5" });
    add_instr(0xA6, "KGSB6", "indirect go to subroutine by R6", { {10, 9}, {5, 9}, {8, 1} }, { "KGOSUB6" });
    add_instr(0xA7, "KGSB7", "indirect go to subroutine by R7", { {10, 9}, {5, 9}, {9, 1} }, { "KGOSUB7" });
    add_instr(0xA8, "KGSB8", "indirect go to subroutine by R8", { {10, 9}, {5, 9}, {10, 1} }, { "KGOSUB8" });
    add_instr(0xA9, "KGSB9", "indirect go to subroutine by R9", { {10, 9}, {5, 9}, {11, 1} }, { "KGOSUB9" });
    add_instr(0xAA, "KGSBA", "indirect go to subroutine by RA", { {10, 9}, {5, 9}, {7, 8} }, { "KGOSUBA" });
    add_instr(0xAB, "KGSBB", "indirect go to subroutine by RB", { {10, 9}, {5, 9}, {8, 8} }, { "KGOSUBB" });
    add_instr(0xAC, "KGSBC", "indirect go to subroutine by RC", { {10, 9}, {5, 9}, {9, 8} }, { "KGOSUBC" });
    add_instr(0xAD, "KGSBD", "indirect go to subroutine by RD", { {10, 9}, {5, 9}, {10, 8} }, { "KGOSUBD" });
    add_instr(0xAE, "KGSBE", "indirect go to subroutine by RE", { {10, 9}, {5, 9}, {11, 8} }, { "KGOSUBE" });
    // Skip 0xAF
    add_instr(0xB0, "KM0", "indirect store to memory by R0", { {10, 9}, {6, 9}, {2, 1} }, { "KMS0", "KSTO0"});
    add_instr(0xB1, "KM1", "indirect store to memory by R1", { {10, 9}, {6, 9}, {3, 1} }, { "KMS1", "KSTO1" });
    add_instr(0xB2, "KM2", "indirect store to memory by R2", { {10, 9}, {6, 9}, {4, 1} }, { "KMS2", "KSTO2" });
    add_instr(0xB3, "KM3", "indirect store to memory by R3", { {10, 9}, {6, 9}, {5, 1} }, { "KMS3", "KSTO3" });
    add_instr(0xB4, "KM4", "indirect store to memory by R4", { {10, 9}, {6, 9}, {6, 1} }, { "KMS4", "KSTO4" });
    add_instr(0xB5, "KM5", "indirect store to memory by R5", { {10, 9}, {6, 9}, {7, 1} }, { "KMS5", "KSTO5" });
    add_instr(0xB6, "KM6", "indirect store to memory by R6", { {10, 9}, {6, 9}, {8, 1} }, { "KMS6", "KSTO6" });
    add_instr(0xB7, "KM7", "indirect store to memory by R7", { {10, 9}, {6, 9}, {9, 1} }, { "KMS7", "KSTO7" });
    add_instr(0xB8, "KM8", "indirect store to memory by R8", { {10, 9}, {6, 9}, {10, 1} }, { "KMS8", "KSTO8" });
    add_instr(0xB9, "KM9", "indirect store to memory by R9", { {10, 9}, {6, 9}, {11, 1} }, { "KMS9", "KSTO9" });
    add_instr(0xBA, "KMA", "indirect store to memory by RA", { {10, 9}, {6, 9}, {7, 8} }, { "KMSA", "KSTOA" });
    add_instr(0xBB, "KMB", "indirect store to memory by RB", { {10, 9}, {6, 9}, {8, 8} }, { "KMSB", "KSTOB" });
    add_instr(0xBC, "KMC", "indirect store to memory by RC", { {10, 9}, {6, 9}, {9, 8} }, { "KMSC", "KSTOC" });
    add_instr(0xBD, "KMD", "indirect store to memory by RD", { {10, 9}, {6, 9}, {10, 8} }, { "KMSD", "KSTOD" });
    add_instr(0xBE, "KME", "indirect store to memory by RE", { {10, 9}, {6, 9}, {11, 8} }, { "KMSE", "KSTOE" });
    // Skip 0xBF
    add_instr(0xC0, "Kx<00", "check x<0, indirect jump by R0", { {10, 9}, {9, 9}, {2, 1} });
    add_instr(0xC1, "Kx<01", "check x<0, indirect jump by R1", { {10, 9}, {9, 9}, {3, 1} });
    add_instr(0xC2, "Kx<02", "check x<0, indirect jump by R2", { {10, 9}, {9, 9}, {4, 1} });
    add_instr(0xC3, "Kx<03", "check x<0, indirect jump by R3", { {10, 9}, {9, 9}, {5, 1} });
    add_instr(0xC4, "Kx<04", "check x<0, indirect jump by R4", { {10, 9}, {9, 9}, {6, 1} });
    add_instr(0xC5, "Kx<05", "check x<0, indirect jump by R5", { {10, 9}, {9, 9}, {7, 1} });
    add_instr(0xC6, "Kx<06", "check x<0, indirect jump by R6", { {10, 9}, {9, 9}, {8, 1} });
    add_instr(0xC7, "Kx<07", "check x<0, indirect jump by R7", { {10, 9}, {9, 9}, {9, 1} });
    add_instr(0xC8, "Kx<08", "check x<0, indirect jump by R8", { {10, 9}, {9, 9}, {10, 1} });
    add_instr(0xC9, "Kx<09", "check x<0, indirect jump by R9", { {10, 9}, {9, 9}, {11, 1} });
    add_instr(0xCA, "Kx<0A", "check x<0, indirect jump by RA", { {10, 9}, {9, 9}, {7, 8} });
    add_instr(0xCB, "Kx<0B", "check x<0, indirect jump by RB", { {10, 9}, {9, 9}, {8, 8} });
    add_instr(0xCC, "Kx<0C", "check x<0, indirect jump by RC", { {10, 9}, {9, 9}, {9, 8} });
    add_instr(0xCD, "Kx<0D", "check x<0, indirect jump by RD", { {10, 9}, {9, 9}, {10, 8} });
    add_instr(0xCE, "Kx<0E", "check x<0, indirect jump by RE", { {10, 9}, {9, 9}, {11, 8} });
    // Skip 0xCF
    add_instr(0xD0, "KMR0", "indirect recall from memory by R0", { {10, 9}, {8, 9}, {2, 1} }, { "KRCL0" });
    add_instr(0xD1, "KMR1", "indirect recall from memory by R1", { {10, 9}, {8, 9}, {3, 1} }, { "KRCL1" });
    add_instr(0xD2, "KMR2", "indirect recall from memory by R2", { {10, 9}, {8, 9}, {4, 1} }, { "KRCL2" });
    add_instr(0xD3, "KMR3", "indirect recall from memory by R3", { {10, 9}, {8, 9}, {5, 1} }, { "KRCL3" });
    add_instr(0xD4, "KMR4", "indirect recall from memory by R4", { {10, 9}, {8, 9}, {6, 1} }, { "KRCL4" });
    add_instr(0xD5, "KMR5", "indirect recall from memory by R5", { {10, 9}, {8, 9}, {7, 1} }, { "KRCL5" });
    add_instr(0xD6, "KMR6", "indirect recall from memory by R6", { {10, 9}, {8, 9}, {8, 1} }, { "KRCL6" });
    add_instr(0xD7, "KMR7", "indirect recall from memory by R7", { {10, 9}, {8, 9}, {9, 1} }, { "KRCL7" });
    add_instr(0xD8, "KMR8", "indirect recall from memory by R8", { {10, 9}, {8, 9}, {10, 1} }, { "KRCL8" });
    add_instr(0xD9, "KMR9", "indirect recall from memory by R9", { {10, 9}, {8, 9}, {11, 1} }, { "KRCL9" });
    add_instr(0xDA, "KMRA", "indirect recall from memory by RA", { {10, 9}, {8, 9}, {7, 8} }, { "KRCLA" });
    add_instr(0xDB, "KMRB", "indirect recall from memory by RB", { {10, 9}, {8, 9}, {8, 8} }, { "KRCLB" });
    add_instr(0xDC, "KMRC", "indirect recall from memory by RC", { {10, 9}, {8, 9}, {9, 8} }, { "KRCLC" });
    add_instr(0xDD, "KMRD", "indirect recall from memory by RD", { {10, 9}, {8, 9}, {10, 8} }, { "KRCLD" });
    add_instr(0xDE, "KMRE", "indirect recall from memory by RE", { {10, 9}, {8, 9}, {11, 8} }, { "KRCLE" });
    // Skip 0xDF
    add_instr(0xE0, "Kx=00", "check x=0, indirect jump by R0", { {10, 9}, {7, 9}, {2, 1} });
    add_instr(0xE1, "Kx=01", "check x=0, indirect jump by R1", { {10, 9}, {7, 9}, {3, 1} });
    add_instr(0xE2, "Kx=02", "check x=0, indirect jump by R2", { {10, 9}, {7, 9}, {4, 1} });
    add_instr(0xE3, "Kx=03", "check x=0, indirect jump by R3", { {10, 9}, {7, 9}, {5, 1} });
    add_instr(0xE4, "Kx=04", "check x=0, indirect jump by R4", { {10, 9}, {7, 9}, {6, 1} });
    add_instr(0xE5, "Kx=05", "check x=0, indirect jump by R5", { {10, 9}, {7, 9}, {7, 1} });
    add_instr(0xE6, "Kx=06", "check x=0, indirect jump by R6", { {10, 9}, {7, 9}, {8, 1} });
    add_instr(0xE7, "Kx=07", "check x=0, indirect jump by R7", { {10, 9}, {7, 9}, {9, 1} });
    add_instr(0xE8, "Kx=08", "check x=0, indirect jump by R8", { {10, 9}, {7, 9}, {10, 1} });
    add_instr(0xE9, "Kx=09", "check x=0, indirect jump by R9", { {10, 9}, {7, 9}, {11, 1} });
    add_instr(0xEA, "Kx=0A", "check x=0, indirect jump by RA", { {10, 9}, {7, 9}, {7, 8} });
    add_instr(0xEB, "Kx=0B", "check x=0, indirect jump by RB", { {10, 9}, {7, 9}, {8, 8} });
    add_instr(0xEC, "Kx=0C", "check x=0, indirect jump by RC", { {10, 9}, {7, 9}, {9, 8} });
    add_instr(0xED, "Kx=0D", "check x=0, indirect jump by RD", { {10, 9}, {7, 9}, {10, 8} });
    add_instr(0xEE, "Kx=0E", "check x=0, indirect jump by RE", { {10, 9}, {7, 9}, {11, 8} });
    // Skip 0xEF..FF
    // Modes
    add_instr(mk_instruction::no_code, "AUT", "calculation mode", { {11, 9}, {8, 8} });
    add_instr(mk_instruction::no_code, "PRG", "programming mode", { {11, 9}, {9, 8} });
    add_instr(mk_instruction::no_code, "STEPL", "step left", { {7, 9} });
    add_instr(mk_instruction::no_code, "STEPR", "step right", { {9, 9} });
}


/*
* mk61_commander
*/
mk61_commander::mk61_commander()
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
    show_short_help();
    m_runner = std::make_unique<emu_runner>();
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
                case mk_cmd_kind_t::cmd_mode:
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
    system("clear");
#endif
}

void mk61_commander::output_display()
{
    std::cout << "RX: " << m_runner->get_reg_stack_str(mk61emu_reg_stack_t::RX) << std::endl;
}

void mk61_commander::show_help()
{
    clear_screen();
    show_short_help();
    for (const auto& instr : m_instructions.data())
    {
        std::cout << instr->instruction().mnemonics() << "\t" << instr->instruction().caption();
        if (instr->instruction().synonyms().size() > 0)
        {
            std::cout << ". Synonyms: ";
            for (const auto& synonym : instr->instruction().synonyms())
            {
                std::cout << synonym << " ";
            }
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

void mk61_commander::show_short_help()
{
    clear_screen();
    std::cout << "MK61 emulator v" << MK61EMU_VERSION_MAJOR << "." << MK61EMU_VERSION_MINOR << "\n"
        << "Commands:\n"
        << "    HELP to show instruction/keys list\n"
        << "    QUIT or EXIT to quit the program\n"
        << "    ON to switch calculator on\n"
        << "    OFF to switch calculator off\n"
        //<< "    SAVE <filename> to save calculator state (program, memory...) to file\n"
        //<< "    LOAD <filename> to restore calculator state from file\n"
        << "    STATE to show calculator state\n"
        << "Setting the angular mode:\n"
        << "    DEG sets degree mode, which uses decimal degrees rather than hexagesimal degrees (degrees, minutes, seconds)\n"
        << "    RAD sets radian mode\n"
        << "    GRAD sets gradient mode\n"
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
    result.parsed = false;
    result.cmd_kind = mk_cmd_kind_t::cmd_unknown;
    if (cmd.size() == 0)
    {
        result.parsed = true;
        result.cmd_kind = mk_cmd_kind_t::cmd_empty;
        return result;
    }
    mk_instruction_keys_sptr instr = m_instructions.find(cmd);
    if (instr)
    {
        result.parsed = true;
        result.cmd_kind = mk_cmd_kind_t::cmd_keys;
        for (const auto& key : instr->keys())
        {
            m_runner->do_key_press(key.key1(), key.key2());
        }
    }
    else
    {
        std::string cmd_up = strutils::to_upper(cmd);
        if (cmd_up == "RAD")
        {
            result.cmd_kind = mk_cmd_kind_t::cmd_mode;
            m_runner->set_angle_unit(angle_unit_t::radian);
        }
        else if (cmd_up == "GRAD")
        {
            result.cmd_kind = mk_cmd_kind_t::cmd_mode;
            m_runner->set_angle_unit(angle_unit_t::grade);
        }
        else if (cmd_up == "DEG")
        {
            result.cmd_kind = mk_cmd_kind_t::cmd_mode;
            m_runner->set_angle_unit(angle_unit_t::degree);
        }
        //else if (cmd_up == "SAVE")
        //    result.cmd_kind = mk_cmd_kind_t::cmd_save;
        //else if (cmd_up == "LOAD")
            //result.cmd_kind = mk_cmd_kind_t::cmd_load;
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
        if (result.cmd_kind != mk_cmd_kind_t::cmd_unknown)
            result.parsed = true;
    }
    return result;
}
