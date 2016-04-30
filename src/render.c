/**
 * Outline a rectangle on the renderer with some given width
 *
 * in;
 *  renderer : handle to where we're outputting
 *  rect     : shape information
 *  width    : width of the outline
 */
static void
DK_RenderOutlineRect(SDL_Renderer *renderer, SDL_Rect rect, int width)
{
    for (int i = 0; i < width; i++) {
        rect.x -= 1;
        rect.y -= 1;
        rect.w += 2;
        rect.h += 2;
        SDL_RenderDrawRect(renderer, &rect);
    }
}

/**
 * Inline a rectangle with some given width line on the renderer
 *
 * in:
 *  renderer : handle to where we're outputting
 *  rect     : shape information
 *  width    : width of the inline
 */
static void
DK_RenderInlineRect(SDL_Renderer *renderer, SDL_Rect rect, int width)
{
    for (int i = 0; i < width; i++) {
        SDL_RenderDrawRect(renderer, &rect);
        rect.x += 1;
        rect.y += 1;
        rect.w -= 2;
        rect.h -= 2;
    }
}

/**
 * Render a single spot on the board, creates a rect and inlines it
 *
 * in:
 *  renderer : handle to where we want to output
 *  rect     : position information
 *  color    : appearance on the piece
 */
static void
DK_RenderSpot(SDL_Renderer *renderer, SDL_Rect rect, v4 color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_Rect r2 = rect;
    r2.x += 4;
    r2.y += 4;
    r2.w -= 8;
    r2.h -= 8;
    SDL_RenderFillRect(renderer, &r2);

    v3 hsv = RGBtoHSV(color);
    hsv.v *= 0.3f;
    color = HSVtoRGB(hsv);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    DK_RenderInlineRect(renderer, r2, 4);
}

/**
 * Render a board at a specified position for the origin, with dimensions
 *
 * in:
 *  renderer : where to output the board
 *  board    : the handle to the specific board
 *  pos      : bottom-left origin
 *  dim      : dimensions to draw the board to
 */
static void
RenderBoardPosDim(SDL_Renderer *renderer, struct board *board, v2 pos, v2 dim)
{
    int screenHeight;
    SDL_RenderGetLogicalSize(renderer, NULL, &screenHeight);

    SDL_Rect rect = {
        .x = pos.x,
        .y = screenHeight - pos.y - (BLOCK_SIZE * dim.y),
        .w = dim.x * BLOCK_SIZE,
        .h = dim.y * BLOCK_SIZE
    };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    DK_RenderOutlineRect(renderer, rect, 5);

    rect = (SDL_Rect) {
        .x = pos.x,
        .y = screenHeight - BLOCK_SIZE - pos.y,
        .w = BLOCK_SIZE,
        .h = BLOCK_SIZE
    };
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        if (y >= dim.y)
            break;

        for (int x = 0; x < dim.x; x++) {
            SDL_SetRenderDrawColor(renderer, 90, 90, 90, 255);
            SDL_RenderFillRect(renderer, &rect);

            SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
            DK_RenderInlineRect(renderer, rect, 2);

            // NOTE(david): draw the pieces
            if (board->pos[y][x] != s_COUNT) {
                DK_RenderSpot(renderer, rect, typeColor[board->pos[y][x]]);
            }

            rect.x += BLOCK_SIZE;
        }
        rect.y -= BLOCK_SIZE;
        rect.x = pos.x;
    }
}

/**
 * Render the board with a position and the default dimensions
 *
 * in:
 *  renderer : where to output to
 *  board    : handle for the board
 *  pos      : bottom-left origin of the board
 */
static void
RenderBoardPos(SDL_Renderer *renderer, struct board *board, v2 pos)
{
    RenderBoardPosDim(renderer, board, pos, V2(BOARD_WIDTH, BOARD_VHEIGHT));
}

/**
 * Render text at a position
 *
 * in:
 *  renderer : handle to where we want to output
 *  font     : handle to the font to use
 *  str      : what we're putting on the screen
 *  pos      : bottom-left position of the text
 */
