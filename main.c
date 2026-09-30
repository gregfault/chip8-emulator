#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#define SCALE 20 //scale used for sdl2
#define INSTRUCTIONS_PER_FRAME 20 //basic amount of instructions per frame for chip8

struct chip8 {
    uint8_t memory[4096];
    uint8_t V[16];
    uint8_t sp;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t screen[32][64];
    uint8_t keys[16];
    uint8_t waiting_release;
    uint8_t pressed_key;
    uint16_t I;
    uint16_t pc;
    uint16_t stack[16];
};

//font for the emulator
static const uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

//static keys so that the SDL takes the physical position of the keys not the value of the keys itself type shi
//it's used to map the keys that used to be on the machine itself it used to be a 4x4 grid of keys im using 1-4 1-Z
static const SDL_Scancode keymap[16] = {
    SDL_SCANCODE_X, // 0
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, // 1 2 3
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, // 4 5 6
    SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, // 7 8 9
    SDL_SCANCODE_Z, SDL_SCANCODE_C, // A B
    SDL_SCANCODE_4, SDL_SCANCODE_R, // C D
    SDL_SCANCODE_F, SDL_SCANCODE_V // E F
};

int load_rom(struct chip8 *chip, const char *path) {
    chip->pc = 0x200; //setting the first address for the emulator to read starting from 512
    FILE *rom;
    rom = fopen(path, "rb");
    if (rom) {
        fread(&chip->memory[chip->pc], 1, sizeof(chip->memory) - 0x200, rom);
        fclose(rom);
        return 0;
    }
    return -1;
}



void load_font(struct chip8 *chip) {
    //loading the font into the chips memory starting from 0x050
    for (int i = 0; i < 80; i++) {
        chip->memory[0x050 + i] = font[i];
    }
}

int handle_events(struct chip8 *chip) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                return 0;
            }
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                SDL_Scancode sc = event.key.keysym.scancode;
                int key_pressed = (event.type == SDL_KEYDOWN);
                for (int i = 0; i < 16; i++) {
                    if (keymap[i] == sc) {
                        chip->keys[i] = key_pressed;
                    }
                }
                break;
            }
        }
    }
    return 1;
}

