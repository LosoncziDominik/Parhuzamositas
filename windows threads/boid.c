#include "boid.h"


Boid boids[NUM_BOIDS];
Player player;
Blood blood_drops[MAX_BLOOD];
int bloodCount = 0;
Circle circles[CIRCLE_AMOUNT];
Color color;
CRITICAL_SECTION crit;
HANDLE bloodMutex;
SDL_Point points[NUM_BOIDS][4];


void init_boids() {
    for (int i = 0; i < NUM_BOIDS; i++) {
        boids[i].x = rand() % SCREEN_WIDTH;
        boids[i].y = rand() % SCREEN_HEIGHT;
        boids[i].vx = ((rand() % 200) / 100.0f - 1) * MAX_SPEED;
        boids[i].vy = ((rand() % 200) / 100.0f - 1) * MAX_SPEED;
        boids[i].alive = true;
        boids[i].is_scared = true;
        boids[i].friendly = false;
        boids[i].is_fleeing = false;
        boids[i].is_coming = false;
        boids[i].skin_color.r = 255;
        boids[i].skin_color.g = 255;
        boids[i].skin_color.b = 255;
    }
}

void init_player(){
    player.x = SCREEN_WIDTH / 2;
    player.y = SCREEN_HEIGHT * 3 / 4;
    player.vx = 0;
    player.vy = 0;
    player.is_doomguy = true;
}

DWORD WINAPI UpdateThreadFunction(LPVOID lpParam) {
    int thread_id = *((int*)lpParam);
    int boids_per_thread = NUM_BOIDS / NUM_THREADS;
    int start = thread_id * boids_per_thread;
    int end = (thread_id == NUM_THREADS - 1) ? NUM_BOIDS : start + boids_per_thread;

    update_boids_range(start, end);
    return 0;
}

DWORD WINAPI RenderThreadFunction(LPVOID lpParam) {
    int thread_id = *((int*)lpParam);
    int boids_per_thread = NUM_BOIDS / NUM_THREADS;
    int start = thread_id * boids_per_thread;
    int end = (thread_id == NUM_THREADS - 1) ? NUM_BOIDS : start + boids_per_thread;

    set_render_state(start, end);

    return 0;
}

void update_boids_paralell() {
    HANDLE threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    bloodMutex = CreateMutex(NULL, FALSE, NULL);
    InitializeCriticalSection(&crit);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        threads[i] = CreateThread(NULL, 0, UpdateThreadFunction, &thread_ids[i], 0, NULL);
        if (threads[i] == NULL) {
            printf("Couldn't make thread (%d)\n", i);
            return;
        }
    }

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
    CloseHandle(bloodMutex);

    for (int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(threads[i]);
    }
}

void render_boids_paralell() {
    HANDLE threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        threads[i] = CreateThread(NULL, 0, RenderThreadFunction, &thread_ids[i], 0, NULL);
        if (threads[i] == NULL) {
            printf("Couldn't make thread (%d)\n", i);
            return;
        }
    }

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
    CloseHandle(bloodMutex);

    for (int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(threads[i]);
    }
}


