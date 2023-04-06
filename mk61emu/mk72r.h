#ifndef MK72R_H
#define MK72R_H

#include "mk72.h"
#include "mk61emu.h"

#define MK72R_VER_MAJOR 0
#define MK72R_VER_MINOR 1
#define MK72R_VER_STR STRINGIFY(MK72R_VER_MAJOR) "." STRINGIFY(MK72R_VER_MINOR)

EXTERN_C int test();

enum MK72rMode
{
    MK_MODE_EMU61,
    MK_MODE_RAPIRA
};

class MK72r
{
public:
    MK72r(MK72rMode mode);
    ~MK72r();
    MK72Result Init();
public:
    MK72Result BeginOutput();
    const char* GetOutput();
    MK72Result EndOutput();
    MK72Result BeginInput(const size_t length);
    void CancelInput();
    MK72Result WriteToInput(const char buf[], const size_t length);
    MK72Result EndInput();

    MK72Result DoKeyPress(const int key1, const int key2);
    MK72Result DoStep();
    const char* GetVersionStr();
    bool IsFinished();
    bool IsOutputRequired();

private:
    MK72rMode m_mode;
    MK72Engine* m_engine;
    bool m_finished;
    char* m_buf;
    size_t m_bufMaxLength, m_bufCurLength;
    char* m_bufPtr;
};


#endif // MK72R_H
