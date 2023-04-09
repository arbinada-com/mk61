#include "mk_common.h"

/**
 * MK72Engine
 */

mk_engine::mk_engine()
{
    m_outputRequired = false;
    m_powerState = engine_power_state_t::engine_off;
}

mk_engine::~mk_engine()
{

}

bool mk_engine::is_output_required()
{
    return m_outputRequired;
}

mk_result_t mk_engine::end_output()
{
    m_outputRequired = false;
    return mk_result_t::mk_ok;
}


engine_power_state_t mk_engine::get_power_state()
{
    return m_powerState;
}

mk_result_t mk_engine::set_power_state(const engine_power_state_t value)
{
    if (m_powerState != value)
    {
        m_powerState = value;
    }
    return mk_result_t::mk_ok;
}