void update_boids_range(int start, int end) {
    for (int i = start; i < end; i++) {
        float avgX = 0, avgY = 0;
        float avgVX = 0, avgVY = 0;
        float sepX = 0, sepY = 0;
        int count = 0;
        boids[i].is_fleeing = false;
        boids[i].is_coming = false;
        boids[i].skin_color = color.white;

        for (int j = 0; j < NUM_BOIDS; j++) {
            if (i != j) {
                float dx = boids[j].x - boids[i].x;
                float dy = boids[j].y - boids[i].y;
                float dist = sqrt(dx * dx + dy * dy);

                float pdx = player.x - boids[i].x;
                float pdy = player.y - boids[i].y;
                float pdist = sqrt(pdx * pdx + pdy * pdy);

                if (dist < VIEW_RADIUS) {
                    avgX += boids[j].x;
                    avgY += boids[j].y;
                    avgVX += boids[j].vx;
                    avgVY += boids[j].vy;
                    count++;
                }

                if (dist < SEPARATION_RADIUS) {
                    sepX -= dx;
                    sepY -= dy;
                }

                if (pdist < VIEW_RADIUS){
                    if(boids[i].is_scared){
                        boids[i].is_fleeing = true;
                        set_skin_color(i);
                        float runx = -pdx / pdist;
                        float runy = -pdy / pdist;
                        boids[i].vx = runx * 1.5;
                        boids[i].vy = runy * 1.5;
                    } else if (boids[i].friendly){
                        boids[i].is_coming = true;
                        set_skin_color(i);
                        float runx = pdx / pdist;
                        float runy = pdy / pdist;
                        boids[i].vx = runx * 1.5;
                        boids[i].vy = runy * 1.5;
                    }
                }
            }
        }

        for (int k = 0; k < CIRCLE_AMOUNT; k++) {
            float dx = circles[k].x - boids[i].x;
            float dy = circles[k].y - boids[i].y;
            float dist = sqrt(dx * dx + dy * dy);

            float avoid_radius = circles[k].radius + 10;
            if (dist < avoid_radius && dist > 5.0f) {
                float vx = boids[i].vx;
                float vy = boids[i].vy;
                float speed = sqrt(vx * vx + vy * vy);
                if (speed < 0.001f) speed = 0.001f;
            
                float sideX = vy;
                float sideY = -vx;
            
                float scale = (1.0f - dist / avoid_radius) * 0.8f;
                boids[i].vx += sideX * scale;
                boids[i].vy += sideY * scale;
            }
            
            if (dist < circles[k].radius) {
                float dx = boids[i].x - circles[k].x;
                float dy = boids[i].y - circles[k].y;
                float pullout_strength = 0.02f;
            
                boids[i].vx += dx * pullout_strength;
                boids[i].vy += dy * pullout_strength;
            }
        }

        if (count > 0) {
            avgX /= count;
            avgY /= count;
            avgVX /= count;
            avgVY /= count;

            float targetVX = (avgX - boids[i].x) * COHESION_WEIGHT;
            float targetVY = (avgY - boids[i].y) * COHESION_WEIGHT;

            boids[i].vx += (targetVX - boids[i].vx) * TURNING_RATE;
            boids[i].vy += (targetVY - boids[i].vy) * TURNING_RATE;

            boids[i].vx += (avgVX - boids[i].vx) * ALIGNMENT_WEIGHT;
            boids[i].vy += (avgVY - boids[i].vy) * ALIGNMENT_WEIGHT;
        }

        boids[i].vx += sepX * SEPARATION_WEIGHT;
        boids[i].vy += sepY * SEPARATION_WEIGHT;

        float speed = sqrt(boids[i].vx * boids[i].vx + boids[i].vy * boids[i].vy);
        float targetSpeed = MAX_SPEED * 0.7 + (rand() % 30) * 0.01;

        if (speed < targetSpeed) {
            boids[i].vx *= (1 + SPEED_SMOOTHING);
            boids[i].vy *= (1 + SPEED_SMOOTHING);
        } else if (speed > targetSpeed) {
            boids[i].vx *= (1 - SPEED_SMOOTHING);
            boids[i].vy *= (1 - SPEED_SMOOTHING);
        }

        boids[i].x += boids[i].vx;
        boids[i].y += boids[i].vy;

        warp(&boids[i].x, &boids[i].y);
        
        if (player.is_doomguy) {
            if (fabs(boids[i].x - player.x ) < 15 && fabs(boids[i].y - player.y ) < 15){
                boids[i].alive = false;
                WaitForSingleObject(bloodMutex, INFINITE);
                    if(bloodCount < MAX_BLOOD){
                        blood_drops[bloodCount].x = boids[i].x + rand() % 10 - 3;
                        blood_drops[bloodCount].y = boids[i].y + rand() % 10 - 3;
                        blood_drops[bloodCount].active = true;
                        bloodCount++;
                    }
                ReleaseMutex(bloodMutex);
            }            
        }
    }
}


