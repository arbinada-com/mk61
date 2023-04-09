#ifndef MK61COMMANDER_H_INCLUDED
#define MK61COMMANDER_H_INCLUDED

#include <memory>
#include <iostream>
#include <string>
#include "mk61emu.h"

enum class mk_cmd_kind_t
{
    cmd_unknown,
    cmd_empty,
    cmd_quit,
    cmd_load,
    cmd_save,
    cmd_on,
    cmd_off,
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

class mk61_commander
{
public:
    mk61_commander();
    void run();
private:
    std::unique_ptr<mk61_emu> m_emu;
    mk_parse_result m_last_parse_result;
private:
    void clear_display();
    void output_display();
    void output_state();
private:
    int mk_kbhit(void);
    int mk_getch(const bool nowait, const bool echo);
    bool get_command(std::string& cmd);
    void mk_show_message(const mk_message_t message_type, const std::string message);
    mk_parse_result parse_input(const std::string& cmd);
    void load_state(const std::string& filename);
    void save_state(const std::string& filename);
};

#endif // MK61COMMANDER_H_INCLUDED
