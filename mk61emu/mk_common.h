#ifndef MK_COMMON_INCLUDED
#define MK_COMMON_INCLUDED

#include <cinttypes>
#include <string>

typedef unsigned char byte;

class strutils
{
public:
    static std::string to_upper(const std::string& s);
};

class mk_instruction
{
public:
    mk_instruction(int32_t code, std::string mnemonics, std::string caption)
        : m_code(code), m_mnemonics(mnemonics), m_caption(caption)
    {}
    mk_instruction(int32_t code, std::string mnemonics)
        : mk_instruction(code, mnemonics, "")
    {}
    mk_instruction(const mk_instruction&) = default;
    mk_instruction& operator =(const mk_instruction&) = default;
    mk_instruction(mk_instruction&&) = default;
    mk_instruction& operator =(mk_instruction&&) = default;
public:
    static const int no_code = -1;
public:
    int32_t code() const { return m_code; }
    const std::string& mnemonics() const { return m_mnemonics; }
    const std::string& caption() { return m_caption; }
private:
    int32_t m_code;
    std::string m_mnemonics;
    std::string m_caption;
};

enum class mk_result_t
{
    mk_ok = 0,
    mk_error
};

enum class engine_power_state_t
{
    engine_off = 0,
    engine_on = 1
};

class mk_engine
{
public:
    mk_engine();
    virtual ~mk_engine();
    virtual bool is_output_required();
    virtual mk_result_t end_output();
    virtual mk_result_t do_step() = 0;
    virtual mk_result_t do_input(const char* buf, size_t length) = 0;
    virtual mk_result_t do_key_press(const uint8_t key1, const uint8_t key2) = 0;
    virtual engine_power_state_t get_power_state();
    virtual mk_result_t set_power_state(const engine_power_state_t value);
protected:
    bool m_is_output_required = false;
private:
    engine_power_state_t m_powerState;
};

#endif // MK_COMMON_INCLUDED
