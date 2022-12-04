#include <glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define MAXH 1000
#define MAXW 1000
typedef unsigned char byte;
typedef struct
{
  int w, h;
  byte *colors;
} Picture;

Picture gibbon, res, res2;
byte frame[MAXH][MAXW][3] = {0};


int LoadPicture(char *fileName, Picture *p)
{
  FILE *F;
  int y;
  short w, h;
 
  F = fopen(fileName, "rb");
 
  if (F == NULL)
  {
    fprintf(stderr, "File couldn't be read\n");
    return 0; 
  }
 
  fread(&w, 2, 1, F);
  fread(&h, 2, 1, F);
  
  p->colors = malloc((int)w * h * 3);
  if (p->colors == NULL)
  {
    fclose(F);
    fprintf(stderr, "Could not allocate %u bytes for the sovature\n");
    return 0;
  }
 
  for (y = 0; y < h; y++)
    fread(p->colors + y * w * 3, 3, w, F);
    p->w = w;
    p->h = h;
  fclose(F);
  return 1;
}
 

void DesPic(Picture *p)
{
  free(p->colors);
}  
 
void DrawPicture(int x0, int y0, Picture *p)
{
  int x, y, c;
 
  for (y = 0; y < p->h; y++)
    for (x = 0; x < p->w; x++)
      for (c = 0; c < 3; c++)
        frame[y0 + y][x0 + x][c]= p->colors [(y * p->w + x) * 3 + c ];
}
void CreatePicture(short w, short h, Picture *p)
{
  p->colors = malloc((int)w * h * 3);
  p->w = w;
  p->h = h; 
}

 
void Negative(Picture *src, Picture *dst)
{
  int x, y , c;
  for(y = 0; y < src->h ; y++)
    for(x = 0; x < src->w; x++)
      for (c = 0; c < 3; c++)
      dst->colors[(y * src->w + x) * 3 + c] = 255 - src->colors[(y * src->w + x) * 3 + c];
} 

byte *get(Picture *p, int x, int y, int c)
{
  return p->colors + (p->w * y + x) * 3 + c; 
}


void Resize(Picture *src, Picture *dst)
{
  int x, y , c, w1, h1, w, h, x1, y1;
  w1 = src->w;
  h1 = src->h;
  w = dst->w;
  h = dst->h;                        
  for(y = 0 ; y < dst->h ; y++)
    for(x = 0 ; x < dst->w; x++)
      for (c = 0; c < 3; c++)
      {
        x1 = (w1 - 1) * x / (w - 1);
        y1 = (h1 - 1) * y / (h - 1); 
        *get(dst, x , y, c) = *get(src, x1, y1, c);
      }
} 

void Rotate(Picture *src, Picture *dst, double angle)
{
  int x, y , c, w1, h1, w, h, x1, y1, dx1, dy1, x0, y0, dx, dy;
  double si, co;
  w1 = src->w;
  h1 = src->h;
  w = dst->w;
  h = dst->h;
  x0 = w1 / 2;
  y0 = h1 / 2;
  angle = (angle / 180) * 3.1416;
  si = sin(angle);
  co = cos(angle);
  for(y = 0 ; y < dst->h ; y++)
    for(x = 0 ; x < dst->w; x++)
      for (c = 0; c < 3; c++)
      {
        dx = x - x0;
        dy = y - y0;                                              
        dx1 = (dx * co) - (dy * si);
        dy1 = (dx * si) + (dy * co);
        x1 = x0 + dx1;
        y1 = y0 + dy1;
        if(x1 >= 0 && x1 < w1 && y1 >= 0 && y1 < h1)
          *get(dst, x, y, c) = *get(src, x1, y1, c);
        else
          *get(dst, x, y, c) = 0;
      }
} 

