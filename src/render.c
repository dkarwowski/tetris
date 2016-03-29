static void
DK_RenderOutlineRect(SDL_Renderer *renderer_p, SDL_Rect rect, int width)
{
    for (int i = 0; i < width; i++) {
        rect.x -= 1;
        rect.y -= 1;
        rect.w += 2;
        rect.h += 2;
        SDL_RenderDrawRect(renderer_p, &rect);
    }
}

static void
DK_RenderInlineRect(SDL_Renderer *renderer_p, SDL_Rect rect, int width)
{
    for (int i = 0; i < width; i++) {
        SDL_RenderDrawRect(renderer_p, &rect);
        rect.x += 1;
        rect.y += 1;
        rect.w -= 2;
        rect.h -= 2;
    }
}

static void
DK_RenderSpot(SDL_Renderer *renderer_p, SDL_Rect rect, v4 color)
{
    SDL_SetRenderDrawColor(renderer_p, color.r, color.g, color.b, color.a);

    SDL_Rect r2 = rect;
    r2.x += 4;
    r2.y += 4;
    r2.w -= 8;
    r2.h -= 8;
    SDL_RenderFillRect(renderer_p, &r2);

    v3 hsv = RGBtoHSV(color);
    hsv.v *= 0.3f;
    color = HSVtoRGB(hsv);

    SDL_SetRenderDrawColor(renderer_p, color.r, color.g, color.b, color.a);
    DK_RenderInlineRect(renderer_p, r2, 4);
}

static void
RenderBoardPosDim(SDL_Renderer *renderer_p, struct board *board_p, v2 pos, v2 dim)
{
    int screenHeight;
    SDL_RenderGetLogicalSize(renderer_p, NULL, &screenHeight);

    SDL_Rect rect = {
        .x = pos.x,
        .y = screenHeight - pos.y - (BLOCK_SIZE * dim.y),
        .w = dim.x * BLOCK_SIZE,
        .h = dim.y * BLOCK_SIZE
    };
    DK_RenderOutlineRect(renderer_p, rect, 5);

    rect = (SDL_Rect) {
        .x = pos.x,
        .y = screenHeight - BLOCK_SIZE - pos.y,
        .w = BLOCK_SIZE,
        .h = BLOCK_SIZE
    };
    for_row(row_p, board_p->first) {
        if (row_p->y >= dim.y)
            break;

        for (int x = 0; x < dim.x; x++) {
            SDL_SetRenderDrawColor(renderer_p, 90, 90, 90, 255);
            SDL_RenderFillRect(renderer_p, &rect);

            SDL_SetRenderDrawColor(renderer_p, 70, 70, 70, 255);
            DK_RenderInlineRect(renderer_p, rect, 2);

            // NOTE(david): draw the pieces
            if (row_p->spots[x] != s_COUNT) {
                DK_RenderSpot(renderer_p, rect, DK_GetTypeColor(row_p->spots[x]));
            }

            rect.x += BLOCK_SIZE;
        }
        rect.y -= BLOCK_SIZE;
        rect.x = pos.x;
    }
}

static void
RenderBoardPos(SDL_Renderer *renderer_p, struct board *board_p, v2 pos)
{
    RenderBoardPosDim(renderer_p, board_p, pos, V2(BOARD_WIDTH, BOARD_VHEIGHT));
}

static void
DK_RenderText(SDL_Renderer *renderer_p, TTF_Font *font_p, const char *str, v2 pos)
{
    int screenHeight;
    SDL_RenderGetLogicalSize(renderer_p, NULL, &screenHeight);

    SDL_Surface *tSurface_p = TTF_RenderText_Blended(font_p, str, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *tTexture_p = SDL_CreateTextureFromSurface(renderer_p, tSurface_p);
    i32 tw, th;
    SDL_QueryTexture(tTexture_p, NULL, NULL, &tw, &th);
    SDL_Rect r = {
        .x = pos.x,
        .y = screenHeight - pos.y - th,
        .w = tw,
        .h = th
    };
    SDL_RenderCopy(renderer_p, tTexture_p, NULL, &r);
    SDL_FreeSurface(tSurface_p);
    SDL_DestroyTexture(tTexture_p);
}
