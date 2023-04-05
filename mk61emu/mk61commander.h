#ifndef MK61COMMANDER_H_INCLUDED
#define MK61COMMANDER_H_INCLUDED

#include "mk61emu.h"

typedef enum
{
    mk61cmd_unknown,
    mk61cmd_empty,
    mk61cmd_quit,
    mk61cmd_load,
    mk61cmd_save,
    mk61cmd_on,
    mk61cmd_off,
    mk61cmd_keys
} mk61commander_cmd_kind_t;

typedef enum
{
    mk_message_info,
    mk_message_warn,
    mk_message_error
} mk_message_t;

typedef struct
{
    bool parsed;
    mk61commander_cmd_kind_t cmd_kind;
} mk61commander_parse_result_t;

class MK61Commander
{
public:
    MK61Commander();
    ~MK61Commander();
    void Run();
private:
    MK61Emu *m_emu;
    mk61commander_parse_result_t m_last_parse_result;
    int mk_show_message(const mk_message_t message_type, const char *format, ... );
private:
    bool GetCommand(char *cmdstr, int num);
    bool InputName(char *str, size_t max_size);
    void OutputState();
    mk61commander_parse_result_t ParseInput(const char *cmd);
};

#endif // MK61COMMANDER_H_INCLUDED
