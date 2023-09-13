#include <graphx.h>
#include <keypadc.h>
#include <sys/timers.h>
#define RECTW 5
#define RECTH 40
#define BALLR 3

// defines the speed of the ball
typedef enum ball_speed
{
    HIGH = 6,
    LOW = 3
} ball_speed;

// left or right side score
typedef enum side_t
{
    LEFT,
    RIGHT
} side_t;

// holds the properties of a score
typedef struct score_t
{
    int score;
    int side;
} score_t;

// holds the properties of the ball,
// i.e. velocity and position
typedef struct ball_t
{
    // the position of the ball
    int x;
    int y;
    // change in x and y
    // "delta x" and "delta y"
    int dx;
    int dy;
} ball_t;

void startScreen();
void checkPaddles(gfx_point_t* lRect, gfx_point_t* rRect);
void checkBall(ball_t* ball, score_t* lscore, score_t* rscore);
void checkBallwPaddles(ball_t* ball, gfx_point_t* lRect, gfx_point_t* rRect);
void updateBall(ball_t* ball);
void score(score_t score, int del_print);
void winningScreen(int lscore);

int main(void)
{
    gfx_point_t lRect;
    lRect.x = RECTW * 2;
    lRect.y = (GFX_LCD_HEIGHT / 4) * 3;

    gfx_point_t rRect;
    rRect.x = GFX_LCD_WIDTH - (RECTW * 3);
    rRect.y = lRect.y;

    ball_t ball;
    ball.x = GFX_LCD_WIDTH / 2;
    ball.y = GFX_LCD_HEIGHT / 2;
    ball.dx = -(HIGH);
    ball.dy = LOW;

    score_t lscore;
    lscore.side = LEFT;
    lscore.score = 0;
    score_t rscore;
    rscore.side = RIGHT;
    rscore.score = 0;

    // drawing starts
    gfx_Begin();

    startScreen();

    while (!kb_AnyKey());

    // game begins

    gfx_FillScreen(255);
    gfx_SetColor(0);

    while (lscore.score < 10 && rscore.score < 10 && !kb_IsDown(kb_KeyClear))
    {
        kb_Scan();

        checkPaddles(&lRect, &rRect);

        // clear last ball from screen
        gfx_SetColor(255);
        gfx_FillCircle(ball.x, ball.y, BALLR);

        checkBall(&ball, &lscore, &rscore);
        checkBallwPaddles(&ball, &lRect, &rRect);
        updateBall(&ball);

        gfx_SetColor(0);
        gfx_FillRectangle(rRect.x, rRect.y, RECTW, RECTH);
        gfx_FillRectangle(lRect.x, lRect.y, RECTW, RECTH);
        gfx_FillCircle(ball.x, ball.y, BALLR);

        score(lscore, 1);
        score(rscore, 1);

        delay(25);
    }

    // clear ball
    gfx_SetColor(255);
    gfx_FillCircle(ball.x, ball.y, BALLR);
    // display winner
    winningScreen(lscore.score);

    while (!kb_AnyKey());

    // drawing ends
    gfx_End();

    return 0;
}

void startScreen()
{
    char title[] = "TI Pong!";
    char credits[] = "Made by: Colin Enck";
    char moreCredits[] = "Made with the CE Toolchain";

    gfx_PrintStringXY(title, (GFX_LCD_WIDTH - gfx_GetStringWidth(title)) / 2,
        ((GFX_LCD_HEIGHT - 8) / 2) - 12);

    gfx_PrintStringXY(credits, (GFX_LCD_WIDTH - gfx_GetStringWidth(credits)) / 2,
        (GFX_LCD_HEIGHT - 8) / 2);

    gfx_PrintStringXY(moreCredits, (GFX_LCD_WIDTH - gfx_GetStringWidth(moreCredits)) / 2,
        ((GFX_LCD_HEIGHT - 8) / 2) + 12);
}

void checkPaddles(gfx_point_t *lRect, gfx_point_t *rRect)
{
    if (kb_IsDown(kb_KeyUp) && rRect->y >= 0)
        {
            // delete previous drawing
            gfx_SetColor(255);
            gfx_FillRectangle(rRect->x, rRect->y, RECTW, RECTH);

            // update location
            rRect->y -= 5;
        }
        else if (kb_IsDown(kb_KeyDown) && rRect->y <= GFX_LCD_HEIGHT - RECTH)
        {
            gfx_SetColor(255);
            gfx_FillRectangle(rRect->x, rRect->y, RECTW, RECTH);

            rRect->y += 5;
        }

        if (kb_IsDown(kb_Key2nd) && lRect->y >= 0)
        {
            gfx_SetColor(255);
            gfx_FillRectangle(lRect->x, lRect->y, RECTW, RECTH);

            lRect->y -= 5;
        } 
        else if (kb_IsDown(kb_KeyAlpha) && lRect->y <= GFX_LCD_HEIGHT - RECTH)
        {
            gfx_SetColor(255);
            gfx_FillRectangle(lRect->x, lRect->y, RECTW, RECTH);

            lRect->y += 5;
        }
}

