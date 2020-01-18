#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Thank you to Lawrence Buckingham and the CAB202 team for some of the
//code that has been used below from the ZDJ examples.

#define MAX_BLOCKS (200)
#define MAX_COLUMNS (screen_width() / (7 + 2))
#define MAX_ROWS (4)

//Global ints for use
int lives = 10;
int score = 0;
int start_time;
int minutes;
int seconds;

//Global bools for use
bool game_over = false;
bool new_game = true;
bool gravity = true;
bool treasure_pause = false;

//Sprite declarations
sprite_id knee_sliding_hero;
sprite_id safe_platform;
sprite_id dangerous_platform;
sprite_id shiny_treasure;
sprite_id platforms[MAX_BLOCKS][MAX_BLOCKS];

//Sprite images
static char * safe =
"======="
"=======";

static char * dangerous = 
"xxxxxxx"
"xxxxxxx";

static char * hero = 
" O "
"/|\\"
"/ \\";

static char * treasure = 
",___,"
"|___|";

void game_info(){
    int width = screen_width() / 4;
	draw_formatted(5, 2, "Lives: %d", lives);
	draw_formatted(width, 2, "Score: %d", score);
	draw_formatted(width * 3, 2, "Student #: n10282653");
    
    //Arithmetic for calculating game time
    int current_time = get_current_time() - start_time;
    seconds = current_time % 60;
    minutes = seconds / 60;
	draw_formatted(width * 2, 2, "Time: %02d:%02d", minutes, seconds);
}

void draw_border(){

    //Variable names to make the border drawing easier to understand
    int left = 0;
    int top = 0;
    int right = screen_width() - 1;

    //Draws the actual border to the edge of terminal window
    draw_line(left, top, right, top, '~');

    //Draws the line to create a box for the time, 
    //lives and score functionality
    draw_line(left, top + 4, right, top + 4, '~');
}

void draw_platforms()
{
    for(int row = 0; row < MAX_ROWS; row++)
    {
        for(int column = 0; column < MAX_COLUMNS; column++)
        {
            sprite_draw (platforms[column][row]);
        }
    }
}

void respawn_hero(){

    int hx = 3;
    int hy = 7;

    char *respawn_animation[4] = 
    {
    "   "
    " |  "
    "    ",
    "   "
    "/|\\"
    "    ",
    "   "
    "/|\\"
    "/ \\",
    " O "
    "/|\\"
    "/ \\"
    };

    knee_sliding_hero = sprite_create(hx, hy, 3, 3, hero);

    //Iterate over the animation array to respawn the hero
    for (int i = 0; i < 4; i++)
    {
        clear_screen();
        draw_border();
        game_info();
        draw_platforms();
        sprite_set_image(knee_sliding_hero, respawn_animation[i]);
        sprite_draw(knee_sliding_hero);
        show_screen();
        timer_pause(250);
    }

}

void respawn_treasure(){

    int tx = 1 + rand() % (screen_width() - 3);
    int ty = (screen_height() - 3);

    shiny_treasure = sprite_create(tx, ty, 5, 2, treasure);
    sprite_draw(shiny_treasure);

    sprite_turn_to(shiny_treasure, 0.25, 0.0 );
	int angle = 180;
	sprite_turn(shiny_treasure, angle);
}

bool sprites_collided(sprite_id s1, sprite_id s2)
{
    int l1 = round( sprite_x( s1 ) );
    int l2 = round( sprite_x( s2 ) );
    int t1 = round( sprite_y( s1 ) );
    int t2 = round( sprite_y( s2 ) );
    int r1 = l1 + sprite_width( s1 ) - 1;
    int r2 = l2 + sprite_width( s2 ) - 1;
    int b1 = t1 + sprite_height( s1 );
    int b2 = t2 + sprite_height( s2 );

    if ( l1 > r2 )
        return false;
    if ( l2 > r1 )
        return false;
    if ( t1 > b2 )
        return false;
    if ( t2 > b1 )
        return false;

    return true;
}

sprite_id sprites_collide_any()
{
    sprite_id result = NULL;

    for(int row = 0; row < MAX_ROWS; row++)
    {
        for(int column = 0; column < MAX_COLUMNS; column++)
        {
            if(sprites_collided (knee_sliding_hero, platforms[column][row]))
            {
                result = platforms[column][row];

                if(result->bitmap == safe)
                {   
                    gravity = !gravity;
                    score += 1;
                }
                else if (result->bitmap == dangerous)
                {   
                    sprite_destroy(knee_sliding_hero);
                    respawn_hero();
                    lives -= 1;
                }
                break;
            }
        }
    }
    return result;
}

