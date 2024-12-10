/* 6502 cc65 mandelbrot for badXpi

GPLv3
Derived from https://github.com/rahra/intfract by Bernhard R. Fischer
*/

// lib extensions
#include "pi_libc.h"
#include "pi_fb.h"


/*! This function contains the outer loop, i.e. calculate the coordinates
 * within the complex plane for each pixel and then call iterate().
 * @param realmin Minimun real value of image.
 * @param imagmin Minimum imaginary value of image.
 * @param realmax Maximum real value.
 * @param imagmax Maximum imaginary value.
 * @param hres Pixel width of image.
 * @param vres Pixel height of image.
 * @param start Column to start calculation with.
 * @param skip Number of columns to skip before starting with the next column.
 */

#define MAXITERATE 32
#define NORM_BITS 13

typedef long nint_t;
#define NORM_FACT ((nint_t)1 << NORM_BITS)

int maxiterate_ = MAXITERATE;

//char msg[45];

int iterate(nint_t real0, nint_t imag0)
{
   nint_t realq, imagq, real, imag;
   int i;

 //    sprintf(msg,"real=%ld   imag=%ld \n",real,imag);
 //    puts(msg);

//   sprintf(msg,"max %d\n",maxiterate_);
//   puts(msg);

   real = real0;
   imag = imag0;
   for (i = 0; i < maxiterate_; i++)
   {
     realq = (nint_t)(real * real) >> NORM_BITS;
     imagq = (nint_t)(imag * imag) >> NORM_BITS;

//     sprintf(msg,"realq=%ld   imagq=%ld \n",realq,imagq);
//     puts(msg);

     if ((realq + imagq) > (nint_t) 4 * NORM_FACT)
        break;

     imag = ((real * imag) >> (NORM_BITS - 1)) + imag0;
     real = realq - imagq + real0;
   }
   return i;
}

void mand_calc(nint_t realmin, nint_t imagmin, nint_t realmax, nint_t imagmax, int hres, int vres, int start, int skip)
{
  nint_t deltareal, deltaimag, real0,  imag0;
  int x, y;
  int col;

  deltareal = realmax - realmin;
  deltaimag = imagmax - imagmin;

  // Fractional inrementation does not work well with integers because of the
  // resolution of the delta being too low. Thus, the outer loop has slightly
  // more operations in the integer variant than in the double variant.
  
  *(unsigned int*)0xFF00=x;
  *(unsigned int*)0xFF02=y;
  *(unsigned char*)0xFF04=1;

  for (x = start; x < hres; x += skip)
  {
    real0 = realmin + deltareal * x / hres;
    for (y = 0; y < vres;)
    {
      int _y;
      imag0 = imagmax - deltaimag * y / vres;
      col = iterate(real0, imag0);
      // fill all pixels which are below int resolution with the same iteration value
      for (_y = 0; y < vres && deltaimag * _y < vres; _y++, y++) {
	      //col=(col & 0x3f) >> 1;
	      //fb_pointRGB(x,y,col,col,col);
        *(unsigned char*)0xFF06=col+16;
      }
    }
    *(unsigned char*)0xFF0A=0;
  }
}

int main()
{
  fb_init();

  puts(CHAR_CLS);
  puts(CHAR_CURSOR_HIDE);
  puts(CHAR_SREGION(24,29));
  puts(CHAR_VPOS(25));
  puts("running.\n");

  // Start and end values precalculated, since cc65 has no float.
  mand_calc(-21*819, -18*819, 11*819, 25*819, 720, 400, 0, 1);

  puts("done\n");
  puts(CHAR_SREGION(24,29));
  puts(CHAR_CURSOR_SHOW);
  
  sysexit(0);
  return 0;
}
