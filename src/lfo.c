/*******************************************************************************
** lfo.c (vibrato / tremolo)
*******************************************************************************/

#include <stdio.h>    /* testing */
#include <stdlib.h>

#include "global.h"
#include "lfo.h"

/* these tables are adapted from fm2612.cpp in mame */

/* ym2612 vibrato                                                       */
/* fnum: 128 levels from the top 7 bits (only storing 1 level per bit)  */
/* rows: 7 vibrato depths                                               */
/* index: 32 steps in the wave                                          */
static char S_ym2612_vib_table[7][7][32];

/* ym2612 tremolo                 */
/* rows: 3 tremolo depths         */
/* index: 128 steps in the wave   */
static char S_ym2612_trem_table[3][128];

#define LFO_UPDATE_OP_LEVEL(num)                                               \
  if (l->mode == LFO_MODE_VIBRATO)                                             \
  {                                                                            \
    l->level[num] = 0;                                                         \
                                                                               \
    if (l->fnum & 0x40)                                                        \
      l->level[num] += S_ym2612_vib_table[6][l->row[num]][l->index];           \
    if (l->fnum & 0x20)                                                        \
      l->level[num] += S_ym2612_vib_table[5][l->row[num]][l->index];           \
    if (l->fnum & 0x10)                                                        \
      l->level[num] += S_ym2612_vib_table[4][l->row[num]][l->index];           \
    if (l->fnum & 0x08)                                                        \
      l->level[num] += S_ym2612_vib_table[3][l->row[num]][l->index];           \
    if (l->fnum & 0x04)                                                        \
      l->level[num] += S_ym2612_vib_table[2][l->row[num]][l->index];           \
    if (l->fnum & 0x02)                                                        \
      l->level[num] += S_ym2612_vib_table[1][l->row[num]][l->index];           \
    if (l->fnum & 0x01)                                                        \
      l->level[num] += S_ym2612_vib_table[0][l->row[num]][l->index];           \
  }                                                                            \
  else if (l->mode == LFO_MODE_TREMOLO)                                        \
    l->level[num] = S_ym2612_trem_table[l->row[num]][l->index];

/*******************************************************************************
** lfo_init()
*******************************************************************************/
short int lfo_init(lfo* l)
{
  int i;

  if (l == NULL)
    return 1;

  l->mode     = LFO_MODE_VIBRATO;

  l->cycles   = 0;
  l->period   = 0;
  l->index    = 0;

  l->fnum     = 0;

  for (i = 0; i < LFO_NUM_OPS; i++)
  {
    l->row[i]   = 0;
    l->level[i] = 0;
  }

  return 0;
}

/*******************************************************************************
** lfo_create()
*******************************************************************************/
lfo* lfo_create()
{
  lfo* l;

  l = malloc(sizeof(lfo));
  lfo_init(l);

  return l;
}

