#include "chip8.h"

int main()
{
    Chip8 chip8;

    chip8.Initialize();
    chip8.LoadRom("PONG");
    chip8.ExecuteOpcode();
}