void setup(){

    //Set the initial timer
    start_time = get_current_time();

    //Hero location
    int hx = 3;
    int hy = 7;

    //Treasure location
    int tx = 1 + rand() % (screen_width() - 3);
    int ty = (screen_height() - 2);

    //Initialise all sprites
    shiny_treasure = sprite_create(tx, ty, 5, 2, treasure);
    knee_sliding_hero = sprite_create(hx, hy, 3, 3, hero);

    //Draw all sprites for the first time
    sprite_draw(knee_sliding_hero);
    sprite_draw(shiny_treasure);

    //Initiate treasure movement
    sprite_turn_to( shiny_treasure, 0.25, 0.0 );
	int angle = 180;
	sprite_turn( shiny_treasure, angle );
    
}

void create_platforms()
{
    //Platform parameters
    int platform_width = 7;
    int platform_height = 2;
    int top_screen_padding = 10;
    int left_screen_padding = 1;
    int x_platform_spacing = platform_width + 2;
    int y_platform_spacing = 
    screen_height() / (platform_height + 
    sprite_height(knee_sliding_hero));

    static char * platform_space =
    "       "
    "       ";

    //Create a 2D array containing the platforms and guarantee the starting
    //block is safe and there is at least 1 safe platform per column
    for(int row = 0; row < MAX_ROWS; row++)
    {
        for(int column = 0; column < MAX_COLUMNS; column++)
        {   
            int randomiser = rand() % 100 + 1;
            
            platforms[column][row] = sprite_create
            (
                column * x_platform_spacing + left_screen_padding, 
                row * y_platform_spacing + top_screen_padding,
                platform_width, platform_height, safe
            );

            if(column % 2 == 1 && (row == 0 || row >= 2)
            && randomiser % 3 != 0)
            {
                platforms[column][row] = sprite_create
                (
                column * x_platform_spacing + left_screen_padding, 
                row * y_platform_spacing + top_screen_padding,
                platform_width, platform_height, platform_space
                );      
            }
            else if (column % 2 == 0 && (row == 1 || row >= 3) 
            && randomiser % 4 != 0)
            {
                platforms[column][row] = sprite_create
                (
                column * x_platform_spacing + left_screen_padding, 
                row * y_platform_spacing + top_screen_padding,
                platform_width, platform_height, platform_space
                );    
            }
            else if(column % 2 == 1 && (row == 0 || row >= 2) 
            && randomiser % 5 != 0)
            {
                platforms[column][row] = sprite_create
                (
                column * x_platform_spacing + left_screen_padding, 
                row * y_platform_spacing + top_screen_padding,
                platform_width, platform_height, dangerous
                );      
            }

            else if (column % 2 == 0 && (row == 1 || row >= 3) 
            && randomiser % 6 != 0)
            {
                platforms[column][row] = sprite_create
                (
                column * x_platform_spacing + left_screen_padding, 
                row * y_platform_spacing + top_screen_padding,
                platform_width, platform_height, dangerous
                );    
            }

            else if ((row == 2 || row == 3) && randomiser % 3 != 0)
            {
                platforms[column][row] = sprite_create
                (
                column * x_platform_spacing + left_screen_padding, 
                row * y_platform_spacing + top_screen_padding,
                platform_width, platform_height, safe
                );    
            }

            else if((row == 2 || row == 3) && randomiser % 2 == 0)
            {
                platforms[column][row] = sprite_create
                (
                column * x_platform_spacing + left_screen_padding, 
                row * y_platform_spacing + top_screen_padding,
                platform_width, platform_height, dangerous
                );      
            }
        }        
    }
}

