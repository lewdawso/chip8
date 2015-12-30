#include "chip8.h"
#include <stdint.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

using namespace std;

void Chip8::Initialize() 
{
    //most CHIP-8 programs start from 0x200
    pc = 0x200;

    //initialize everything to zero
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

bool Chip8::LoadRom(const char *filename) 
{
    FILE *rom;
    if ((rom = fopen(filename, "rb")) == NULL) {
        cout << "unable to open rom file: " << *filename << endl;
        return false;
    }
    const int filesize = SizeRom(rom);
    unsigned char buffer[filesize];
    fread(buffer, filesize, 1, rom);
    for (int i=0; i < filesize; i++) {
        memory[0x200 + i] = buffer[i];
    }
    return true;
}

int Chip8::SizeRom(FILE *rom) 
{
    const int cur_pos = ftell(rom);
    fseek(rom, 0, SEEK_END);
    const int end_pos = ftell(rom);
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
            if ((opcode & 0x00FF) == 0xEE) 
            {
                //RET - return from a subroutine
                pc = stack[--sp];
            } else if ((opcode & 0x00FF) == 0xE0) {
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
            if ((V[(opcode & 0x0F00)>>8]) == (opcode & 0x00FF)) 
            {
                pc += 2;
            }
            break;
        case 0x4: //4xkk - skip next instruction if Vx!=kk
            if ((V[(opcode & 0x0F00)>>8]) != (opcode & 0x00FF)) 
            {
                pc += 2;
            }
            break;
        case 0x5: //4xy0 - skip next instruction if Vx!=kk
            if (V[(opcode & 0x0F00)>>8] == V[(opcode & 0x00F0)>>4]) 
            {
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
            switch(opcode & 0x000F) 
            {
                case 0x0: //Vx = Vy
                    V[(opcode & 0x0F00)>>8] = V[(opcode & 0x00F0)>>4];
                    break;
                case 0x1: //Vx = Vx | Vy 
                    V[(opcode & 0x0F00)>>8] |= V[(opcode & 0x00F0)>>4];
                    break;
                case 0x2: //Vx = Vx & Vy 
                    V[(opcode & 0x0F00)>>8] &= V[(opcode & 0x00F0)>>4];
                    break;
                case 0x3: //Vx = Vx XOR Vy 
                    V[(opcode & 0x0F00)>>8] ^= V[(opcode & 0x00F0)>>4];
                    break;
                case 0x4: { //Vx = Vx + Vy 
                    const int temp = V[(opcode & 0x0F00)>>8] + V[(opcode & 0x00F0)>>4];
                    if (temp > 255) 
                    {
                       V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0xF00)>>8] = temp & 0xFF;
                    break;
                }
                case 0x5: { //Vx = Vx - Vy
                    const int temp = V[(opcode & 0x0F00)>>8] - V[(opcode & 0x00F0)>>4];
                    if (temp > 0) 
                    {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00)>>8] = temp;
                    break;
                }
                case 0x6: //Vx = Vx / 2
                    V[0xF] = V[(opcode & 0x0F00)>>8] & 0x1;
                    V[(opcode & 0x0F00)>>8] >>= 1;
                    break;
                case 0x7: { //Vx = Vy - Vx
                    const int temp = V[(opcode & 0x00F0)>>4] - V[(opcode & 0xF00)>>8];
                    if (temp > 0) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00)>>8] = temp;
                    break;
                }
                case 0xE: //Vx = Vx * 2
                    V[0xF] = V[(opcode & 0x0F00)>>8] & 0x80;
                    V[(opcode & 0x0F00)>>8] <<= 1;
                    break;
                default:
                    cout << "opcode not recognised: " << hex << opcode << endl;
            }
            break;
        case 0x9: //9xy0 - Skip next instruction if Vx != Vy
            if ((V[(opcode & 0x0F00)>>8]) != (V[(opcode & 0x00F0)])) 
            {
                pc += 2;
            }
            break;
        case 0xA: //Annn - I=nnn
            I = opcode & 0xFFF;
            break;
        case 0xB: //Bnnn - I=nnn+V0
            pc = opcode & 0xFFF + V[0];
            break;
        case 0xC: //Cxkk - Vx = RAND & kk
            V[(opcode & 0x0F00)>>8] = ((rand() % 256) & (opcode & 0xFF));
            break;
        case 0xD: { //Dxyn, draw n bytes starting at memory location I, at coordinates (Vx, Vy)
            //a sprite is an n*8 block, the width of which is 'filled' by the byte at I
            // if any pixels are erased, Vf = 1, otherwise Vf = 0
            const uint8_t vx = V[opcode & 0xF00];
            const uint8_t vy = V[opcode & 0xF0];
            const uint8_t n = opcode & 0xF;
            uint8_t x;
            uint8_t y;
            for (int dy=0; dy<n; dy++) 
            {
                uint8_t byte = memory[I+y];
                for (int dx=0; dx<8; dx++)
                {
                    if (vx+dx>32)
                    {
                        x = (vx+dx) - 32;
                    } else {
                        x = (vx+dx);
                    }
                    if (vy+y>32) 
                    {
                        y = (vy+dy) - 64;
                    } else {
                        y = (vy+dy);
                    }

                    if (byte & (1<<(8-dx)))
                    {
                        V[0xF] = graphics[y][x];
                        graphics[y][x] ^= 1;
                    } else {
                        V[0xF] = 0;
                    }
                }
            }
            break;
        }
        case 0xE:
            switch(opcode & 0xFF) 
            {
                case 0x9E: //Ex9E - skip next instruction if key V[x] is pressed
                    if (keyboard[V[(opcode & 0x0F00)>>8]])
                    {
                        pc += 2;
                    }
                    break;
                case 0xA1: //ExA1 - skip next instruction if key V[x] is not pressed
                    if (keyboard[V[(opcode & 0x0F00)>>8]])
                    {
                        pc += 2;
                    }
                    break;
                default:
                    cout << "opcode not recognised: " << hex << opcode << endl;
            }
            break;
        case 0xF:
            switch(opcode & 0xFF)
            {
                case 0x07: //Fx07 - Vx = delay_timer
                    V[(opcode & 0x0F00)>>8] = delay_timer;
                    break;
                case 0x0A: { //Fx0A - wait for user to press key, Vx = key
                    bool press = false;
                    while(!press)
                    {
                        for (int i=0; i<16; i++)
                        {
                            if (keyboard[i])
                            {
                                V[(opcode & 0x0F00)>>8] = i;
                                press = true;
                                break;
                            }
                        }
                    }
                    break;
                }
                case 0x15: //Fx15 - delay_timer = Vx
                    delay_timer = V[(opcode & 0x0F00)>>8];
                    break;
                case 0x18: //Fx18 - sound_timer = Vx
                    sound_timer = V[(opcode & 0x0F00)>>8];
                    break;
                case 0x1E: //Fx18 = I = I + Vx
                    I += V[(opcode & 0x0F00)>>8];
                    break;
                case 0x29: //Fx29 - I = location of sprite for digit Vx
                    I = V[(opcode & 0x0F00)>>8] * 5;
                    break;
                case 0x33: { //Fx33 - BCD representation of Vx in I, I+1, I+2
                    const uint8_t x = V[(opcode & 0x0F00)>>8];
                    memory[I] = (x -(x % 100)) / 100;
                    memory[I+1] = (x - (x % 10) - (memory[I] * 100)) / 10;
                    memory[I+2] = (x - (memory[I] * 100) - (memory[I+1] * 10));
                    break;
                }
                case 0x55: { //Fx55 - store V0 through Vx in memory starting at I
                    const uint8_t x = V[(opcode & 0x0F00)>>8];
                    for (int i=0; i<x; i++)
                    {
                        memory[I+i] = V[i];
                    }
                    break;
                }
                case 0x65: { //Fx65 - store memory[I + (0 through x)] into V
                    const uint8_t x = V[(opcode & 0x0F00)>>8];
                    for (int i=0; i<x; i++)
                    {
                        V[i] = memory[I+i];
                    }
                    break;
                }
                default:
                    cout << "opcode not recognised: " << hex << opcode << endl;
            }
            break;
    }

}
