#include <string.h>
#include "mk72.h"
#include "mk72r.h"

int test()
{
    return 1;
}


MK72r::MK72r(MK72rMode mode)
{
  m_mode = mode;
  m_engine = NULL;
}

MK72r::~MK72r()
{
    if (m_engine != NULL)
    {
        m_engine->SetPowerState(MK72Engine_Off);
        delete m_engine;
    }
}


MK72Result MK72r::Init()
{
  m_finished = true;
  switch (m_mode)
  {
  case MK_MODE_EMU61:
      m_engine = new MK61Emu();
      m_engine->SetPowerState(MK72Engine_On);
      break;
  case MK_MODE_RAPIRA:
      return MK_ERROR;
  }
  return MK_OK;
}

MK72Result MK72r::BeginOutput()
{
    return MK_OK;
}

const char* MK72r::GetOutput()
{
    return "Test\n";
}

MK72Result MK72r::EndOutput()
{
    return MK_OK;
}

MK72Result MK72r::BeginInput(const size_t length)
{
    m_bufMaxLength = 0;
    m_buf = (char*) mk_malloc(length);
    if (m_buf == 0)
        return MK_ERROR;
    m_bufCurLength = 0;
    m_bufMaxLength = length;
    m_bufPtr = m_buf;
    return MK_OK;
}

void MK72r::CancelInput()
{
    if (m_buf != NULL)
        mk_free(m_buf);
}

MK72Result MK72r::WriteToInput(const char buf[], const size_t length)
{
    if ((m_bufCurLength + length) > m_bufMaxLength)
        return MK_ERROR;
    memcpy(m_bufPtr, buf, length);
    m_bufPtr += length;
    m_bufCurLength += length;
    return MK_OK;
}

MK72Result MK72r::EndInput()
{
    mk_free(m_buf);
    return MK_OK;
}

MK72Result MK72r::DoKeyPress(const int key1, const int key2)
{
    return m_engine->DoKeyPress(key1, key2);
}

MK72Result MK72r::DoStep()
{
    return m_engine->DoStep();
}


const char* MK72r::GetVersionStr()
{
  return MK72R_VER_STR;
}

bool MK72r::IsFinished()
{
    return m_finished;
}

bool MK72r::IsOutputRequired()
{
    return m_engine->IsOutputRequired();
}
