#include <complex_plane_colorschemes.h>

#define COLORSCHEME_ITERATIONS_DEFAULT 0
#define COLORSCHEME_ITERATIONS_BLUE 1
#define COLORSCHEME_ITERATIONS_GREEN 2
#define COLORSCHEME_ITERATIONS_BLOODRED 3
#define COLORSCHEME_ITERATIONS_LEAFAGE 4
#define COLORSCHEME_ITERATIONS_SEABLUE 5
#define COLORSCHEME_ITERATIONS_GLITCH 6

#define COLORSCHEME_NUM 7

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
    default:
      return "Colorscheme name not defined";
      break;
  }
}

int complex_plane_colorschemes_get_num(){
  return COLORSCHEME_NUM;
}
