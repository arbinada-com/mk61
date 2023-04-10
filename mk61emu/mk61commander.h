#ifndef MK61COMMANDER_H_INCLUDED
#define MK61COMMANDER_H_INCLUDED

#include <memory>
#include <iostream>
#include <string>
#include <mutex>
#include <vector>
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

class mk61_commander
{
public:
    mk61_commander();
    mk61_commander(const mk61_commander&) = delete;
    mk61_commander& operator =(const mk61_commander&) = delete;
public:
    void run();
private:
    std::unique_ptr<emu_runner> m_runner;
private:
    void clear_screen();
    void output_display();
    void show_help();
    void output_state();
private:
    void show_message(const mk_message_t message_type, const std::string message);
    strings_t parse_cmdline(const std::string& cmdline);
    mk_parse_result parse_input(const std::string& cmd);
    void load_state(const std::string& filename);
    void save_state(const std::string& filename);
};

#endif // MK61COMMANDER_H_INCLUDED
