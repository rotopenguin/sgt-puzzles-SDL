#pragma once

struct frontend {
midend *me;
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *sdl_surface;
const float *colours;
int ncolours;
cairo_t *cr;
cairo_surface_t *image;
int bbox_l, bbox_r, bbox_u, bbox_d;
int quit;
};
typedef frontend frontend;
static const struct drawing_api sdl_drawing;

static void changed_preset(frontend *fe);
static void load_prefs(frontend *fe);
static char *save_prefs(frontend *fe);
static void draw_fill(frontend *fe);
static void draw_fill_preserve(frontend *fe);
void nom_key_event(frontend *fe, SDL_Event *event);


void sdl_drawing_free(drawing *dr) ;
void sdl_draw_text(drawing *dr, int x, int y, int fonttype, int fontsize, int align, int colour, const char *text);
void sdl_draw_rect(drawing *dr, int x, int y, int w, int h, int colour) ;
void sdl_draw_line(drawing* dr, int x1, int y1, int x2, int y2, int colour) ;
void sdl_draw_thick_line(drawing *dr, float thickness, float x1, float y1, float x2, float y2, int colour);
void sdl_draw_polygon(drawing *dr, const int *coords, int npoints, int fillcolour, int outlinecolour) ;
void sdl_draw_circle(drawing *dr, int cx, int cy, int radius, int fillcolour, int outlinecolour);
void sdl_clip(drawing *dr, int x, int y, int w, int h) ;
void sdl_unclip(drawing *dr) ;
void sdl_start_draw(drawing *dr) ;
void sdl_draw_update(drawing *dr, int x, int y, int w, int h);
void sdl_end_draw(drawing *dr);

blitter *sdl_blitter_new(drawing *dr, int w, int h);
void sdl_blitter_free(drawing *dr, blitter *bl);
void sdl_blitter_save(drawing *dr, blitter *bl, int x, int y);
void sdl_blitter_load(drawing *dr, blitter *bl, int x, int y);
void sdl_status_bar(drawing *dr, const char *text);
void document_add_puzzle(document *doc, const game *game, game_params *par, game_ui *ui, game_state *st, game_state *st2);

struct blitter {
    cairo_surface_t *image;
    int w, h;
};