void update_player() {
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    float vx = 0, vy = 0;

    if (keystates[SDL_SCANCODE_W]) vy -= 1;
    if (keystates[SDL_SCANCODE_S]) vy += 1;
    if (keystates[SDL_SCANCODE_A]) vx -= 1;
    if (keystates[SDL_SCANCODE_D]) vx += 1;

    float norm = sqrt(vx * vx + vy * vy);
    if (norm > 0) {
        vx = (vx / norm) * PLAYER_SPEED;
        vy = (vy / norm) * PLAYER_SPEED;

        player.vx += (vx - player.vx) * TURNING_RATE;
        player.vy += (vy - player.vy) * TURNING_RATE;

        player.x += vx;
        player.y += vy;

        player.angle = atan2(player.vy, player.vx);
    }

    warp(&player.x, &player.y);

    player.counter = score_counter();
}

void set_render_state(int start, int end){
    
    for (int i = start; i < end; i++)
    {
        if(boids[i].alive == true){

            float angle = atan2(boids[i].vy, boids[i].vx);
    
            points[i][0].x = (int)(boids[i].x + cos(angle) * 8);
            points[i][0].y = (int)(boids[i].y + sin(angle) * 8);
            points[i][1].x = (int)(boids[i].x + cos(angle + 2.5) * 5);
            points[i][1].y = (int)(boids[i].y + sin(angle + 2.5) * 5);
            points[i][2].x = (int)(boids[i].x + cos(angle - 2.5) * 5);
            points[i][2].y = (int)(boids[i].y + sin(angle - 2.5) * 5);
            points[i][3] = points[i][0];
        }
    }
}

void render_boids(SDL_Renderer* renderer) {
    
    for (int i = 0; i < NUM_BOIDS; i++)
    {
        if (boids[i].alive ==  true)
        {
            SDL_SetRenderDrawColor(renderer, boids[i].skin_color.r, boids[i].skin_color.g, boids[i].skin_color.b, 255);
            SDL_RenderDrawLines(renderer, points[i], 4);
        }
    }
}

void render_player(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    player.angle = atan2(player.vy, player.vx);

    SDL_Point p1 = { (int)(player.x + cos(player.angle) * 8), (int)(player.y + sin(player.angle) * 8) };
    SDL_Point p2 = { (int)(player.x + cos(player.angle + 2.5) * 5), (int)(player.y + sin(player.angle + 2.5) * 5) };
    SDL_Point p3 = { (int)(player.x + cos(player.angle - 2.5) * 5), (int)(player.y + sin(player.angle - 2.5) * 5) };

    SDL_Point triangle[4] = { p1, p2, p3, p1 };
    SDL_RenderDrawLines(renderer, triangle, 4);
}

void warp (float* x, float* y){
    if (*x < 0) *x += SCREEN_WIDTH;
    if (*x >= SCREEN_WIDTH) *x -= SCREEN_WIDTH;
    if (*y < 0) *y += SCREEN_HEIGHT;
    if (*y >= SCREEN_HEIGHT) *y -= SCREEN_HEIGHT;
}

