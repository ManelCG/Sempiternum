#include <complex_plane_colorschemes.h>

#define COLORSCHEME_ITERATIONS_DEFAULT 0
#define COLORSCHEME_ITERATIONS_BLUE 1
#define COLORSCHEME_ITERATIONS_GREEN 2
#define COLORSCHEME_ITERATIONS_GLITCH 3

#define COLORSCHEME_NUM 4

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