void Xreny(Picture *src, Picture *dst, int r, int x0, int y0)
{
  int x, y , c, w1, h1, w, h, x1, y1, dx, dy;
  double d;
  w1 = src->w;
  h1 = src->h;
  w = dst->w;
  h = dst->h;
  for(y = 0 ; y < dst->h ; y++)
    for(x = 0 ; x < dst->w; x++)
      for (c = 0; c < 3; c++)
      {
        dx = x - x0;
        dy = y - y0;
        d = sqrt(dx * dx + dy * dy);
        x1 = x0 + (dx * d) / r;
        y1 = y0 + (dy * d) / r;
        if (d < r)
          *get(dst, x, y, c) = *get(src, x1, y1, c);
        else
          *get(dst, x, y, c) = *get(src, x, y, c);
      }
}
void Blur(Picture *src, Picture *dst, int r)
{
  int sum = 0, x, y, u, v, c; 
  for(y = 0; y < src->h; y++)
    for(x = 0; x < src->w ; x++)
      for (c = 0; c < 3; c++)
      {
        for(u = y - r ; u <= y + r ; u++)
          for(v = x - r ; v <= x + r ; v++)
          
              sum += *get(src, u , v, c);
       *get(dst, y , x, c) = sum / ((2 * r + 1) * (2 * r + 1));
       sum = 0;
      }
}                               
int Truncate(int i)
{
   if (i < 0)
     return i = 0;
   else if (i > 255)
     return i = 255;
   else 
     return i;
}  
void Brightness(Picture *src, Picture *dst, int d)
{
  int x, y , c;
  for(y = 0; y < src->h ; y++)
    for(x = 0; x < src->w; x++)
      for (c = 0; c < 3; c++)
        dst->colors[(y * src->w + x) * 3 + c] = Truncate(src->colors[(y * src->w + x) * 3 + c] + d);
}

void Contraste(Picture *src, Picture *dst, int maxc, int minc)
{
  int x, y, c;
  for(y = 0; y < src->h ; y++)
    for(x = 0; x < src->w; x++)
      for (c = 0; c < 3; c++)
      dst->colors[(y * src->w + x) * 3 + c] = Truncate(255 * (*get(src, x, y, c) - minc) / (maxc - minc));
} 


void KeyBoard(byte key, int x, int y)
{
   if (key == 27)
     exit(0);
}
void Display(void)
{
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2d(-1, 1);
  glPixelZoom(1, -1);
  glDrawPixels(MAXW, MAXH, GL_BGR_EXT, GL_UNSIGNED_BYTE, frame);  
  glFinish();
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB);
  glutInitWindowPosition(0, -1);
  glutInitWindowSize(500, 500);
  glutCreateWindow("Picture");
  glutKeyboardFunc(KeyBoard);
  if(!LoadPicture("X:\\PICS\\M.G24", &gibbon))
    return -2;
  glutDisplayFunc(Display);
 
  LoadPicture("X:\\PICS\\M.G24", &gibbon);
  DrawPicture(1, 1, &gibbon);

  CreatePicture(gibbon.w, gibbon.h, &res);
  Negative(&gibbon, &res);
  DrawPicture(gibbon.w, 1, &res);

 
  Blur(&gibbon, &res, 2);
  DrawPicture(1, gibbon.h , &res);   

  Brightness(&gibbon, &res, 50);
  DrawPicture(gibbon.w , gibbon.h , &res);

  Contraste(&gibbon, &res, 200, 100);
  DrawPicture(1, gibbon.h + gibbon.h  , &res);

  CreatePicture(59, 200, &res);
  Resize(&gibbon, &res);
  DrawPicture(gibbon.w, gibbon.h + gibbon.h  , &res);

  CreatePicture(gibbon.w, gibbon.h, &res);
  Rotate(&gibbon, &res, 30);
  DrawPicture(gibbon.w + gibbon.w, 1, &res);

  Xreny(&gibbon, &res, 50, 43, 20);
  DrawPicture(gibbon.w + gibbon.w, gibbon.h, &res);

  CreatePicture(gibbon.w, gibbon.h, &res2);  
  Xreny(&res, &res2, 50, 90, 18);
  DrawPicture(gibbon.w + gibbon.w, gibbon.h + gibbon.h, &res2);

  DesPic(&gibbon);
  DesPic(&res);
  DesPic(&res2);
  glutMainLoop(); 
  return 0;
}

