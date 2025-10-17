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
#include "sdl-fe.h"


void fatal(const char *fmt, ...) {
   va_list ap;
   fprintf(stderr, "fatal error: ");
   va_start(ap, fmt);
   vfprintf(stderr, fmt, ap);
   va_end(ap);
   fprintf(stderr, "\n");
   exit(1);
}

void get_random_seed(void **randseed, int *randseedsize) {
    struct timeval *tvp = snew(struct timeval);
    gettimeofday(tvp, NULL);
    *randseed = (void *)tvp;
    *randseedsize = sizeof(struct timeval);
}

static void snaffle_colours(frontend *fe) {
   fe->colours = midend_colours(fe->me, &fe->ncolours);
}

void frontend_default_colour(frontend *fe, float *output) {
        output[0] = output[1] = output[2] = 1.0F;
}

void activate_timer(frontend *fe) {
   printf("activating timer, not.\n");
}

void deactivate_timer(frontend *fe) {
   printf("deactivating timer, not\n");
}

static void draw_fill(frontend *fe) {
   cairo_fill(fe->cr);
}

static void draw_fill_preserve(frontend *fe) {
   cairo_fill_preserve(fe->cr);
}

static void draw_set_colour(frontend *fe, int colour) {
    cairo_set_source_rgb(fe->cr,
                        fe->colours[3*colour + 0], 
                        fe->colours[3*colour + 1],
                        fe->colours[3*colour + 2]);
}

void sdl_drawing_free(drawing* dr) { 
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   sfree(fe); 
}

void sdl_draw_text(drawing *dr, int x, int y, int fonttype, int fontsize, int align, int colour, const char *text) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   cairo_text_extents_t extents;
   int origx=x, origy=y;
   draw_set_colour(fe,colour);
   cairo_set_font_size(fe->cr, fontsize);
   cairo_text_extents(fe->cr, text, &extents); 
   if (align & ALIGN_VCENTRE) {
	   y += extents.height / 2;
   } else { // ALIGN_VNORMAL
	   //SGT and Cairo agree that the baseline = vertical origin.
   }
   if (align & ALIGN_HCENTRE) {
	   x -= extents.x_advance / 2; // extents.width makes a dog's dinner of measuring the width of '1'
   } else if (align & ALIGN_HRIGHT) {
	  x -= extents.x_advance;
   } else {//ALIGN_HLEFT
   }
   cairo_move_to(fe->cr, x, y);
   cairo_show_text(fe->cr, text);
   //sdl_draw_circle(dr,origx,origy,2,1,2);
}

void sdl_draw_rect(drawing *dr, int x, int y, int w, int h, int colour) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   draw_set_colour(fe, colour);
   cairo_save(fe->cr);
   cairo_new_path(fe->cr);
   cairo_set_antialias(fe->cr, CAIRO_ANTIALIAS_NONE);
   cairo_rectangle(fe->cr, x, y, w, h);
   cairo_fill(fe->cr); 
   cairo_restore(fe->cr);
   //printf("Drew a rect\n");
}

void sdl_draw_line(drawing *dr, int x1, int y1, int x2, int y2, int colour) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   draw_set_colour(fe, colour);
   cairo_new_path(fe->cr);
   cairo_move_to(fe->cr, x1 + 0.5, y1 + 0.5);
   cairo_line_to(fe->cr, x2 + 0.5, y2 + 0.5);
   cairo_stroke(fe->cr);
   //printf("Drew a line\n");
}

void sdl_draw_thick_line(drawing *dr, float thickness, float x1, float y1, float x2, float y2, int colour) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   draw_set_colour(fe, colour);
   cairo_save(fe->cr);
   cairo_set_line_width(fe->cr, thickness);
   cairo_new_path(fe->cr);
   cairo_move_to(fe->cr, x1, y1);
   cairo_line_to(fe->cr, x2, y2);
   cairo_stroke(fe->cr);
   cairo_restore(fe->cr);
   //printf("Drew a THICC line\n");
}

void sdl_draw_polygon(drawing *dr, const int *coords, int npoints,  int fillcolour, int outlinecolour) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   int i;
   cairo_new_path(fe->cr);
   for (i = 0; i < npoints; i++)
	   cairo_line_to(fe->cr, coords[i*2] + 0.5, coords[i*2 + 1] + 0.5);
   cairo_close_path(fe->cr);
   if (fillcolour >= 0) {
     draw_set_colour(fe, fillcolour);
	  draw_fill_preserve(fe);
   }
   //assert(outlinecolour >= 0);
   draw_set_colour(fe, outlinecolour);
   cairo_stroke(fe->cr);
   //printf("drew an %i-sided polygon\n", npoints);
}

