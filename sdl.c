/*
 *
 */

 #include <stdio.h>
 #include <sys/time.h>
 #include <stdarg.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <cairo/cairo.h>
#include "puzzles.h"

struct frontend {
midend *me;
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *sdl_surface;

cairo_t *cr;
cairo_surface_t *image;
};

void fatal(const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr, "fatal error: ");

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");
    exit(1);
}

void get_random_seed(void **randseed, int *randseedsize)
{
    struct timeval *tvp = snew(struct timeval);
    gettimeofday(tvp, NULL);
    *randseed = (void *)tvp;
    *randseedsize = sizeof(struct timeval);
}

static void changed_preset(frontend *fe);
static void load_prefs(frontend *fe);
static char *save_prefs(frontend *fe);

void frontend_default_colour(frontend *fe, float *output) {
        output[0] = output[1] = output[2] = 0.9F;
}

drawing *drawing_new(const drawing_api *api, midend *me, void *handle)
{ return snew(drawing); }
void drawing_free(drawing *dr) { sfree(dr); }
void draw_text(drawing *dr, int x, int y, int fonttype, int fontsize,
               int align, int colour, const char *text) {}
void draw_rect(drawing *dr, int x, int y, int w, int h, int colour) {}
#ifndef STANDALONE_POLYGON
void draw_line(drawing *dr, int x1, int y1, int x2, int y2, int colour) {}
#endif
void draw_thick_line(drawing *dr, float thickness,
		     float x1, float y1, float x2, float y2, int colour) {}
void draw_polygon(drawing *dr, const int *coords, int npoints,
                  int fillcolour, int outlinecolour) {}
void draw_circle(drawing *dr, int cx, int cy, int radius,
                 int fillcolour, int outlinecolour) {}
char *text_fallback(drawing *dr, const char *const *strings, int nstrings)
{ return dupstr(strings[0]); }
void clip(drawing *dr, int x, int y, int w, int h) {}
void unclip(drawing *dr) {}
void start_draw(drawing *dr) {}
void draw_update(drawing *dr, int x, int y, int w, int h) {}
void end_draw(drawing *dr) {}
struct blitter { char dummy; };
blitter *blitter_new(drawing *dr, int w, int h) { return snew(blitter); }
void blitter_free(drawing *dr, blitter *bl) { sfree(bl); }
void blitter_save(drawing *dr, blitter *bl, int x, int y) {}
void blitter_load(drawing *dr, blitter *bl, int x, int y) {}
int print_mono_colour(drawing *dr, int grey) { return 0; }
int print_grey_colour(drawing *dr, float grey) { return 0; }
int print_hatched_colour(drawing *dr, int hatch) { return 0; }
int print_rgb_mono_colour(drawing *dr, float r, float g, float b, int grey)
{ return 0; }
int print_rgb_grey_colour(drawing *dr, float r, float g, float b, float grey)
{ return 0; }
int print_rgb_hatched_colour(drawing *dr, float r, float g, float b, int hatch)
{ return 0; }
void print_line_width(drawing *dr, int width) {}
void print_line_dotted(drawing *dr, bool dotted) {}
void status_bar(drawing *dr, const char *text) {}
void document_add_puzzle(document *doc, const game *game, game_params *par,
			 game_ui *ui, game_state *st, game_state *st2) {}



int main( void )
{
   int width      = 720;
   int height     = 480;
   int videoFlags = SDL_WINDOW_FULLSCREEN;
   int quit = 0;
   double x;
   double y;

   const char * text = "https://github.com/rjopek/sdl-cairo";

   if( ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) )
   {
      SDL_Log( "Unable to initialize SDL: %s.\n", SDL_GetError() );
      exit( EXIT_FAILURE );
   }

   fe->window = SDL_CreateWindow( "SDL .AND. Cairo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, videoFlags );
   if( fe->window == NULL )
   {
      SDL_Log( "Could not create window: %s.\n", SDL_GetError() );
      exit( EXIT_FAILURE );
   }

   fe->renderer = SDL_CreateRenderer( fe->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
   if( fe->renderer == NULL )
   {
      SDL_Log( "Could not create Renderer: %s.\n", SDL_GetError() );
      exit( EXIT_FAILURE );
   }

   fe->sdl_surface = SDL_CreateRGBSurface( videoFlags, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0 );
   if( fe->sdl_surface == NULL )
   {
      SDL_Log( "SDL_CreateRGBSurface() failed: %s\n", SDL_GetError() );
      exit( EXIT_FAILURE );
   }

   while( ! quit )
   {
      SDL_FillRect( fe->sdl_surface, NULL, SDL_MapRGB( fe->sdl_surface->format, 255, 255, 255 ) );

      cairo_surface_t * cr_surface = cairo_image_surface_create_for_data( (unsigned char *) fe->sdl_surface->pixels, CAIRO_FORMAT_RGB24, fe->sdl_surface->w, fe->sdl_surface->h, fe->sdl_surface->pitch );
      cairo_t * cr = cairo_create( cr_surface );

      //---
      cairo_select_font_face( cr, "FreeMono", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD );
      cairo_set_font_size( cr, 34 );
      cairo_set_source_rgba( cr, 0.5, 0.5, 0.5, 1.0 );

      cairo_text_extents_t te;
      cairo_text_extents( cr, text, &te );

      x = ( width  - te.width ) / 2;
      y = ( height + te.height ) / 2;

      cairo_move_to( cr, x, y );
      cairo_show_text( cr, text );
      //---

      SDL_RenderClear( fe->renderer );
      SDL_Texture * texture = SDL_CreateTextureFromSurface( fe->renderer, fe->sdl_surface );
      SDL_RenderCopy( fe->renderer, texture, NULL, NULL ) ;
      SDL_RenderPresent( fe->renderer );
      SDL_DestroyTexture( texture );

      cairo_surface_destroy( cr_surface );
      cairo_destroy( cr );

      SDL_Event event;
      if( SDL_WaitEvent( &event ) )
      {
         do
         {
            switch( event.type )
            {
               case SDL_KEYDOWN:
                  if( event.key.keysym.sym == SDLK_ESCAPE )
                  {
                     quit = 1;
                  }
                  break;

               case SDL_QUIT:
                  quit = 1;
                  break;

               case SDL_WINDOWEVENT:
                  switch( event.window.event )
                  {
                     case SDL_WINDOWEVENT_SIZE_CHANGED:
                        width = event.window.data1;
                        height = event.window.data2;
                        break;
                     case SDL_WINDOWEVENT_CLOSE:
                        event.type = SDL_QUIT;
                        SDL_PushEvent( &event );
                        break;
                  }

               SDL_FreeSurface( fe->sdl_surface );
               fe->sdl_surface = SDL_CreateRGBSurface( videoFlags, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0 );
               break;
            }
         } while( SDL_PollEvent( &event ) );
         printf( "window width  = %d\n" "window height = %d\n", width, height );
      }
   }

   SDL_DestroyRenderer( fe->renderer );
   SDL_DestroyWindow( fe->window );
   SDL_Quit();

   return 0;
}
