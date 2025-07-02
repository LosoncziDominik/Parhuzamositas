#include "boid.h"

SDL_Event event;

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Boid Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        printf("Can't load font: %s\n", TTF_GetError());
        return 1;
    }

    Dropdown_Menu menu1 = {
        .options = {"peaceful", "doom mode"},
        .option_count = 2,
        .selected = 1,
        .x = 50, .y = 50, .w = 170, .h = 35,
        .open = false,
        .is_active = true,
        .menuID = 1
    };

    Dropdown_Menu menu2 = {
        .options = {"boid is friendly", "boid is natural", "boid is scared"},
        .option_count = 3,
        .selected = 2,
        .x = 50, .y = 50, .w = 170, .h = 35,
        .open = false,
        .is_active = false,
        .menuID = 2
    };

    Dropdown_Menu* menus[] = { &menu1, &menu2 };
    int menu_count = sizeof(menus) / sizeof(menus[0]);
    int active_menu = 0;
       

    SDL_Color white = {255, 255, 255, 255};
    char buf[64];
    bool display = false;

    int circle_count = 0;
    int click_state = 0;
    int center_x, center_y;
    int x, y;
    int radius;
    int delay = 0;

    init_boids();
    init_player();
    init_color();

    int running = 1;

    while (running) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = 0;
            
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    menus[active_menu]->is_active = false;
                    menus[active_menu]->open = false;
                    active_menu = (active_menu - 1 + menu_count) % menu_count;
                    menus[active_menu]->open = true;
                    menus[active_menu]->is_active = true;
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    menus[active_menu]->is_active = false;
                    menus[active_menu]->open = false;
                    active_menu = (active_menu + 1) % menu_count;
                    menus[active_menu]->open = true;
                    menus[active_menu]->is_active = true;
                }
                if (event.key.keysym.sym == SDLK_TAB) {
                    menus[active_menu]->open = !menus[active_menu]->open;
                }
               if (menus[active_menu]->is_active) {
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        menus[active_menu]->open = true;
                        menus[active_menu]->selected = (menus[active_menu]->selected + 1) % menus[active_menu]->option_count;
                        settings(menus[active_menu]);
                    }
                    if (event.key.keysym.sym == SDLK_UP) {
                        menus[active_menu]->open = true;
                        menus[active_menu]->selected = (menus[active_menu]->selected - 1 + menus[active_menu]->option_count) % menus[active_menu]->option_count;
                        settings(menus[active_menu]);
                    }
                    if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                        menus[active_menu]->open = false;
                    }
                }
                if  (event.key.keysym.sym == SDLK_F1){
                    display = !display;
                }
                
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
                x = event.button.x;
                y = event.button.y;
        
                if (click_state == 0) {
                    center_x = x;
                    center_y = y;
                    click_state = 1;
                } else if (click_state == 1) {
                    int dx = x - center_x;
                    int dy = y - center_y;
                    radius = (int)sqrt(dx * dx + dy * dy);
        
                    if (circle_count < CIRCLE_AMOUNT) {
                        circle_init(center_x, center_y, radius, circle_count);
                        circle_count++;
                    }
                    click_state = 0;
                }
            }
        }
        Uint64 start = SDL_GetPerformanceCounter();
        update_boids_paralell();
        update_player();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        
        SDL_RenderClear(renderer);

        render_blood(renderer);
        render_boids_paralell();
        render_boids(renderer);
        render_player(renderer);

        for (int i = 0; i < circle_count; ++i) {
            draw_circle_outline(renderer, get_circle_x(i), get_circle_y(i), get_circle_radius(i), 200, 4);
        }

        if(display){
            display_guide(renderer, font);
        }
        
        sprintf(buf, "Score: %d", get_player_score());
        
        draw_score(renderer, buf, SCREEN_WIDTH - 150, 50, font, white);
        render_dropdown_menu(renderer, font, menus, menu_count);
        SDL_RenderPresent(renderer);

        Uint64 end = SDL_GetPerformanceCounter();

        double elapsed_ms = (end - start) * 1000.0 / SDL_GetPerformanceFrequency();

        if(delay == 50){
        printf("performace value: %.3f ms\n", elapsed_ms);
        delay = 0;
        }
        delay++;

        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