/*******************************************************************************
** lfo_deinit()
*******************************************************************************/
short int lfo_deinit(lfo* l)
{
  if (l == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** lfo_destroy()
*******************************************************************************/
short int lfo_destroy(lfo* l)
{
  if (l == NULL)
    return 1;

  lfo_deinit(l);
  free(l);

  return 0;
}

/*******************************************************************************
** lfo_update()
*******************************************************************************/
short int lfo_update(lfo* l)
{
  if (l == NULL)
    return 1;

  /* increment cycles */
  l->cycles++;

  /* update index */
  if (l->mode == LFO_MODE_VIBRATO)
  {
    if (l->cycles >= l->period)
    {
      l->index = (l->index + 1) % 32;
      l->cycles -= l->period;
    }
  }
  else if (l->mode == LFO_MODE_TREMOLO)
  {
    if (l->cycles >= l->period)
    {
      l->index = (l->index + 1) % 128;
      l->cycles -= l->period;
    }
  }

  /* update levels */
  LFO_UPDATE_OP_LEVEL(0)
  LFO_UPDATE_OP_LEVEL(1)
  LFO_UPDATE_OP_LEVEL(2)
  LFO_UPDATE_OP_LEVEL(3)

  return 0;
}

/*******************************************************************************
** lfo_generate_tables()
*******************************************************************************/
short int lfo_generate_tables()
{
  int i;
  int j;
  int k;

  /* ym2612 vibrato table */
  /* set up values for fnum 0x40 at each depth, then generate the other fnum  */
  /* amounts within each depth by dividing by 2                               */

  /* depth 7: 80 cents */
  S_ym2612_vib_table[6][6][0] = 0;
  S_ym2612_vib_table[6][6][1] = 0;
  S_ym2612_vib_table[6][6][2] = 32;
  S_ym2612_vib_table[6][6][3] = 48;
  S_ym2612_vib_table[6][6][4] = 64;
  S_ym2612_vib_table[6][6][5] = 64;
  S_ym2612_vib_table[6][6][6] = 80;
  S_ym2612_vib_table[6][6][7] = 96;

  /* depth 6: 40 cents */
  S_ym2612_vib_table[6][5][0] = 0;
  S_ym2612_vib_table[6][5][1] = 0;
  S_ym2612_vib_table[6][5][2] = 16;
  S_ym2612_vib_table[6][5][3] = 24;
  S_ym2612_vib_table[6][5][4] = 32;
  S_ym2612_vib_table[6][5][5] = 32;
  S_ym2612_vib_table[6][5][6] = 40;
  S_ym2612_vib_table[6][5][7] = 48;

  /* depth 5: 20 cents */
  S_ym2612_vib_table[6][4][0] = 0;
  S_ym2612_vib_table[6][4][1] = 0;
  S_ym2612_vib_table[6][4][2] = 8;
  S_ym2612_vib_table[6][4][3] = 12;
  S_ym2612_vib_table[6][4][4] = 16;
  S_ym2612_vib_table[6][4][5] = 16;
  S_ym2612_vib_table[6][4][6] = 20;
  S_ym2612_vib_table[6][4][7] = 24;

  /* depth 4: 14 cents */
  S_ym2612_vib_table[6][3][0] = 0;
  S_ym2612_vib_table[6][3][1] = 0;
  S_ym2612_vib_table[6][3][2] = 4;
  S_ym2612_vib_table[6][3][3] = 8;
  S_ym2612_vib_table[6][3][4] = 8;
  S_ym2612_vib_table[6][3][5] = 8;
  S_ym2612_vib_table[6][3][6] = 12;
  S_ym2612_vib_table[6][3][7] = 16;

  /* depth 3: 10 cents */
  S_ym2612_vib_table[6][2][0] = 0;
  S_ym2612_vib_table[6][2][1] = 0;
  S_ym2612_vib_table[6][2][2] = 4;
  S_ym2612_vib_table[6][2][3] = 4;
  S_ym2612_vib_table[6][2][4] = 8;
  S_ym2612_vib_table[6][2][5] = 8;
  S_ym2612_vib_table[6][2][6] = 12;
  S_ym2612_vib_table[6][2][7] = 12;

  /* depth 2: 6.7 cents */
  S_ym2612_vib_table[6][1][0] = 0;
  S_ym2612_vib_table[6][1][1] = 0;
  S_ym2612_vib_table[6][1][2] = 0;
  S_ym2612_vib_table[6][1][3] = 4;
  S_ym2612_vib_table[6][1][4] = 4;
  S_ym2612_vib_table[6][1][5] = 4;
  S_ym2612_vib_table[6][1][6] = 8;
  S_ym2612_vib_table[6][1][7] = 8;

  /* depth 1: 3.4 cents */
  S_ym2612_vib_table[6][0][0] = 0;
  S_ym2612_vib_table[6][0][1] = 0;
  S_ym2612_vib_table[6][0][2] = 0;
  S_ym2612_vib_table[6][0][3] = 0;
  S_ym2612_vib_table[6][0][4] = 4;
  S_ym2612_vib_table[6][0][5] = 4;
  S_ym2612_vib_table[6][0][6] = 4;
  S_ym2612_vib_table[6][0][7] = 4;

  for (i = 5; i >= 0; i--)
  {
    for (j = 0; j < 7; j++)
    {
      for (k = 0; k < 8; k++)
      {
        S_ym2612_vib_table[i][j][k] = S_ym2612_vib_table[i + 1][j][k] / 2;
      }
    }
  }

  /* generate the rest of the wave */
  for (i = 0; i < 7; i++)
  {
    for (j = 0; j < 7; j++)
    {
      for (k = 0; k < 8; k++)
      {
        S_ym2612_vib_table[i][j][15 - k] = S_ym2612_vib_table[i][j][k];
        S_ym2612_vib_table[i][j][16 + k] = -S_ym2612_vib_table[i][j][k];
        S_ym2612_vib_table[i][j][31 - k] = -S_ym2612_vib_table[i][j][k];
      }
    }
  }

  /* ym2612 tremolo table */
  for (j = 0; j < 64; j++)
  {
    S_ym2612_trem_table[1][127 - j] = S_ym2612_trem_table[1][j] = (char) j;
  }

  for (j = 0; j < 128; j++)
  {
    S_ym2612_trem_table[0][j] = S_ym2612_trem_table[1][j] / 4;
    S_ym2612_trem_table[2][j] = S_ym2612_trem_table[1][j] * 2;
  }

#if 0
  /* testing: print tables */
  printf("YM2612 Vibrato Table:\n");

  for (i = 0; i < 7; i++)
  {
    for (j = 0; j < 7; j++)
    {
      printf("Fnum %d, Row %d:", i, j);

      for (k = 0; k < 32; k++)
      {
        if (k % 16 == 0)
          printf("\n");

        printf("%d ", S_ym2612_vib_table[i][j][k]);
      }

      printf("\n");
    }
  }

  printf("YM2612 Tremolo Table:\n");

  for (i = 0; i < 3; i++)
  {
    printf("Row %d:", i);

    for (j = 0; j < 128; j++)
    {
      if (j % 16 == 0)
        printf("\n");

      printf("%d ", S_ym2612_trem_table[i][j]);
    }

    printf("\n");
  }
#endif

  return 0;
}