void sdl_draw_circle(drawing *dr, int cx, int cy, int radius, int fillcolour, int outlinecolour) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   cairo_new_path(fe->cr);
   cairo_arc(fe->cr, cx + 0.5, cy + 0.5, radius, 0, 2*PI);
   cairo_close_path(fe->cr);		/* Just in case... */
   if (fillcolour >= 0) {
	   draw_set_colour(fe, fillcolour);
	   draw_fill_preserve(fe);
   }
   //assert(outlinecolour >= 0);
   draw_set_colour(fe, outlinecolour);
   cairo_stroke(fe->cr);
   //printf("Drew a circle\n");
}

void sdl_clip(drawing *dr, int x, int y, int w, int h) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   //printf("clipped something\n");
   cairo_new_path(fe->cr);
   cairo_rectangle(fe->cr, x, y, w, h);
   cairo_clip(fe->cr);
}

void sdl_unclip(drawing *dr) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   //printf("unclipped a thing\n");
   cairo_reset_clip(fe->cr);
}

void sdl_start_draw(drawing *dr) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   fe->bbox_l = fe->sdl_surface->w;
   fe->bbox_r = 0;
   fe->bbox_u = fe->sdl_surface->h;
   fe->bbox_d = 0;
   SDL_FillRect( fe->sdl_surface, NULL, SDL_MapRGB( fe->sdl_surface->format, 255, 255, 255 ) );
   fe->image = cairo_image_surface_create_for_data( (unsigned char *) fe->sdl_surface->pixels, CAIRO_FORMAT_RGB24, fe->sdl_surface->w, fe->sdl_surface->h, fe->sdl_surface->pitch );
   fe->cr = cairo_create(fe->image);
   cairo_select_font_face (fe->cr,  "@cairo:monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
   cairo_set_antialias(fe->cr, CAIRO_ANTIALIAS_GRAY);
   cairo_set_line_width(fe->cr, 1.0);
   cairo_set_line_cap(fe->cr, CAIRO_LINE_CAP_SQUARE);
   cairo_set_line_join(fe->cr, CAIRO_LINE_JOIN_ROUND);
   //printf("Starting a draw\n");
}

void sdl_draw_update(drawing *dr, int x, int y, int w, int h) {
   // this seems to expand the "dirty area" that will be refreshed on the next end_draw. I could just refresh everything?
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   if (fe->bbox_l > x  ) fe->bbox_l = x  ;
   if (fe->bbox_r < x+w) fe->bbox_r = x+w;
   if (fe->bbox_u > y  ) fe->bbox_u = y  ;
   if (fe->bbox_d < y+h) fe->bbox_d = y+h;
}

void sdl_end_draw(drawing *dr) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   printf("end_draw, I should probably poke SDL to render or something\n");
   //I Guess I have to make a repaint happen here? 
   SDL_Texture * texture = SDL_CreateTextureFromSurface( fe->renderer, fe->sdl_surface );
   SDL_RenderCopy( fe->renderer, texture, NULL, NULL ) ;
   SDL_RenderPresent( fe->renderer );
   SDL_DestroyTexture( texture );

   cairo_surface_destroy( fe->image );
   cairo_destroy( fe->cr );
}

blitter *sdl_blitter_new(drawing *dr, int w, int h) { 
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   blitter *bl = snew(blitter);
   bl->image = NULL;
   bl->w = w;
   bl->h = h;
   return bl;
 }

void sdl_blitter_free(drawing *dr, blitter *bl) { 
   if (bl->image)
      cairo_surface_destroy(bl->image);
   sfree(bl); 
}

void sdl_blitter_save(drawing *dr, blitter *bl, int x, int y) {
       frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
    cairo_t *cr;
    if (!bl->image)
        bl->image = cairo_surface_create_similar(fe->image, CAIRO_CONTENT_COLOR, bl->w, bl->h);
    cr = cairo_create(bl->image);
    cairo_set_source_surface(cr, fe->image, -x, -y);
    cairo_paint(cr);
    cairo_destroy(cr);
}

void sdl_blitter_load(drawing *dr, blitter *bl, int x, int y) {
   frontend *fe = GET_HANDLE_AS_TYPE(dr, frontend);
   cairo_save(fe->cr);
   cairo_set_source_surface(fe->cr, bl->image, x, y);
   cairo_paint(fe->cr);
   cairo_restore(fe->cr);
}