void draw_score(SDL_Renderer* renderer, const char* score, int x, int y, TTF_Font* font, SDL_Color color){
    SDL_Surface* surface = TTF_RenderText_Blended(font, score, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstrect = { x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int score_counter(){
    int counter = 0;

    for(int i = 0; i < NUM_BOIDS; i++) if(!boids[i].alive) counter++;

    return counter;
}

void render_blood(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 180);

    for (int i = 0; i < bloodCount; i++)
    {
        if(blood_drops[i].active){
            for (int j = 0; j < 10; j++)
            {
                SDL_RenderDrawPoint(renderer, (int)(blood_drops[i].x), (int)(blood_drops[i].y));
            }
        }
    }
}

int get_player_score(){
    return player.counter;
}

void render_dropdown_menu(SDL_Renderer* renderer, TTF_Font* font, Dropdown_Menu* ddm[], int menu_count) {
    SDL_Color white = {255, 255, 255, 255};

    for (int i = 0; i < menu_count; i++) {
        if (!ddm[i]) continue;

        int x = ddm[i]->x + (i * 170);
        int y = ddm[i]->y;
        int w = ddm[i]->w;
        int h = ddm[i]->h;

        SDL_Rect menu = {x, y, w, h};

        if(ddm[i]->is_active){
            SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
        }
        else{
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        }
        
        SDL_RenderFillRect(renderer, &menu);
        SDL_RenderDrawRect(renderer, &menu);

        SDL_Surface* surface = TTF_RenderText_Blended(font, ddm[i]->options[ddm[i]->selected], white);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect text_rect = {x + 5, y + 5, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &text_rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        if (ddm[i]->open) {
            for (int j = 0; j < ddm[i]->option_count; j++) {
                SDL_Rect opt = {x, y + h * (j + 1), w, h};
                SDL_SetRenderDrawColor(renderer, (j == ddm[i]->selected ? 80 : 30), 30, 30, 255);
                SDL_RenderFillRect(renderer, &opt);
                SDL_RenderDrawRect(renderer, &opt);

                surface = TTF_RenderText_Blended(font, ddm[i]->options[j], white);
                texture = SDL_CreateTextureFromSurface(renderer, surface);
                text_rect = (SDL_Rect){x + 5, opt.y + 5, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &text_rect);
                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            }
        }
    }
}

void settings(Dropdown_Menu* ddm){
    if(ddm->menuID == 1){
        if(ddm->selected == 0){
            player.is_doomguy = false;
            }
            else{
            player.is_doomguy = true;
            }
    }
    else if (ddm->menuID == 2){
        if(ddm->selected == 0){
            for (int i = 0; i < NUM_BOIDS; i++)
            {
                boids[i].is_scared = false;
                boids[i].friendly = true;
            }
            

        }
        else if (ddm->selected == 1){
            for (int i = 0; i < NUM_BOIDS; i++)
            {
                boids[i].is_scared = false;
                boids[i].friendly = false;
            }
        }
        else{
            for (int i = 0; i < NUM_BOIDS; i++)
            {
                boids[i].is_scared = true;
                boids[i].friendly = false;
            }
        }
    }
}

void display_guide(SDL_Renderer* renderer, TTF_Font* font) {
    FILE* file = fopen("guide.txt", "r");
    if (!file) {
        perror("can't open guide.txt");
        return;
    }

    SDL_Color white = {0, 0, 0, 255};
    SDL_Color bg = {255, 255, 255, 150};

    char line[256];
    int y = 100;

    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        SDL_Surface* surface = TTF_RenderText_Blended(font, line, white);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect text_rect = {40, y, surface->w, surface->h};

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
        SDL_RenderFillRect(renderer, &text_rect);

        SDL_RenderCopy(renderer, texture, NULL, &text_rect);
        y += surface->h;

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    fclose(file);
}

void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int radius, int segments, int dash_ratio) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    double angle_step = 2 * 3.14 / segments;
    for (int i = 0; i < segments; i += dash_ratio * 2) {
        for (int j = 0; j < dash_ratio && (i + j + 1) < segments; ++j) {
            double angle1 = (i + j) * angle_step;
            double angle2 = (i + j + 1) * angle_step;

            int x1 = cx + (int)(radius * cos(angle1));
            int y1 = cy + (int)(radius * sin(angle1));
            int x2 = cx + (int)(radius * cos(angle2));
            int y2 = cy + (int)(radius * sin(angle2));

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}

void circle_init(int center_x, int center_y, int radius, int circle_count){

    circles[circle_count].x = (double)center_x;
    circles[circle_count].y = (double)center_y;
    if(radius > 100) radius = 100;
    circles[circle_count].radius = (double)radius;
    circles[circle_count].color = color.white;
}

int get_circle_x(int index){
    return circles[index].x;
}

int get_circle_y(int index){
    return circles[index].y;
}

int get_circle_radius(int index){
    return circles[index].radius;
}

void set_skin_color(int index){
    if(boids[index].is_fleeing){
        boids[index].skin_color = color.yellow;
    } 
    else if (boids[index].is_coming){
        boids[index].skin_color = color.green;
    } 
    else boids[index].skin_color = color.white;

    
}

void init_color(){
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color green  = {0,   255,   0, 255};
    SDL_Color red    = {255,   0,   0, 255};
    SDL_Color yellow = {255, 255,   0, 255};

    color.white  = white;
    color.green  = green;
    color.red    = red;
    color.yellow = yellow;
}


