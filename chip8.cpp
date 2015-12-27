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

    switch((opcode & 0xF000)>>12) 
    {
        case 0x0:
            if (opcode & 0x00FF == 0xEE) {
                //RET - return from a subroutine
                pc = stack[--SP];
            } else if (opcode & 0x00FF == 0xE0) {
                //CLS - clear the display
                for (int y=0; y < 64; y++)
                    for (int x=0; x < 32; x++)
                        graphics[y][x] = 0;
            } else {
                //0nnn - SYS addr
                pc = opcode & 0x0FFF;
            }
            break;
        case 0x1: //1nnn - JP addr
            pc = opcode & 0x0FFF;
            break;
        case 0x2: //2nnn - call subroutine
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
            break;
        case 0x3: //3xkk - skip next instruction if Vx==kk
            if (V[(opcode & 0x0F00)>>8] == opcode & 0x00FF) {
                pc += 2;
            }
            break;
        case 0x4: //4xkk - skip next instruction if Vx!=kk
            if (V[(opcode & 0x0F00)>>8] != opcode & 0x00FF) {
                pc += 2;
            }
            break;
        case 0x5: //4xy0 - skip next instruction if Vx!=kk
            if (V[(opcode & 0x0F00)>>8] == V[(opcode & 0x00F0)>>4]) {
                pc += 2;
            }
            break;
        case 0x6: //6xkk - Vx=kk
            V[(opcode & 0x0F00)>>8] = opcode & 0x00FF;
            break;
        case 0x7: //7xkk - Vx+=kk
            V[(opcode & 0x0F00)>>8] += 0x00FF;
            break;
        case 0x8: 
            switch(opcode & 0x000F) {
                case 0x0: //Vx = Vy
                    V[(opcode & 0x0F00)>>8] = V[(opcode & 0x0F0)>>4];
                    break;
                case 0x1: //Vx = Vx | Vy 
                    V[(opcode & 0x0F00)>>8] |= V[(opcode & 0x0F0)>>4];
                    break;
                case 0x2: //Vx = Vx & Vy 
                    V[(opcode & 0x0F00)>>8] &= V[(opcode & 0x0F0)>>4];
                    break;
                case 0x3: //Vx = Vx XOR Vy 
                    V[(opcode & 0x0F00)>>8] ^= V[(opcode & 0x0F0)>>4];
                    break;
                case 0x4: //Vx = Vx + Vy 
                    int temp = V[(opcode & 0x0F00)>>8] + V[(opcode & 0x0F0)>>4];
                    if (temp > 255) {
                       V[0xF] = 1;
                       V[(opcode & 0x0F00)>>8] = temp & 0xFF;
                    } else {
                    V[0xF] = 0;
                    V[(opcode & 0xF00)>>8] = temp;
                    }
                    break;
                case 0x5: //Vx = Vx - Vy
                    int temp = V[(opcode & 0x0F00)>>8] - V[(opcode & 0x0F0)>>4];
                    if (temp > 0) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00)>>8] = temp;
                    break;
                case 0x6: //Vx = Vx / 2
                    V[0xF] = V[(opcode & 0x0F00)>>8] & 0x1;
                    V[(opcode & 0x0F00)>>8] >>= 1;
                    break;
                case 0x7: //Vx = Vy - Vx
                    int temp = V[(opcode & 0x00F0)>>4] - V[(opcode & 0xF00)>>8];
                    if (temp > 0) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00)>>8] = temp;
                    break;
                case 0xE: //Vx = Vx * 2
                    V[0xF] = V[(opcode & 0x0F00)>>8] & 0x80;
                    V[(opcode & 0x0F00)>>8] <<= 1;
                    break;

                case 0xE: //Vx=Vx|Vy 
                    V[(opcode & 0x0F00)>>8] |= V[(opcode & 0x0F0)>>4];
                    break;
                default:
                    cout << "opcode not recognised: " << hex << opcode << endl;
            }
            break;
        case 0xA: //Annn - I=nnn
            I = opcode & 0xFFF;
            break;
        case 0xD: //Dxyn, draw n bytes starting at memory location I, at (Vx, Vy)
            break;

        case 
            
            
        
    }

}

int main() 
{
    Chip8 chip8;
    
    chip8.Initialize();
    chip8.LoadRom("PONG");
    chip8.ExecuteOpcode();
}
