#include <SDL2/SDL.h>
#include <stdio.h>

typedef struct _chip8 {
    uint8_t memory[4096];
    uint8_t registers[16];
    uint8_t sound_timer;
    uint8_t delay_timer;
    uint16_t pc;
    uint8_t sp;
    uint16_t stack[16];
} chip8;


void fetch_instruction(chip8 *cpu) {

}

void execute_instruction(chip8 *cpu) {
    
}

int main()
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

    FILE *fp = fopen("br8kout.ch8", "rb");
    if (fp == NULL) {
        printf("Failed to open file.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    // Read game data in the correct spot in memory (0x200 - 0xFFF)
    fread(&machine.memory[0x200], sizeof(machine.memory) - 0x200, 1, fp);
    fclose(fp);

    SDL_Event event;
    int running = 1;
    while (running) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        while (SDL_PollEvent(&event)) {
            if ((SDL_QUIT == event.type) || (SDL_KEYDOWN == event.type && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode)) {
                running = 0;
                break;
            }
        }

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
