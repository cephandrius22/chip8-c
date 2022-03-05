#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

enum key {
    ONE = 0,
    TWO,
    THREE,
    Q,
    W,
    E,
    A,
    S,
    D,
    Z,
    X,
    C
};

typedef struct _chip8 {
    uint8_t memory[4096];
    uint8_t registers[16];
    uint16_t I;
    uint8_t sound_timer;
    uint8_t delay_timer;
    uint16_t pc;
    uint8_t sp;
    uint16_t stack[16];
    bool keys[12];
} chip8;


bool process_input(chip8 *machine, SDL_Event event)
{
    while (SDL_PollEvent(&event)) {
        if ((SDL_QUIT == event.type) || (SDL_KEYDOWN == event.type && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode)) {
            return false;
        }

        enum key k;
        switch (event.type) {
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_1:
                        k = ONE;
                        break;
                    case SDLK_2:
                        k = TWO;
                        break;
                    case SDLK_3:
                        k = THREE;
                        break;
                    case SDLK_q:
                        k = Q;
                        break;
                    case SDLK_w:
                        k = W;
                        break;
                    case SDLK_e:
                        k = E;
                        break;
                    case SDLK_a:
                        k = A;
                        break;
                    case SDLK_s:
                        k = S;
                        break;
                    case SDLK_d:
                        k = D;
                        break;
                    case SDLK_z:
                        k = Z;
                        break;
                    case SDLK_x:
                        k = X;
                        break;
                    case SDLK_c:
                        k = C;
                        break;
                }
                machine->keys[k] = true;
                break;
        }
    }

    return true;
}

void fetch_instruction(chip8* cpu)
{
}

void clear_display()
{

}