void game_finished()
{
    game_over = true;
    new_game = false;
    clear_screen();

    int key = get_char();

    int msg_x = (screen_width()/ 2 - 22);
    int msg_y = (screen_height()/ 2);

    draw_formatted(msg_x, msg_y,
    "                GAME OVER!");
    
    draw_formatted(msg_x, msg_y + 1, 
    "You were alive for %d minutes and %d seconds.", minutes, seconds);

    draw_formatted(msg_x, msg_y + 2,
    "      You scored a total of %d points!", score);

    draw_formatted(msg_x, msg_y + 3,
    "Press r to play a new game or press q to quit.");

    show_screen();

    timer_pause(1000);

    if(key == 'r')
    {   
        lives = 10;
        new_game = true;
        game_over = false;
    }

    else if (key == 'q')
    {
        exit(0);
    }

    wait_char();
}

void hero_movement()
{
    //Use standard input from keyboard
    int key = get_char();

    static char * hero_right = 
    ">>>"
    "/|\\"
    "/ \\";

    static char * hero_left = 
    "<<<"
    "/|\\"
    "/ \\";

    //If statements for player movement and 
    //collision detection with the border
    if (key == 'a')
    {
        sprite_move(knee_sliding_hero, -1, 0);
        sprite_set_image(knee_sliding_hero, hero_left);
    }

    else if ( key == 'd')
    {
        sprite_move(knee_sliding_hero, +1, 0);
        sprite_set_image(knee_sliding_hero, hero_right);
    }

    else if (key == 't')
    {
        treasure_pause = !treasure_pause;
    }

    else
    {
        sprite_set_image(knee_sliding_hero, hero);
    }

    if (sprites_collided(knee_sliding_hero, shiny_treasure))
    {
        sprite_destroy(knee_sliding_hero);
        sprite_destroy(shiny_treasure);
        respawn_hero();
        respawn_treasure();
        lives += 2;
    }

    sprites_collide_any();

        
    static char * hero_falling = 
    " O "
    "<|>"
    "/ \\";

    if (gravity)
    {
        sprite_move(knee_sliding_hero, 0, + 0.25);
        sprite_set_image(knee_sliding_hero, hero_falling);
    }

    if (lives <= 0)
    {
        game_finished();
    }

    sprite_draw(knee_sliding_hero);

}

void treasure_movement()
{
    int key_code = get_char();

    static char * treasure_full = 
    ",___,"
    "|$$$|";

    if (key_code < 0 && !treasure_pause)
    {
        sprite_step(shiny_treasure);

        int tx = round(sprite_x(shiny_treasure));
        int ty = round(sprite_y(shiny_treasure));

        double tdx = sprite_dx(shiny_treasure);
        double tdy = sprite_dy(shiny_treasure);

        if (tx <= 0)
        {
            tdx = fabs(tdx);
        }
        else if (tx >= screen_width() - sprite_width(shiny_treasure))
        {
            tdx = -fabs(tdx);
        }

        if (ty <= 0)
        {
            tdy = fabs(tdy);
        }
        else if (ty >= 2 - sprite_height(shiny_treasure))
        {
            tdy = -fabs(tdy);
        }

        if (tdx != sprite_dx(shiny_treasure) || tdy != sprite_dy(shiny_treasure))
        {
            sprite_back(shiny_treasure);
            sprite_turn_to(shiny_treasure, tdx, tdy);
        }
        if (tx > screen_width() / 2)
        {
            sprite_set_image(shiny_treasure, treasure_full);
        }
        else
        {
            sprite_set_image(shiny_treasure, treasure);
        }
    }

    sprite_draw(shiny_treasure);
}

void border_death()
{
    //Round off decimals to allow for collision detection
    int hx = round(sprite_x(knee_sliding_hero));
    int hy = round(sprite_y(knee_sliding_hero));

    if(hx >= screen_width() || hx < -1)
    {
        sprite_destroy(knee_sliding_hero);
        respawn_hero();
        lives -= 1;
    }

    if(hy >= screen_height())
    {
        sprite_destroy(knee_sliding_hero);
        respawn_hero();
        lives -= 1;
    }

}


void process()
{
    hero_movement();
    treasure_movement();
    draw_border();
    game_info();
    draw_platforms();
    border_death();
    gravity = true;
}

int main(void) {
    while(new_game)
    {
        srand(get_current_time());
        setup_screen();
        setup();
        create_platforms();
        bool update_screen = true;

        while ( !game_over ) {
            process();

            if ( update_screen ) {
                show_screen();
            }

            timer_pause(10);
            clear_screen();
        }
    
	return 0;
    }
}



//DEBUG
// clear_screen();
// draw_string(50, 20, "Debug");
// show_screen();
// timer_pause(5000);