static void
DK_RenderText(SDL_Renderer *renderer, TTF_Font *font, const char *str, v2 pos)
{
    int screenHeight;
    SDL_RenderGetLogicalSize(renderer, NULL, &screenHeight);

    SDL_Surface *tSurface = TTF_RenderText_Blended(font, str, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *tTexture = SDL_CreateTextureFromSurface(renderer, tSurface);
    i32 tw, th;
    SDL_QueryTexture(tTexture, NULL, NULL, &tw, &th);
    SDL_Rect r = {
        .x = pos.x,
        .y = screenHeight - pos.y - th,
        .w = tw,
        .h = th
    };
    SDL_RenderCopy(renderer, tTexture, NULL, &r);
    SDL_FreeSurface(tSurface);
    SDL_DestroyTexture(tTexture);
}

/**
 * Render text centered at a position
 *
 * in:
 *  renderer : handle to where we want to output
 *  font     : handle to the font to use
 *  str      : what we're putting on the screen
 *  pos      : centered position of the text
 */
static void
DK_RenderTextCenter(SDL_Renderer *renderer, TTF_Font *font, const char *str, v2 pos)
{
    int screenHeight;
    SDL_RenderGetLogicalSize(renderer, NULL, &screenHeight);

    SDL_Surface *tSurface = TTF_RenderText_Blended(font, str, (SDL_Color){0, 0, 0, 255});
    SDL_Texture *tTexture = SDL_CreateTextureFromSurface(renderer, tSurface);
    i32 tw, th;
    SDL_QueryTexture(tTexture, NULL, NULL, &tw, &th);
    SDL_Rect r = {
        .x = pos.x - tw/2,
        .y = screenHeight - pos.y - th - th/2,
        .w = tw,
        .h = th
    };
    SDL_RenderCopy(renderer, tTexture, NULL, &r);
    SDL_FreeSurface(tSurface);
    SDL_DestroyTexture(tTexture);
}

/**
 * Render text right-adjusted at a position
 *
 * in:
 *  renderer : handle to where we want to output
 *  font     : handle to the font to use
 *  str      : what we're putting on the screen
 *  pos      : bottom-right position of the text
 */
static void
DK_RenderTextRight(SDL_Renderer *renderer, TTF_Font *font, const char *str, v2 pos)
{
    int screenHeight;
    SDL_RenderGetLogicalSize(renderer, NULL, &screenHeight);

    SDL_Surface *tSurface = TTF_RenderText_Blended(font, str, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *tTexture = SDL_CreateTextureFromSurface(renderer, tSurface);
    i32 tw, th;
    SDL_QueryTexture(tTexture, NULL, NULL, &tw, &th);
    SDL_Rect r = {
        .x = pos.x - tw,
        .y = screenHeight - pos.y - th,
        .w = tw,
        .h = th
    };
    SDL_RenderCopy(renderer, tTexture, NULL, &r);
    SDL_FreeSurface(tSurface);
    SDL_DestroyTexture(tTexture);
}

/**
 * Render the game itself, runs through necessary functions
 *
 * in:
 *  renderer : handle to our output
 *  state    : the current state of the game
 */
static void
DK_RenderGame(SDL_Renderer *renderer, struct game_state *state)
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 140, 255);
    SDL_RenderClear(renderer);

    int screenHeight;
    SDL_RenderGetLogicalSize(renderer, NULL, &screenHeight);

    // RENDER DROPPING -----------------------------------------------------------------------------------
    RenderBoardPos(renderer, &(state->board), V2(20, 20));

    // RENDER PREVIEW ------------------------------------------------------------------------------------
    RenderBoardPosDim(
            renderer, 
            &(state->nextView), 
            V2(40 + (BOARD_WIDTH*BLOCK_SIZE), 20 + 14 * BLOCK_SIZE), 
            V2(5,5));

    // RENDER HOLD ---------------------------------------------------------------------------------------
    RenderBoardPosDim(
            renderer, 
            &(state->holdView), 
            V2(40 + (BOARD_WIDTH*BLOCK_SIZE), 20 + 8 * BLOCK_SIZE), 
            V2(5,5));

    // RENDER SCORE & LEFT TO CLEAR ----------------------------------------------------------------------
    char temp[15];
    sprintf(temp, "%5d", (i32)state->score);
    DK_RenderTextRight(renderer, state->font, temp, V2i(185 + (BOARD_WIDTH*BLOCK_SIZE), 200));

    sprintf(temp, "%2d - %3d",
            (state->clearedGoal - 10)/2 + 1, (state->clearedGoal - state->clearedRows));
    DK_RenderTextRight(renderer, state->font, temp, V2i(185 + (BOARD_WIDTH*BLOCK_SIZE), 150));
}
