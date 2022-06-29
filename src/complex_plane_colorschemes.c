#include <complex_plane_colorschemes.h>

#define COLORSCHEME_ITERATIONS_DEFAULT 0
#define COLORSCHEME_ITERATIONS_BLUE 1
#define COLORSCHEME_ITERATIONS_GREEN 2
#define COLORSCHEME_ITERATIONS_BLOODRED 3
#define COLORSCHEME_ITERATIONS_LEAFAGE 4
#define COLORSCHEME_ITERATIONS_SEABLUE 5
#define COLORSCHEME_ITERATIONS_GLITCH 6
#define COLORSCHEME_ITERATIONS_SUPER 7
#define COLORSCHEME_ITERATIONS_MEGA 8
#define COLORSCHEME_ITERATIONS_GIGA 9
#define COLORSCHEME_ITERATIONS_INFINITY 10
#define COLORSCHEME_ITERATIONS_PRECISSION 11
#define COLORSCHEME_ITERATIONS_BLACK 12
#define COLORSCHEME_ITERATIONS_WHITE 13
#define COLORSCHEME_ITERATIONS_TRANSCENDENTAL 14

#define COLORSCHEME_NUM 15

const char *complex_plane_colorschemes_get_name(int c){
  switch(c){
    case COLORSCHEME_ITERATIONS_DEFAULT:
      return "Default colorscheme";
      break;
    case COLORSCHEME_ITERATIONS_BLUE:
      return "Blue";
      break;
    case COLORSCHEME_ITERATIONS_GREEN:
      return "Green";
      break;
    case COLORSCHEME_ITERATIONS_BLOODRED:
      return "Blood Red";
      break;
    case COLORSCHEME_ITERATIONS_LEAFAGE:
      return "Leafage";
      break;
    case COLORSCHEME_ITERATIONS_SEABLUE:
      return "Sea Blue";
      break;
    case COLORSCHEME_ITERATIONS_GLITCH:
      return "Glitch";
      break;
    case COLORSCHEME_ITERATIONS_SUPER:
      return "Super";
      break;
    case COLORSCHEME_ITERATIONS_MEGA:
      return "Mega";
      break;
    case COLORSCHEME_ITERATIONS_GIGA:
      return "Giga";
      break;
    case COLORSCHEME_ITERATIONS_INFINITY:
      return "Infinity";
      break;
    case COLORSCHEME_ITERATIONS_PRECISSION:
      return "Precission";
      break;
    case COLORSCHEME_ITERATIONS_BLACK:
      return "Pure Black";
      break;
    case COLORSCHEME_ITERATIONS_WHITE:
      return "White on Black";
      break;
    case COLORSCHEME_ITERATIONS_TRANSCENDENTAL:
      return "Transcendental";
      break;
    default:
      return "Colorscheme name not defined";
      break;
  }
}

int complex_plane_colorschemes_get_num(){
  return COLORSCHEME_NUM;
}
