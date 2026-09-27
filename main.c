#include <stdint.h>
#include <stdio.h>

struct chip8 {
    uint8_t memory[4096];
    uint8_t V[16];
    uint8_t sp;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t screen[32][64];
    uint8_t keys[16];
    uint16_t I;
    uint16_t pc;
    uint16_t stack[16];
};

int load_rom(struct chip8 *chip, const char *path) {
    chip->pc = 0x200; //setting the first address for the emulator to read starting from 512
    FILE *rom;
    rom = fopen(path, "rb");
    if (rom) {
        fread(&chip->memory[chip->pc], 1, sizeof(chip->memory)-0x200, rom);
        fclose(rom);
        return 0;
    }return -1;
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        struct chip8 chip = {0};
        load_rom(&chip, argv[1]);

        for (int j=0; j<30; j++) {
            uint8_t top = chip.memory[chip.pc];
            uint8_t bottom = chip.memory[chip.pc + 1];
            uint16_t instruction = (top << 8) | bottom; // merging two bytes into one instruction top+bottom
            chip.pc += 2;
            uint8_t TYPE = instruction >> 12;
            uint8_t X = (instruction >> 8) & 0xF;
            uint8_t Y = (instruction >> 4) & 0xF;
            uint8_t N = instruction & 0xF;
            uint8_t NN = instruction & 0xFF;
            uint16_t NNN = instruction & 0xFFF;

            switch (TYPE) {

                case 0x6: {
                    chip.V[X] = NN;
                    break;
                }

                case 0xA: {
                    chip.I = NNN;
                    break;
                }

                case 0x7: {
                    chip.V[X] += NN;
                    break;
                }

                case 0x1: {
                    chip.pc = NNN;
                    break;
                }


                case 0xD: { //case for painting the screen
                    uint8_t x = chip.V[X] % 64;
                    uint8_t y = chip.V[Y] % 32;
                    chip.V[0xF] = 0; //flag used for collisions
                    for (int i=0; i<N; i++) {
                        uint8_t paint = chip.memory[chip.I+i];
                        for (int b=0; b<8; b++) {
                            uint8_t pixel = (paint >> (7-b)) & 1;
                            uint8_t px = x + b;
                            uint8_t py = y + i;
                            if (px >= 64 || py >= 32) continue;
                            if (pixel) {
                                if (chip.screen[py][px] == 1) {
                                    chip.V[0xF] = 1;
                                }
                                chip.screen[py][px] ^= 1; //flipping the pixel from 1 to 0 and otherwise
                            }
                        }
                    }
                    break;
                }

                default: {
                    printf("invalid instruction: %04X \n", instruction);
                    break;
                }
            }
        }

        for (int i =0; i<32;i++) {
            for (int j=0; j<64; j++) {
                if (chip.screen[i][j] == 1) printf("#");
                else printf(" ");
            }
            printf("\n");
        }
        return 0;
    }
    printf("file address not given \n");
    return 1;
}