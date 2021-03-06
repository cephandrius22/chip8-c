#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

const int SCREEN_HEIGHT = 32;
const int SCREEN_WIDTH = 64;

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
    uint8_t screen[SCREEN_HEIGHT][SCREEN_WIDTH];
} chip8;


void dump_screen(chip8 *machine) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            printf("%d ", machine->screen[y][x]);
        }
        printf("\n");
    }
}

void dump_memory(chip8 *machine) {
    for (int i = 0; i < 4096; i+=2) {
        uint16_t opcode = ((uint16_t) machine->memory[i]) | (((uint16_t) machine->memory[i + 1]) << 8);
        printf("%02x: %04x\n", i, opcode);
    }
}

void push_stack(uint16_t *stack, uint16_t value) {
    for (int i = 15; i > 0; i--) {
        stack[i] = stack[i - 1];
    }

    stack[0] = value;

    // XXX unsure what the behavior is when the stack is full
}

uint16_t pop_stack(uint16_t *stack) {
    uint16_t ret = stack[0];

    for (int i = 0; i < 15; i++) {
        stack[i] = stack[i + 1];
    }

    return ret;
}

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

void clear_display(chip8 *machine)
{
    for (unsigned int y = 0; y < SCREEN_HEIGHT; y++) {
        for (unsigned int x = 0; x < SCREEN_WIDTH; x++) {
            machine->screen[y][x] = 0;
        }
    }
}

