#include "mk_common.h"


/**
* strutils
*/

std::string strutils::to_upper(const std::string& s)
{
    std::string result;
    for (const auto& c : s)
        result += (char)toupper(c);
    return result;
}

/**
 * MK72Engine
 */

mk_engine::mk_engine()
{
    m_is_output_required = false;
    m_powerState = engine_power_state_t::engine_off;
}

mk_engine::~mk_engine()
{

}

bool mk_engine::is_output_required()
{
    return m_is_output_required;
}

mk_result_t mk_engine::end_output()
{
    m_is_output_required = false;
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

