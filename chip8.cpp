#include <stdint.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

class Chip8
{
    public:
        void Initialize();
        void LoadFontSet();
        bool LoadRom(char *);
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

void Chip8::Initialize() 
{
    //most CHIP-8 programs start from 0x200
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;

    //clear memory, screen, stack, registers, keyboard
    for (int i=0; i < 4096; i++)
        memory[i] = 0;

    for (int i=0; i < 16; i++)
        stack[i]= 0;

    for (int i=0; i < 16; i++)
        keyboard[i] = 0;

    for (int i=0; i < 16; i++)
        V[i]= 0;
    
    for (int y=0; y < 64; y++)
        for (int x=0; x < 32; x++)
            graphics[y][x] = 0;

    //load fontset into memory
    LoadFontSet();
}

void Chip8::LoadFontSet() 
{
    uint8_t font[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0   
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0X80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0x80, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80 //F
    };
    
    for (int i=0; i < sizeof(font); i++)
        memory[i] = font[i];    
}

bool Chip8::LoadRom(char *filename) 
{
    FILE *rom;
    if ((rom = fopen(filename, "rb")) == NULL) {
        cout << "unable to open rom file" << endl;
        return false;
    }
    int filesize = SizeRom(rom);
    unsigned char buffer[filesize];
    fread(buffer, filesize, 1, rom);
    for (int i=0; i < filesize; i++) {
        memory[0x200 + i] = buffer[i];
    }
    return true;
}

int Chip8::SizeRom(FILE *rom) 
{
    int cur_pos = ftell(rom);
    fseek(rom, 0, SEEK_END);
    int end_pos = ftell(rom);
    fseek(rom, cur_pos, SEEK_SET);
    return end_pos;
}

void Chip8::ExecuteOpcode()
{
    opcode = memory[pc]<<8 | memory[pc+1];
    pc += 2;
    cout << hex << opcode << endl;
}

int main() 
{
    Chip8 chip8;
    
    chip8.Initialize();
    chip8.LoadRom("PONG");
    chip8.ExecuteOpcode();
}
