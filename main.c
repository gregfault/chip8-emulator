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

                case 0x0: {
                    //multiple instructions for type 0 hence if statements
                    if (instruction == 0x00E0) { //clearing screen
                        for (int i = 0; i<32; i++) {
                            for (int j = 0; j<64; j++) {
                                chip.screen[i][j] = 0;
                            }
                        }
                        break;
                    }
                    if ( instruction == 0x00EE) { //return, pop the return address from the stack
                        chip.sp--;
                        chip.pc = chip.stack[chip.sp];
                        break;
                    }
                    printf("invalid instruction: %04X \n", instruction);
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

                case 0x2: {
                    chip.stack[chip.sp] = chip.pc;
                    chip.sp++;
                    chip.pc = NNN;
                    break;
                }

                    //cases 0x3, 0x4, 0x5, 0x9 are basically replacement for if statements similar to conditional jumps in x86 putting it simply
                    //skipping by 2 so one instance of instruction is skipped if the criteria is met
                case 0x3: {
                    if (chip.V[X] == NN) {
                        chip.pc += 2;
                    }
                    break;
                }

                case 0x4: {
                    if (chip.V[X] != NN) {
                        chip.pc += 2;
                    }
                    break;
                }

                case 0x5 : {
                    if (chip.V[X] == chip.V[Y]) {
                        chip.pc += 2;
                    }
                    break;
                }

                case 0x9: {
                    if (chip.V[X] != chip.V[Y]) {
                        chip.pc += 2;
                    }
                    break;
                }

                case 0x8: {
                    //calc
                    switch (N) {
                        case 0x0: { //copy
                            chip.V[X] = chip.V[Y];
                            break;
                        }
                        case 0x1: { //or
                            chip.V[X] |= chip.V[Y];
                            break;
                        }
                        case 0x2: { //and
                            chip.V[X] &= chip.V[Y];
                            break;
                        }
                        case 0x3: { //xor
                            chip.V[X] ^= chip.V[Y];
                            break;
                        }
                        //cases below can set the VF flag to 1 if there is overflow
                        case 0x4: { //add
                            uint16_t sum = chip.V[X] + chip.V[Y];
                            chip.V[X] = sum & 0xFF;
                            sum = sum >> 8;
                            if (sum == 1) {
                                chip.V[0xF] = 1;
                            }else chip.V[0xF] = 0;
                            break;
                        }
                        case 0x5: { //subtract
                            uint8_t x_copy = chip.V[X];
                            chip.V[X] -= chip.V[Y];
                            if (x_copy >= chip.V[Y]) {
                                chip.V[0xF] = 1;
                            }else chip.V[0xF] = 0;
                            break;
                        }
                        case 0x6: { //move by one bit
                            uint8_t old_y = chip.V[Y];
                            chip.V[X] = chip.V[Y] >> 1;
                            chip.V[0xF] = old_y & 0x1;
                            break;
                        }
                        case 0x7: {
                            uint8_t flag;
                            if (chip.V[Y] >= chip.V[X]) {
                                flag = 1;
                            }else flag = 0;
                            chip.V[X] = chip.V[Y] - chip.V[X];
                            chip.V[0xF] = flag;
                            break;
                        }
                        case 0xE: { //move by one bit left
                            uint8_t flag = chip.V[Y] >> 7;
                            chip.V[X] = chip.V[Y] << 1;
                            if (flag == 1) {
                                chip.V[0xF] = 1;
                            }else chip.V[0xF] = 0;
                            break;
                        }
                        default: {
                            printf("invalid instruction: %04X \n", instruction);
                            break;
                        }
                    }
                    break;
                }


                case 0xF: {
                    switch (NN) {
                        case 0x1E: {
                            chip.I += chip.V[X];
                            break;
                        }
                        case 0x33: {
                            uint8_t VX_copy = chip.V[X];
                            chip.memory[chip.I] = VX_copy / 100;
                            chip.memory[chip.I+1] = (VX_copy / 10) % 10;
                            chip.memory[chip.I+2] = VX_copy % 10;
                            break;
                        }
                        case 0x55: {
                            for (int i = 0; i<=X; i++) {
                                chip.memory[chip.I + i] = chip.V[i];
                            }
                            chip.I = chip.I + X + 1;
                            break;
                        }
                        case 0x65: {
                            for (int i = 0; i<=X; i++) {
                                chip.V[i] = chip.memory[chip.I + i];
                            }
                            chip.I = chip.I + X + 1;
                            break;
                        }


                        default: {
                            printf("invalid instruction: %04X \n", instruction);
                            break;
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