#ifndef MK_COMMON_INCLUDED
#define MK_COMMON_INCLUDED

typedef unsigned char byte;

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
    virtual mk_result_t do_key_press(const int key1, const int key2) = 0;
    virtual engine_power_state_t get_power_state();
    virtual mk_result_t set_power_state(const engine_power_state_t value);
protected:
    bool m_outputRequired;
private:
    engine_power_state_t m_powerState;
};

#endif // MK_COMMON_INCLUDED