void execute_instruction(chip8* cpu)
{
    uint16_t opcode = ((uint16_t) cpu->memory[cpu->pc] << 8) | ((uint16_t) cpu->memory[cpu->pc + 1]);
    // opcode = (opcode << 8) | (opcode & 0xFF);
    uint8_t msb = (uint8_t)(opcode >> 8);
    uint8_t lsb = (uint8_t)(opcode & 0xFF);


    printf("pc: %04X opcode: %04X\n", cpu->pc, opcode);
    printf("msb: %02X\n", msb);
    
    cpu->pc+=2;

    switch (msb >> 4) {
    case 0x0:
        switch (opcode) {
        case 0x00E0:
            clear_display(cpu);
            break;
        case 0x00EE:
            cpu->pc = pop_stack(cpu->stack);
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
        push_stack(cpu->stack, cpu->pc);
        cpu->pc = call_location;
        break;
    }
    case 0x3:
    {
        uint8_t reg_idx = (opcode >> 8) & 0x0F;
        uint16_t kk = opcode & 0xFF;
        if (kk == cpu->registers[reg_idx]) {
            cpu->pc += 2;
        }
        break;
    }
    case 0x4:
    {
        uint8_t reg_idx = (opcode >> 8) & 0x0F;
        uint16_t kk = opcode & 0xFF;
        if (kk != cpu->registers[reg_idx]) {
            cpu->pc += 2;
        }
        break;
    }
    case 0x5:
    {
        uint8_t reg_idx_x = (opcode >> 8) & 0x0F;
        uint8_t reg_idx_y = (opcode >> 4) & 0x0F;
        if (cpu->registers[reg_idx_x] == cpu->registers[reg_idx_y]) {
            cpu->pc += 2;
        }
        break;
    }
    case 0x6:
    {
        uint8_t reg_idx = (opcode >> 8) & 0x0F;
        uint8_t kk = opcode & 0xFF;
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
        // if ((opcode & 0x0F) == 0) {
        //     cpu->registers[reg_idx_x] = cpu->registers[reg_idx_y];
        // } else {
        //     cpu->registers[reg_idx_x] |= cpu->registers[reg_idx_y];
        // }
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
            uint8_t add_overflow = cpu->registers[reg_idx_x] + cpu->registers[reg_idx_y];
            cpu->registers[reg_idx_x] = (add_overflow & 0xFF);
            break;
        }
        case 5:
            cpu->registers[0xF] = (int)(cpu->registers[reg_idx_x] > cpu->registers[reg_idx_y]);
            cpu->registers[reg_idx_x] -= cpu->registers[reg_idx_y];
            break;
        case 6:
            cpu->registers[reg_idx_x] = cpu->registers[reg_idx_y];
            cpu->registers[0xF] = (cpu->registers[reg_idx_x] & 0x1);
            cpu->registers[reg_idx_x] = cpu->registers[reg_idx_x] >> 1;
            break;
        case 7:
            cpu->registers[0xF] = (int)(cpu->registers[reg_idx_y] > cpu->registers[reg_idx_x]);
            cpu->registers[reg_idx_x] = cpu->registers[reg_idx_y] - cpu->registers[reg_idx_x];
            break;
        case 0xE:
            cpu->registers[reg_idx_x] = cpu->registers[reg_idx_y];
            cpu->registers[0xF] = (cpu->registers[reg_idx_x] & 0x80);
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
            cpu->pc += 2;
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
        cpu->registers[reg] = random & k;
        break;
    }
    case 0xD:
    {
        uint8_t x = ((opcode >> 8) & 0xF);
        uint8_t y = ((opcode >> 4) & 0xF);
        x = cpu->registers[x] % SCREEN_WIDTH;
        y = cpu->registers[y] % SCREEN_HEIGHT;
        uint8_t n = (opcode & 0xF);
        for (int i = 0; i < n; i++) {
            uint8_t byte = cpu->memory[cpu->I + i];
            // TODO: fix this, it is messy
            for (int shift = 0; shift < 8; shift++) {
                uint8_t bit = (byte >> (7 - shift)) & 1;
                uint8_t lit = (cpu->screen[y][x + shift] & 0x1) ^ bit;
                // TODO: handle wraping around display
                cpu->registers[0xF] = (uint8_t) (lit == 0 && cpu->screen[y][x + shift] > 0);
                cpu->screen[y][x + shift] = lit;
            }
            y++;
        }
        break;
    }
    case 0xE:
    {
        uint8_t x = ((opcode >> 8) & 0xF);
        /// TODO input thing
        if ((opcode & 0xFF) == 0x9E) {
            if (cpu->keys[x]) {
                cpu->pc += 2;
            }
        } else if ((opcode & 0xFF) == 0xA1) {
            if (!cpu->keys[x]) {
                cpu->pc += 2;
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
                break;
            case 0x0A:
                // wait for key press
                break;
            case 0x15:
                cpu->delay_timer = cpu->registers[x];
                break;
            case 0x18:
                cpu->sound_timer = cpu->registers[x];
                break;
            case 0x1E:
                cpu->I += cpu->registers[x];
                break;
            case 0x29:
            {
                uint8_t hex = cpu->registers[x];
                cpu->I = hex * 5; // each hex character is 5 bytes
                break; // sprite thing
            }
            case 0x33:
            {
                uint8_t first_digit = cpu->registers[x] % 10;
                uint8_t second_digit = (cpu->registers[x] / 10) % 10;
                uint8_t third_digit = (cpu->registers[x] / 100) % 10;
                cpu->memory[cpu->I] = third_digit;
                cpu->memory[cpu->I + 1] = second_digit;
                cpu->memory[cpu->I + 2] = first_digit;
                break;
            }
            case 0x55:
            {
                // use memcpy probably
                for (int i = 0; i < 16; i++) {
                    cpu->memory[cpu->I + i] = cpu->registers[i];
                }
                break;
            }
            case 0x65:
            {
                // use memcpy probably
                for (int i = 0; i < 16; i++) {
                    cpu->registers[i] = cpu->memory[cpu->I + i];
                }
                break;
            }
        }
        break;
    }
    }
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH * 15, SCREEN_HEIGHT * 15, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    printf("Renderer name: %s\n", info.name);
    printf("Texture formats: \n");
    for (uint32_t i = 0; i < info.num_texture_formats; i++) {
        printf("%s\n", SDL_GetPixelFormatName(info.texture_formats[i]));
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    chip8 machine;

    machine.pc = 0x200;
    machine.sp = 0;

    if (argc < 2) {
        printf("Please provide a rom to run.\n");
        return 0;
    }

    FILE* fp = fopen(argv[1], "rb");
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

    uint8_t image[SCREEN_WIDTH * SCREEN_HEIGHT * 4];

    for (unsigned int y = 0; y < SCREEN_HEIGHT; y++) {
        for (unsigned int x = 0; x < SCREEN_WIDTH; x++) {
            const unsigned int offset = (SCREEN_WIDTH * 4 * y) + x * 4;
            int color = 0;
            image[offset + 0] = color; // b
            image[offset + 1] = color; // g
            image[offset + 2] = color; // r
            image[offset + 3] = SDL_ALPHA_OPAQUE; // a
        }
    }

    SDL_Event event;
    bool running = true;
    while (running) {
        uint64_t start = SDL_GetPerformanceCounter();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for (int i = 0; i < 16; i++) {
            machine.keys[i] = false;
        }

        running = process_input(&machine, event);

        execute_instruction(&machine);

        // dump_screen(&machine);

        for (unsigned int y = 0; y < SCREEN_HEIGHT; y++) {
            for (unsigned int x = 0; x < SCREEN_WIDTH; x++) {
                uint8_t draw_color = machine.screen[y][x];

                const unsigned int offset = (SCREEN_WIDTH * 4 * y) + x * 4;
                image[offset + 0] = 255 * draw_color; // b
                image[offset + 1] = 255 * draw_color; // g
                image[offset + 2] = 255 * draw_color; // r
                image[offset + 3] = SDL_ALPHA_OPAQUE; // a
            }
        }

        SDL_UpdateTexture(
            texture,
            NULL,
            image,
            SCREEN_WIDTH * 4);

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);


        uint64_t end = SDL_GetPerformanceCounter();

        float elapsed_ms = (end - start) / (float) SDL_GetPerformanceFrequency() * 1000.0f;

        // cap to 60 fps
        SDL_Delay(floor(16.666f - elapsed_ms));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