void execute_instruction(struct chip8 *chip) {
    uint8_t top = chip->memory[chip->pc];
    uint8_t bottom = chip->memory[chip->pc + 1];
    uint16_t instruction = (top << 8) | bottom; //merging two bytes into one instruction top+bottom
    chip->pc += 2;
    uint8_t TYPE = instruction >> 12;
    uint8_t X = (instruction >> 8) & 0xF;
    uint8_t Y = (instruction >> 4) & 0xF;
    uint8_t N = instruction & 0xF;
    uint8_t NN = instruction & 0xFF;
    uint16_t NNN = instruction & 0xFFF;

        switch (TYPE) {
        case 0x6: {
            chip->V[X] = NN;
            break;
        }

        case 0x0: {
            //multiple instructions for type 0 hence if statements
            if (instruction == 0x00E0) {
                //clearing screen
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < 64; j++) {
                        chip->screen[i][j] = 0;
                    }
                }
                break;
            }
            if (instruction == 0x00EE) {
                //return, pop the return address from the stack
                chip->sp--;
                chip->pc = chip->stack[chip->sp];
                break;
            }
            printf("invalid instruction: %04X \n", instruction);
            break;
        }

        case 0xA: {
            chip->I = NNN;
            break;
        }

        case 0x7: {
            chip->V[X] += NN;
            break;
        }

        case 0x1: {
            chip->pc = NNN;
            break;
        }

        case 0xD: {
            //case for painting the screen
            uint8_t x = chip->V[X] % 64;
            uint8_t y = chip->V[Y] % 32;
            chip->V[0xF] = 0; //flag used for collisions
            for (int i = 0; i < N; i++) {
                uint8_t paint = chip->memory[chip->I + i];
                for (int b = 0; b < 8; b++) {
                    uint8_t pixel = (paint >> (7 - b)) & 1;
                    uint8_t px = x + b;
                    uint8_t py = y + i;
                    if (px >= 64 || py >= 32) continue;
                    if (pixel) {
                        if (chip->screen[py][px] == 1) {
                            chip->V[0xF] = 1;
                        }
                        chip->screen[py][px] ^= 1; //flipping the pixel from 1 to 0 and otherwise
                    }
                }
            }
            break;
        }

        case 0x2: {
            chip->stack[chip->sp] = chip->pc;
            chip->sp++;
            chip->pc = NNN;
            break;
        }

        //cases 0x3, 0x4, 0x5, 0x9 are basically replacement for if statements similar to conditional jumps in x86 putting it simply
        //skipping by 2 so one instance of instruction is skipped if the criteria is met
        case 0x3: {
            if (chip->V[X] == NN) {
                chip->pc += 2;
            }
            break;
        }

        case 0x4: {
            if (chip->V[X] != NN) {
                chip->pc += 2;
            }
            break;
        }

        case 0x5: {
            if (chip->V[X] == chip->V[Y]) {
                chip->pc += 2;
            }
            break;
        }

        case 0x9: {
            if (chip->V[X] != chip->V[Y]) {
                chip->pc += 2;
            }
            break;
        }

        case 0x8: {
            //calc
            switch (N) {
                case 0x0: {
                    //copy
                    chip->V[X] = chip->V[Y];
                    break;
                }
                case 0x1: {
                    //or
                    chip->V[X] |= chip->V[Y];
                    break;
                }
                case 0x2: {
                    //and
                    chip->V[X] &= chip->V[Y];
                    break;
                }
                case 0x3: {
                    //xor
                    chip->V[X] ^= chip->V[Y];
                    break;
                }
                //cases below can set the VF flag to 1 if there is overflow
                case 0x4: {
                    //add
                    uint16_t sum = chip->V[X] + chip->V[Y];
                    chip->V[X] = sum & 0xFF;
                    sum = sum >> 8;
                    if (sum == 1) {
                        chip->V[0xF] = 1;
                    } else chip->V[0xF] = 0;
                    break;
                }
                case 0x5: {
                    //subtract
                    uint8_t x_copy = chip->V[X];
                    chip->V[X] -= chip->V[Y];
                    if (x_copy >= chip->V[Y]) {
                        chip->V[0xF] = 1;
                    } else chip->V[0xF] = 0;
                    break;
                }
                case 0x6: {
                    //move by one bit
                    uint8_t old_y = chip->V[Y];
                    chip->V[X] = chip->V[Y] >> 1;
                    chip->V[0xF] = old_y & 0x1;
                    break;
                }
                case 0x7: {
                    uint8_t flag;
                    if (chip->V[Y] >= chip->V[X]) {
                        flag = 1;
                    } else flag = 0;
                    chip->V[X] = chip->V[Y] - chip->V[X];
                    chip->V[0xF] = flag;
                    break;
                }
                case 0xE: {
                    //move by one bit left
                    uint8_t flag = chip->V[Y] >> 7;
                    chip->V[X] = chip->V[Y] << 1;
                    if (flag == 1) {
                        chip->V[0xF] = 1;
                    } else chip->V[0xF] = 0;
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
                    chip->I += chip->V[X];
                    break;
                }
                case 0x33: {
                    uint8_t VX_copy = chip->V[X];
                    chip->memory[chip->I] = VX_copy / 100;
                    chip->memory[chip->I + 1] = (VX_copy / 10) % 10;
                    chip->memory[chip->I + 2] = VX_copy % 10;
                    break;
                }
                case 0x55: {
                    for (int i = 0; i <= X; i++) {
                        chip->memory[chip->I + i] = chip->V[i];
                    }
                    chip->I = chip->I + X + 1;
                    break;
                }
                case 0x65: {
                    for (int i = 0; i <= X; i++) {
                        chip->V[i] = chip->memory[chip->I + i];
                    }
                    chip->I = chip->I + X + 1;
                    break;
                }
                case 0x29: {
                    //each digit takes 5 bytes that's why there is *5 and V[X] points to the location starting from 0x050
                    chip->I = 0x050 + chip->V[X] * 5;
                    break;
                }
                case 0x07: {
                    chip->V[X] = chip->delay_timer;
                    break;
                }
                case 0x15: {
                    chip->delay_timer = chip->V[X];
                    break;
                }
                case 0x18: {
                    chip->sound_timer = chip->V[X];
                    break;
                }
                case 0x0A: {
                    if (chip->waiting_release == 0) {
                        for (int i = 0; i <= 0xF; i++) {
                            if (chip->keys[i]) {
                                chip->pressed_key = i;
                                chip->waiting_release = 1;
                                break;
                            }
                        }
                        chip->pc -= 2;
                    } else if (chip->keys[chip->pressed_key]) {
                        chip->pc -= 2;
                    } else {
                        chip->V[X] = chip->pressed_key;
                        chip->waiting_release = 0;
                    }
                    break;
                }

                default: {
                    printf("invalid instruction: %04X \n", instruction);
                    break;
                }
            }
            break;
        }

        case 0xB: {
            chip->pc = NNN + chip->V[0];
            break;
        }

        case 0xC: {
            uint8_t random = rand() % 256; //256 since 0xFF would be one number too small
            chip->V[X] = random & NN;
            break;
        }

        case 0xE: {
            switch (NN) {
                case 0x9E: {
                    if (chip->keys[chip->V[X] & 0xF]) chip->pc += 2;
                    break;
                }
                case 0xA1: {
                    if (chip->keys[chip->V[X] & 0xF] == 0) chip->pc += 2;
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

void draw_screen(SDL_Renderer *renderer,const struct chip8 *chip) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //black
    SDL_RenderClear(renderer); // fill everything with black
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //white

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if (chip->screen[i][j] == 1) {
                SDL_Rect rect = {j * SCALE, i * SCALE, SCALE, SCALE}; // j = column (x), i = row (y)
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer); //displays the finished frame
}

void update_timers(struct chip8 *chip) {
    if (chip->delay_timer > 0) chip->delay_timer--;
    if (chip->sound_timer > 0) chip->sound_timer--;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc >= 2) {
        struct chip8 chip = {0};

        //loading font into memory
        load_font(&chip);

        //loading the rom
        if (load_rom(&chip, argv[1]) != 0) {
            printf("ROM error\n");
            return 1;
        }
        //SDL2 SETUP
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("SDL_Init error: %s\n", SDL_GetError());
            return 1;
        }

        SDL_Window *window = SDL_CreateWindow("CHIP-8",
                                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                              64 * SCALE, 32 * SCALE, 0);

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                    SDL_RENDERER_ACCELERATED);

        //main loop: one iteration = one frame (1/60 s)
        while (handle_events(&chip)) {
            Uint32 frame_start_time = SDL_GetTicks();

            for (int step = 0; step < INSTRUCTIONS_PER_FRAME; step++) {
                execute_instruction(&chip);
            }
            update_timers(&chip);
            draw_screen(renderer, &chip);

            Uint32 elapsed = SDL_GetTicks() - frame_start_time;
            if (elapsed < 16) SDL_Delay(16 - elapsed);
        }

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
    }
    printf("file address not given \n");
    return 1;
}