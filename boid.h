#ifndef BOID_H
#define BOID_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define NUM_BOIDS 1000
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 700
#define MAX_SPEED 3.0
#define MIN_SPEED 1.0
#define VIEW_RADIUS 60.0
#define SEPARATION_RADIUS 15.0
#define COHESION_WEIGHT 0.05
#define ALIGNMENT_WEIGHT 0.04
#define SEPARATION_WEIGHT 0.03
#define TURNING_RATE 0.1
#define SPEED_SMOOTHING 0.5
#define PLAYER_SPEED 5.0
#define MAX_BLOOD 10000
#define MAX_OPTIONS 5
#define CIRCLE_AMOUNT 10


/**
 * Data of Boid
 */
typedef struct {
    float x, y;
    float vx, vy;
    bool friendly;
    bool alive;
    bool is_scared;
    bool is_fleeing;
    bool is_coming;
    SDL_Color skin_color;
} Boid;

/**
 * Data of Player
 */
typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    int counter;
    bool is_doomguy;
} Player;

/**
 * Data of Blood
 */
typedef struct{
    float x, y;
    bool active;
} Blood;


/**
 * Data of Menus
 */
typedef struct {
    const char* options[MAX_OPTIONS];
    int option_count;
    int selected;
    int x, y, w, h;
    bool open;
    bool is_active;
    int menuID;
} Dropdown_Menu;

/**
 * Data of Circle
 */
typedef struct Circle
{
	double x;
	double y;
	double radius;
    SDL_Color color;
} Circle;

/**
 * Data of Color
 */
typedef struct {
    SDL_Color white;
    SDL_Color green;
    SDL_Color red;
    SDL_Color yellow;
    SDL_Color current;
} Color;



/**
 * Move the boid to the given position
 */
void init_boids();

/**
 * Move the player to the starting position
 */
void init_player();

/**
 * Updates the player's behaveior
 */
void update_player();

/**
 * Updates the boid's behavior
 */
void update_boids();

/**
 * Renders the boids
 */
void render_boids(SDL_Renderer* renderer);

/**
 * Renders the player
 */
void render_player(SDL_Renderer* renderer);

/**
 * Renders the blood droplets
 */
void render_blood(SDL_Renderer* renderer);

/**
 * Rendsers the dropdown menu
 */
void render_dropdown_menu(SDL_Renderer* renderer, TTF_Font* font, Dropdown_Menu* ddm[], int menu_count);

/**
 * If an object passes the screen length it will come out of the other side
 */
void warp(float* x, float* y);

/**
 * Draws the player's score
 */
void draw_score(SDL_Renderer* renderer, const char* score, int x, int y, TTF_Font* font, SDL_Color color);

/**
 * Counts how many boids have been killed
 */
int score_counter();

/**
 * Getter function for the player's score
 */
int get_player_score();

/**
 * This function applies the settings from the options menu
 */
void settings(Dropdown_Menu* ddm);

/**
 * This function displays the guide.txt on the screen
 */
void display_guide(SDL_Renderer* renderer, TTF_Font* font);

/**
 * Draws the outline of a circle, it's position and radius is given by your left click
 */
void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int radius, int segments, int dash_ratio);

/**
 * Sets the circle's values
 */
void circle_init(int center_x, int center_y, int radius, int circle_count);

/**
 * Returnes the circle's x value
 */
int get_circle_x(int index);

/**
 * Returnes the circle's y value
 */
int get_circle_y(int index);

/**
 * Returnes the circle's radius
 */
int get_circle_radius(int index);

/**
 * Sets the boid's color based of it's .is_scared and .friendly value
 */
void set_skin_color(int index);

/**
 * Sets the color's values
 */
void init_color();

/**********************************************************************************************
*                          PHTREADHEZ SZÜKSÉGES FÜGGVÉNYEK                                    *
***********************************************************************************************/


 /**
 * Sets the color's values
 */
 typedef struct {
    int start;
    int end;
} ThreadRange;


void update_boids_parallel();

void update_single_boid(int i);

void* update_boid_range(void* arg);

#endif