void checkBall(ball_t* ball, score_t* lscore, score_t* rscore)
{
    // if hits top/bottom wall then negate dx or dy
    // if hits side wall then return to start
    // has to account for radius of ball plus buffer of BALLR/2
    if (ball->y <= BALLR)
    {
        ball->y = BALLR;
        ball->dy = -(ball->dy);
    } else if (ball->y >= (GFX_LCD_HEIGHT - BALLR))
    {
        ball->y = GFX_LCD_HEIGHT - BALLR;
        ball->dy = -(ball->dy);
    }
    if (ball->x <= (BALLR + 1))
    {
        ball->x = GFX_LCD_WIDTH / 2;
        ball->y = GFX_LCD_HEIGHT / 2;
        ball->dx = HIGH;
        ball->dy = LOW;
        score(*rscore, 0);
        (rscore->score)++;
    } else if (ball->x >= (GFX_LCD_WIDTH - BALLR - 1))
    {
        ball->x = GFX_LCD_WIDTH / 2;
        ball->y = GFX_LCD_HEIGHT / 2;
        ball->dx = -(HIGH);
        ball->dy = LOW;
        score(*lscore, 0);
        (lscore->score)++;
    }
}

void checkBallwPaddles(ball_t* ball, gfx_point_t* lRect, gfx_point_t* rRect)
{
    // collisions with paddles
    // high speed for dy, low for dx between 0/4-1/4 (of paddle) or 3/4-4/4
    // low speed for dy, high for dx between 1/4-3/5
    if (ball->x >= rRect->x)
    {
        if (ball->y >= rRect->y && ball->y <= rRect->y + (RECTH / 4))
        {
            ball->dy = (ball->dy > 0) ? HIGH : -(HIGH);
            ball->dx = -(LOW);
        }
        else if (ball->y >= rRect->y + ((RECTH / 4) * 3) && ball->y <= (rRect->y + RECTH))
        {
            ball->dy = (ball->dy > 0) ? HIGH : -(HIGH);
            ball->dx = -(LOW);
        }
        else if (ball->y > rRect->y + (RECTH / 4) && ball->y < rRect->y + ((RECTH / 4) * 3))
        {
            ball->dy = (ball->dy > 0) ? LOW : -(LOW);
            ball->dx = -(HIGH);
        }
    }
    else if (ball->x <= lRect->x)
    {
        if (ball->y >= lRect->y && ball->y <= lRect->y + (RECTH / 5))
        {
            ball->dy = (ball->dy > 0) ? HIGH : -(HIGH);
            ball->dx = LOW;
        }
        else if (ball->y >= lRect->y + ((RECTH / 5) * 4) && ball->y <= (lRect->y + RECTH))
        {
            ball->dy = (ball->dy > 0) ? HIGH : -(HIGH);
            ball->dx = LOW;
        }
        else if (ball->y > lRect->y + (RECTH / 5) && ball->y < lRect->y + ((RECTH / 5) * 4))
        {
            ball->dy = (ball->dy > 0) ? LOW : -(LOW);
            ball->dx = HIGH;
        }
    }
}

void updateBall(ball_t* ball)
{
    // updates the new coordinates
    int new_x = ball->x + ball->dx;
    if (new_x > 0 && new_x < GFX_LCD_WIDTH)
        ball->x = new_x;

    int new_y = ball->y + ball->dy;
    if (new_y > 0 && new_y < GFX_LCD_HEIGHT)
        ball->y = new_y;
}

// 0 for delete, 1 for print
void score(score_t score, int del_print)
{
    gfx_point_t loc;
    loc.y = GFX_LCD_HEIGHT / 5;

    if (score.side == LEFT)
        loc.x = GFX_LCD_WIDTH / 3;
    else if (score.side == RIGHT)
        loc.x = (GFX_LCD_WIDTH / 3) * 2;

    if (del_print)
    {
        gfx_SetTextXY(loc.x, loc.y);
        gfx_PrintInt(score.score, 0);
    } else 
    {
        gfx_SetColor(255);
        gfx_FillRectangle(loc.x, loc.y, 8, 8);
    }
}

// only needs left score to determine winner
void winningScreen(int lscore)
{
    char lwins[] = "LEFT WINS!";
    char rwins[] = "RIGHT WINS!";
    char* title;
    if (lscore == 10)
    {
        title = (char*) malloc(11); // size of lwins
        if (title == NULL) exit(1);
        title = lwins;
    } else 
    {
        title = (char*) malloc(13); // size of rwins
        if (title == NULL) exit(1);
        title = rwins;
    }
    gfx_PrintStringXY(title, (GFX_LCD_WIDTH - gfx_GetStringWidth(title)) / 2,
        ((GFX_LCD_HEIGHT - 8) / 2) - 6);
}