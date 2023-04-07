#ifndef MK61EMU_H_INCLUDED
#define MK61EMU_H_INCLUDED

#define MK61EMU_VERSION_MAJOR 1
#define MK61EMU_VERSION_MINOR 1

#include "mk_common.h"

typedef uint32_t microinstruction_t; // 4-byte microinstructions
typedef uint32_t instruction_t;      // 4-byte instructions
typedef uint8_t io_t;
typedef uint8_t mtick_t;
typedef uint16_t tick_t;

typedef struct
{
    microinstruction_t microinstructions[68];
    instruction_t instructions[256];
    uint8_t microprograms[1152];
} IK13_ROM;

/**
 * mk61ROM
 */
typedef struct
{
    IK13_ROM IK1302;
    IK13_ROM IK1303;
    IK13_ROM IK1306;
} mk61ROM_t;

/**
 * IK13
 */
const uint8_t IK13_MTICK_COUNT = 42;

/**
 * The IK13 chip
 */
class IK13
{
    friend class mk61_emu;
public:
    IK13();
private:
    uint16_t GetStateSizeInBytes();
    char* ReadState(char *state);
    char* WriteState(char *state);
    void SetROM(const IK13_ROM *ROM);
    void Tick();
private:
    const IK13_ROM *ROM;
    io_t R[IK13_MTICK_COUNT];
    io_t M[IK13_MTICK_COUNT];
    io_t ST[IK13_MTICK_COUNT];
    io_t S, S1, L, T, P;
    mtick_t mtick;
    microinstruction_t microinstruction;
    io_t AMK, ASP, AK, MOD;
    io_t input;
    io_t output;
    int8_t key_x, key_y, comma;
};

/**
 * IR2
 */
const uint8_t IR2_MTICK_COUNT = 252;

class IR2
{
    friend class mk61_emu;
public:
    IR2();
private:
    size_t GetStateSizeInBytes();
    char* ReadState(char *state);
    char* WriteState(char *state);
    void Tick();
private:
    io_t M[IR2_MTICK_COUNT];
    mtick_t mtick;
    io_t input;
    io_t output;
};

extern IR2 *IR2_create();
extern void IR2_free(IR2 *self);

/**
 * Chipset MK61
 */
typedef enum
{
    mk61emu_mode_61,
    mk61emu_mode_54
} mk61emu_mode;

typedef char mk61_register_position_t;
const int mk61_register_positions_count = 14;
typedef mk61_register_position_t mk61_register_t[mk61_register_positions_count];

const uint8_t MK61EMU_REG_STACK_COUNT = 5;
typedef enum
{
    mk61emu_RX1 = 0,
    mk61emu_RX  = 1,
    mk61emu_RY  = 2,
    mk61emu_RZ  = 3,
    mk61emu_RT  = 4
} mk61emu_reg_stack_t;

const uint8_t MK61EMU_REG_MEM_COUNT = 15;
typedef enum
{
    mk61emu_R0 = 0,
    mk61emu_R1 = 1,
    mk61emu_R2 = 2,
    mk61emu_R3 = 3,
    mk61emu_R4 = 4,
    mk61emu_R5 = 5,
    mk61emu_R6 = 6,
    mk61emu_R7 = 7,
    mk61emu_R8 = 8,
    mk61emu_R9 = 9,
    mk61emu_RA = 10,
    mk61emu_RB = 11,
    mk61emu_RC = 12,
    mk61emu_RD = 13,
    mk61emu_RE = 14
} mk61emu_reg_mem_t;

typedef enum
{
    angle_unit_radian = 10,
    angle_unit_degree = 11,
    angle_unit_grade  = 12
} angle_unit_t;

typedef struct
{
    bool succeeded;
    char message[128];
} mk61emu_result_t;

/**
 * The MK61 emulator class
 */
class mk61_emu : public mk_engine
{
public:
    mk61_emu();
    virtual ~mk61_emu();
    const char* get_reg_stack_str(mk61emu_reg_stack_t reg);
    angle_unit_t get_angle_unit();
    void set_angle_unit(const angle_unit_t value);
    const char* get_angle_unit_str();
    const char* get_indicator_str();
    const char* get_prog_counter_str();
    const char* get_reg_mem_str(mk61emu_reg_mem_t reg);
    mk_result_t do_step() override;
    virtual mk_result_t do_input(const char* buf, size_t length);
    virtual mk_result_t do_key_press(const int key1, const int key2);
    virtual bool is_output_required();
    virtual mk_result_t set_power_state(const engine_power_state_t value);
    bool is_running();
    bool load_state(const char *name, mk61emu_result_t *result);
    bool save_state(const char *name, mk61emu_result_t *result);
private:
    void clear_registers();
    static void clear_register_str(mk61_register_t &reg);
    void cleanup();
    static const char* get_file_result_message(mk_file_result_t result);
    size_t get_state_size_bytes();
    void read_all_fields(uint8_t replacement);
    void read_number(mk61_register_t &reg, uint8_t chip, unsigned char address);
    void tick();
private:
    mk61emu_mode m_mode;
    angle_unit_t m_angle_unit;
    IR2 *m_IR2_1, *m_IR2_2;
    IK13 *m_IK1302, *m_IK1303, *m_IK1306;
    mk61_register_t m_reg_stack[MK61EMU_REG_STACK_COUNT]; // X1, X, Y, Z, T;
    mk61_register_t m_reg_mem[MK61EMU_REG_MEM_COUNT];  // R1, R2, R3, R4, R5, R6, R7, R8, R9, RA, RB, RC, RD, RE;
    mk61_register_position_t m_prog_counter[2];
    mk61_register_position_t m_returns[5][2];
    char m_prog_counter_str[3];
    char m_indicator_str[15];
    bool m_RSModeChanged;
};

#endif // MK61EMU_H_INCLUDED
