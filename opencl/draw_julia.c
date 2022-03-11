__kernel void rec_f(__global unsigned char *m,
                    __global int *Np,
                    __global int *hp,
                    __global int *wp,
                    __global double *c,
                    __global double *Sx,
                    __global double *Sy) {
  double c_abs = native_sqrt(pow(c[0], 2) +  pow(c[1],2));
  double R = (c_abs > 2)? c_abs: 2;

  int h = *hp, w = *wp, N = *Np;


  const int y = get_global_id(0);
  const int x = get_global_id(1);

  //Map x and y to range between -R and R
  double newx = (double) x / (double) w;
  newx = newx * (Sx[1] - Sx[0]) + Sx[0];
  double newy = -(y - h);
  newy = (double) newy / (double) h;
  newy = newy * (Sy[1] - Sy[0]) + Sy[0];
  double z[2] = {newx, newy};
  double z_abs;
  double aux;

  for (int i = 0; i < N; i++){
    aux = (pow(z[0], 2) - pow(z[1], 2)) + c[0];
    z[1] = 2*z[0]*z[1] + c[1];
    z[0] = aux;
    z_abs = native_sqrt(pow(z[0], 2) + pow(z[1] , 2));

    if (z_abs > R){
      double ni = log10((double) i);
      double n = log10((double) N);
      m[(y*w + x)*3+0] = 255;
      m[(y*w + x)*3+1] = (int) floor( ((double) ni / (double) n) * 255);
      m[(y*w + x)*3+2] = i % 256;

      ////Blue
      //m[(y*w + x)*3+2] = 255;
      //m[(y*w + x)*3+0] = (int) floor( ((double) ni / (double) n) * 255);
      //m[(y*w + x)*3+1] = i % 256;
      break;
    }
  }
}

__kernel void parameter_space(__global unsigned char *m,
                    __global int *Np,
                    __global int *hp,
                    __global int *wp,
                    __global double *z0,
                    __global double *Sx,
                    __global double *Sy) {
  double z0_abs = native_sqrt(pow(z0[0], 2) +  pow(z0[1],2));
  double R = (z0_abs > 2)? z0_abs: 2;

  int h = *hp, w = *wp, N = *Np;


  const int y = get_global_id(0);
  const int x = get_global_id(1);

  //Map x and y to range between -R and R
  double newx = (double) x / (double) w;
  newx = newx * (Sx[1] - Sx[0]) + Sx[0];
  double newy = (double) y / (double) h;
  newy = newy * (Sy[1] - Sy[0]) + Sy[0];
  double c[2] = {newx, newy};
  double c_abs;
  double z[2];
  z[0] = z0[0];
  z[1] = z0[1];
  double z_abs;
  double aux;

  for (int i = 0; i < N; i++){
    aux = (pow(z[0], 2) - pow(z[1], 2)) + c[0];
    z[1] = 2*z[0]*z[1] + c[1];
    z[0] = aux;
    z_abs = native_sqrt(pow(z[0], 2) + pow(z[1] , 2));

    if (z_abs > R){
      double ni = log10((double) i);
      double n = log10((double) N);
      m[(y*w + x)*3] = 255;
      m[(y*w + x)*3+1] = (int) floor( ((double) ni / (double) n) * 255);
      // m[(y*w + x)*3+2] = 128;
      break;
    }
  }
}
