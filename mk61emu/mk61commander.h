#ifndef MK61COMMANDER_H_INCLUDED
#define MK61COMMANDER_H_INCLUDED

#include <memory>
#include <iostream>
#include <string>
#include <mutex>
#include <vector>
#include <map>
#include "mk61emu.h"

using strings_t = std::vector<std::string>;

enum class mk_cmd_kind_t
{
    cmd_unknown,
    cmd_empty,
    cmd_quit,
    cmd_load,
    cmd_save,
    cmd_on,
    cmd_off,
    cmd_output_state,
    cmd_help,
    cmd_mode,
    cmd_keys
};

enum class mk_message_t
{
    msg_info,
    msg_warn,
    msg_error
};

struct mk_parse_result
{
    mk_cmd_kind_t cmd_kind = mk_cmd_kind_t::cmd_unknown;
    bool          parsed   = false;
};

class emu_runner
{
public:
    emu_runner();
    ~emu_runner();
    emu_runner(const emu_runner&) = delete;
    emu_runner& operator =(const emu_runner&) = delete;
public:
    void start();
    void terminate();
public:
    mk_result_t do_key_press(const uint8_t key1, const uint8_t key2);
    angle_unit_t get_angle_unit();
    engine_power_state_t get_power_state();
    std::string get_prog_counter_str();
    std::string get_reg_mem_str(const mk61emu_reg_mem_t reg);
    std::string get_reg_stack_str(const mk61emu_reg_stack_t reg);
    bool is_emu_running();
    void set_angle_unit(angle_unit_t value);
    void set_power_state(engine_power_state_t value);
private:
    void do_step_unsafe();
    void internal_run();
private:
    std::unique_ptr<std::thread> m_emu_thread;
    std::unique_ptr<mk61_emu> m_emu;
    std::mutex m_lock;
    std::atomic_bool m_sig_term = false;
    std::atomic_bool m_simulate_delay = true;
};

class mk_key_coord
{
public:
    mk_key_coord(uint8_t key1, uint8_t key2);
    mk_key_coord(const mk_key_coord&) = default;
    mk_key_coord& operator =(const mk_key_coord&) = default;
public:
    bool operator ==(const mk_key_coord& rhs) const;
public:
    uint8_t key1() const { return m_key1; }
    uint8_t key2() const { return m_key2; }
    std::string to_string() const;
private:
    uint8_t m_key1;
    uint8_t m_key2;
};

class mk_instruction_keys
{
public:
    mk_instruction_keys(mk_instruction instruction, std::vector<mk_key_coord> keys)
        : m_instruction(instruction), m_keys(keys)
    {}
public:
    const mk_instruction& instruction() const { return m_instruction; }
    const std::vector<mk_key_coord>& keys() const { return m_keys; }
    std::string keys_to_string() const;
private:
    mk_instruction m_instruction;
    std::vector<mk_key_coord> m_keys;
};


using mk_instruction_keys_sptr = std::shared_ptr<mk_instruction_keys>;

class instruction_index
{
public:
    typedef std::vector<mk_instruction_keys_sptr> data_t;
    typedef std::map<std::string, mk_instruction_keys_sptr> index_t;
public:
    void init();
public:
    mk_instruction_keys_sptr find(const std::string& mnemonics) const;
    static std::string make_key(const std::string& mnemonics);
    const data_t& data() const {
        return m_data;
    }
private:
    void add_instr(
        int32_t code,
        const std::string& mnemonics,
        const std::string& caption,
        std::vector<mk_key_coord> keys,
        mk_instruction::synonyms_t mnemonics_synonyms = {}
    );
    void check_mnemonics_not_exists(const std::string& mnemonics);
    void check_keys_not_exist(std::vector<mk_key_coord> keys);
private:
    data_t m_data;
    index_t m_index;
};


class mk61_commander
{
public:
    mk61_commander();
    mk61_commander(const mk61_commander&) = delete;
    mk61_commander& operator =(const mk61_commander&) = delete;
    mk61_commander(mk61_commander&&) = delete;
    mk61_commander& operator =(mk61_commander&&) = delete;
public:
    void run();
private:
    std::unique_ptr<emu_runner> m_runner;
    instruction_index m_instructions;
private:
    void clear_screen();
    void output_display();
    void show_help();
    void show_short_help();
    void output_state();
private:
    void show_message(const mk_message_t message_type, const std::string message);
    strings_t parse_cmdline(const std::string& cmdline);
    mk_parse_result parse_input(const std::string& cmd);
    void load_state(const std::string& filename);
    void save_state(const std::string& filename);
};

#endif // MK61COMMANDER_H_INCLUDED