void sdl_status_bar(drawing *dr, const char *text) {
   printf("status bar isn't '%s'\n",text);
}

void document_add_puzzle(document *doc, const game *game, game_params *par,
			 game_ui *ui, game_state *st, game_state *st2) {return;} // midend seems to call this in relation to printing.



int main( void ) {
   int width      = 720; 
   int height     = 480;
   int videoFlags = SDL_WINDOW_FULLSCREEN;
   int quit = 0;
   double x;
   double y;
   frontend* fe = snew(frontend);
   fe->quit=0;
   fe->me=midend_new(fe, &thegame, &sdl_drawing, fe);

   if( ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) ) {
      SDL_Log( "Unable to initialize SDL: %s.\n", SDL_GetError() );
      exit( EXIT_FAILURE );
   }
   fe->window = SDL_CreateWindow( "SGT Puzzles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, videoFlags );
   if( fe->window == NULL ) {
      SDL_Log( "Could not create window: %s.\n", SDL_GetError() );
      exit( EXIT_FAILURE );
   }
   fe->renderer = SDL_CreateRenderer( fe->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
   if( fe->renderer == NULL ) {
      SDL_Log( "Could not create Renderer: %s.\n", SDL_GetError() );
      exit( EXIT_FAILURE );
   }
   fe->sdl_surface = SDL_CreateRGBSurface( videoFlags, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0 );
   if( fe->sdl_surface == NULL ) {
      SDL_Log( "SDL_CreateRGBSurface() failed: %s\n", SDL_GetError() );
      exit( EXIT_FAILURE );
   }

   midend_new_game(fe->me);
   midend_size(fe->me, &width, &height, 1, 1.0);
   snaffle_colours(fe);

   while( ! fe->quit ) {
      midend_force_redraw(fe->me);
      //SDL_RenderClear( fe->renderer );
  
      // SDL_Texture * texture = SDL_CreateTextureFromSurface( fe->renderer, fe->sdl_surface );
      // SDL_RenderCopy( fe->renderer, texture, NULL, NULL ) ;
      // SDL_RenderPresent( fe->renderer );
      // SDL_DestroyTexture( texture );

      SDL_Event event;
      if( SDL_WaitEvent( &event ) ) {
         do {
            nom_key_event(fe, &event);
            switch( event.type ) {
           

               case SDL_QUIT:
                  fe->quit = 1;
                  break;

               case SDL_WINDOWEVENT:
                  switch( event.window.event ) {
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
   midend_free(fe->me);
   sfree(fe);
   return 0;
}

void nom_key_event(frontend *fe, SDL_Event *event) {
   int keydown = 0, keyup = 0;
   if (event->type ==SDL_KEYUP ) keyup = 1;
   if (event->type ==SDL_KEYDOWN ) keydown = 1;
   SDL_Keycode sym= event->key.keysym.sym;
   int keyval=0;
   switch(sym) {
      case SDLK_UP:
         keyval=CURSOR_UP; break;
      case SDLK_DOWN:
         keyval=CURSOR_DOWN; break;
      case SDLK_LEFT:
         keyval=CURSOR_LEFT; break;
      case SDLK_RIGHT:
         keyval=CURSOR_RIGHT; break;
      case SDLK_z: 
         keyval=UI_UNDO; break;
      case SDLK_r:
         keyval=UI_REDO; break;
      case SDLK_SPACE:
         keyval=' ' ; break;
      case SDLK_RETURN:
         keyval='\n' ; break; //yolo
      case SDLK_q:
         fe->quit=1; break;
   }
   if (keydown && keyval) {
      midend_process_key(fe->me, 0, 0, keyval);
   }
}

static const struct drawing_api sdl_drawing = {
    1,
    sdl_draw_text,
    sdl_draw_rect,
    sdl_draw_line,
    sdl_draw_polygon,
    sdl_draw_circle,
    sdl_draw_update,
    sdl_clip,
    sdl_unclip,
    sdl_start_draw,
    sdl_end_draw,
    sdl_status_bar,
    sdl_blitter_new,
    sdl_blitter_free,
    sdl_blitter_save,
    sdl_blitter_load,
    NULL, //begin_doc
    NULL, //begin_page
    NULL, //begin_puzzle
    NULL, //end_doc
    NULL, //end_page
    NULL, //end_puzzle
    NULL, //line_width
    NULL, //line_dotted	
    NULL, //text_fallback
    sdl_draw_thick_line,
};