void execute_instruction(chip8* cpu)
{
    uint16_t opcode = (uint16_t)cpu->memory[cpu->sp];
    uint8_t msb = (uint8_t)(opcode > 8);
    uint8_t lsb = (uint8_t)(opcode & 0xFF);

    switch (msb >> 4) {
    case 0x0:
        switch (opcode) {
        case 0x00E0:
            clear_display();
            break;
        case 0x00EE:
            // return from subroutine
            break;
        }
        break;
    case 0x1:
    {
        uint16_t jump_location = (opcode & 0x0FFF);
        cpu->pc = jump_location;
        break;
    }
    case 0x2:
    {
        uint16_t call_location = (opcode & 0x0FFF);
        cpu->stack[0] = cpu->pc; // XXX
        cpu->sp = call_location;
        break;
    }
    case 0x3:
    {
        uint8_t reg_idx = (opcode >> 8) & 0x0F;
        uint16_t kk = opcode & 0xFF;
        if (kk == cpu->registers[reg_idx]) {
            cpu->pc += 1;
        }
        break;
    }
    case 0x4:
    {
        uint8_t reg_idx = (opcode >> 8) & 0x0F;
        uint16_t kk = opcode & 0xFF;
        if (kk != cpu->registers[reg_idx]) {
            cpu->pc += 1;
        }
        break;
    }
    case 0x5:
    {
        uint8_t reg_idx_x = (opcode >> 8) & 0x0F;
        uint8_t reg_idx_y = (opcode >> 4) & 0x0F;
        if (cpu->registers[reg_idx_x] == cpu->registers[reg_idx_y]) {
            cpu->pc += 1;
        }
        break;
    }
    case 0x6:
    {
        uint8_t reg_idx = (opcode >> 8) & 0x0F;
        uint16_t kk = opcode & 0xFF;
        cpu->registers[reg_idx] = kk;
        break;
    }
    case 0x7:
    {
        uint8_t reg_idx = (opcode >> 8) & 0x0F;
        uint16_t kk = opcode & 0xFF;
        cpu->registers[reg_idx] += kk;
        break;
    }
    case 0x8:
    {
        uint8_t reg_idx_x = (opcode >> 8) & 0x0F;
        uint8_t reg_idx_y = (opcode >> 4) & 0x0F;
        if ((opcode & 0x0F) == 0) {
            cpu->registers[reg_idx_x] = cpu->registers[reg_idx_y];
        } else {
            cpu->registers[reg_idx_x] |= cpu->registers[reg_idx_y];
        }
        switch (opcode & 0xF) {
        case 0:
            cpu->registers[reg_idx_x] = cpu->registers[reg_idx_y];
            break;
        case 1:
            cpu->registers[reg_idx_x] |= cpu->registers[reg_idx_y];
            break;
        case 2:
            cpu->registers[reg_idx_x] &= cpu->registers[reg_idx_y];
            break;
        case 3:
            cpu->registers[reg_idx_x] ^= cpu->registers[reg_idx_y];
            break;
        case 4:
        {
            uint16_t add = cpu->registers[reg_idx_x] + cpu->registers[reg_idx_y];
            if (add > 255) {
                cpu->registers[0xF] = 1;
            } else {
                cpu->registers[0xF] = 0;
            }
            cpu->registers[reg_idx_x] = (uint8_t)(add & 0xFF);
            break;
        }
        case 5:
            cpu->registers[0xF] = (int)(cpu->registers[reg_idx_x] > cpu->registers[reg_idx_y]);
            cpu->registers[reg_idx_x] -= cpu->registers[reg_idx_y];
            break;
        case 6:
            cpu->registers[0xF] = (cpu->registers[reg_idx_x] & 0x1);
            cpu->registers[reg_idx_x] = cpu->registers[reg_idx_x] >> 1;
            break;
        case 7:
            cpu->registers[0xF] = (int)(cpu->registers[reg_idx_y] > cpu->registers[reg_idx_x]);
            cpu->registers[reg_idx_y] -= cpu->registers[reg_idx_x];
            break;
        case 14:
            cpu->registers[0xF] = ((cpu->registers[reg_idx_x] >> 7) & 0x1);
            cpu->registers[reg_idx_x] = cpu->registers[reg_idx_x] << 1;
            break;
        }
        break;
    }
    case 0x9:
    {
        uint8_t reg_idx_x = (opcode >> 8) & 0x0F;
        uint8_t reg_idx_y = (opcode >> 4) & 0x0F;
        if (cpu->registers[reg_idx_x] != cpu->registers[reg_idx_y]) {
            cpu->pc += 1;
        }
        break;
    }
    case 0xA:
    {
        uint16_t n = (opcode & 0x0FFF);
        cpu->I = n;
        break;
    }
    case 0xB:
    {
        uint16_t n = (opcode & 0x0FFF);
        cpu->pc = n += cpu->registers[0x0];
        break;
    }
    case 0xC:
    {
        uint8_t k = (opcode & 0xFF);
        uint8_t reg = ((opcode >> 8) & 0xF);
        uint8_t random = rand() % (255 + 1);
        cpu->registers[reg] &= random;
        break;
    }
    case 0xD:
    {
        uint8_t x = ((opcode >> 8) & 0xF);
        uint8_t y = ((opcode >> 4) & 0xF);
        uint8_t n = (opcode & 0xF);
        // TODO draw stuff
        break;
    }
    case 0xE:
    {
        uint8_t x = ((opcode >> 8) & 0xF);
        /// TODO input thing
        if ((opcode & 0xFF) == 0x9E) {
            if (cpu->keys[x]) {
                cpu->pc += 1;
            }
        } else if ((opcode & 0xFF) == 0xA1) {
            if (!cpu->keys[x]) {
                cpu->pc += 1;
            }
        }
        break;
    }
    case 0xF:
    {
        uint8_t x = ((opcode >> 8) & 0xF);
        uint8_t code = ((opcode) & 0xFF);
        switch (code) {
            case 0x07:
                cpu->registers[x] = cpu->delay_timer;
            case 0x0A:
                // wait for key press
            case 0x15:
                cpu->delay_timer = cpu->registers[x];
            case 0x18:
                cpu->sound_timer = cpu->registers[x];
            case 0x1E:
                cpu->I += cpu->registers[x];
            case 0x29:
                break; // sprite thing
            case 0x33:
            {
                uint8_t first_digit = cpu->registers[x] % 10;
                uint8_t second_digit = (cpu->registers[x] / 10) % 10;
                uint8_t third_digit = (cpu->registers[x] / 100) % 10;
                cpu->memory[cpu->I] = third_digit;
                cpu->memory[cpu->I + 1] = second_digit;
                cpu->memory[cpu->I + 2] = first_digit;
            }
            case 0x55:
            {
                // use memcpy probably
                for (int i = 0; i < 16; i++) {
                    cpu->memory[cpu->I + i] = cpu->registers[i];
                }
            }
            case 0x65:
            {
                // use memcpy probably
                for (int i = 0; i < 16; i++) {
                    cpu->registers[i] = cpu->memory[cpu->I + i];
                }
            }
        }
        break;
    }
    }

    cpu->pc++;
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        600, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    printf("Renderer name: %s\n", info.name);
    printf("Texture formats: \n");
    for (uint32_t i = 0; i < info.num_texture_formats; i++) {
        printf("%s\n", SDL_GetPixelFormatName(info.texture_formats[i]));
    }

    const unsigned int tex_width = 64;
    const unsigned int tex_height = 32;
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, tex_width, tex_height);

    uint8_t pixels[tex_width * tex_height * 4];

    chip8 machine;
    machine.pc = 0x200;
    machine.sp = 0;

    FILE* fp = fopen("br8kout.ch8", "rb");
    if (fp == NULL) {
        printf("Failed to open file.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    uint8_t hex_sprites[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40, 
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80
    };

    memcpy(&machine.memory, hex_sprites, sizeof(hex_sprites));

    // Read game data in the correct spot in memory (0x200 - 0xFFF)
    fread(&machine.memory[0x200], sizeof(machine.memory) - 0x200, 1, fp);
    fclose(fp);

    SDL_Event event;
    bool running = true;
    while (running) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for (int i = 0; i < 16; i++) {
            machine.keys[i] = false;
        }

        running = process_input(&machine, event);

        execute_instruction(&machine);

        for (unsigned int tex_y = 0; tex_y < tex_height; tex_y++) {
            for (unsigned int tex_x = 0; tex_x < tex_width; tex_x++) {
                const unsigned int x = rand() % tex_width;
                const unsigned int y = rand() % tex_height;

                const unsigned int offset = (tex_width * 4 * y) + x * 4;
                pixels[offset + 0] = rand() % 256; // b
                pixels[offset + 1] = rand() % 256; // g
                pixels[offset + 2] = rand() % 256; // r
                pixels[offset + 3] = SDL_ALPHA_OPAQUE; // a
            }
        }

        SDL_UpdateTexture(
            texture,
            NULL,
            pixels,
            tex_width * 4);

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
