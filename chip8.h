#include <stdint.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

class Chip8
{
    public:
        void Initialize();
        void LoadFontSet();
        bool LoadRom(const char *);
        int SizeRom(FILE *);
        void ExecuteOpcode();
    private:
        uint16_t opcode;
        uint16_t pc;
        uint16_t I;
        uint8_t V[16];
        uint8_t memory[4096];
        uint8_t sp;
        uint16_t stack[16];
        uint8_t graphics[64][32];
        uint8_t delay_timer;
        uint8_t sound_timer;
        uint8_t keyboard[16];
};
