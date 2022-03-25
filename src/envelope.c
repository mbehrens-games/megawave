/*******************************************************************************
** envelope.c (amplitude envelope)
*******************************************************************************/

#include <stdlib.h>

#include "envelope.h"

/* this table is from the spritesmind.net forums */
/* gendev.spritesmind.net/forum/viewtopic.php?f=24&t=386&start=105 */
static unsigned char S_ym2612_att_inc_table[64][8] =
  {{0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,1,0,1,0,1,0,1}, {0,1,0,1,0,1,0,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,0,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,0,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {1,1,1,1,1,1,1,1}, {1,1,1,2,1,1,1,2}, {1,2,1,2,1,2,1,2}, {1,2,2,2,1,2,2,2},
   {2,2,2,2,2,2,2,2}, {2,2,2,4,2,2,2,4}, {2,4,2,4,2,4,2,4}, {2,4,4,4,2,4,4,4},
   {4,4,4,4,4,4,4,4}, {4,4,4,8,4,4,4,8}, {4,8,4,8,4,8,4,8}, {4,8,8,8,4,8,8,8},
   {8,8,8,8,8,8,8,8}, {8,8,8,8,8,8,8,8}, {8,8,8,8,8,8,8,8}, {8,8,8,8,8,8,8,8}};

/*******************************************************************************
** envelope_init()
*******************************************************************************/
short int envelope_init(envelope* e)
{
  if (e == NULL)
    return 1;

  e->state  = ENVELOPE_STATE_RELEASE;

  e->a_index  = 0;
  e->d_index  = 0;
  e->s_index  = 0;
  e->r_index  = 0;

  e->total_bound    = 1023;
  e->sustain_bound  = 1023;

  e->attenuation      = 1023;
  e->period           = 1;
  e->cycles           = 0;
  e->increment_index  = 0;

  return 0;
}

/*******************************************************************************
** envelope_create()
*******************************************************************************/
envelope* envelope_create()
{
  envelope* e;

  e = malloc(sizeof(envelope));
  envelope_init(e);

  return e;
}

/*******************************************************************************
** envelope_deinit()
*******************************************************************************/
short int envelope_deinit(envelope* e)
{
  if (e == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** envelope_destroy()
*******************************************************************************/
short int envelope_destroy(envelope* e)
{
  if (e == NULL)
    return 1;

  envelope_deinit(e);
  free(e);

  return 0;
}

/*******************************************************************************
** envelope_setup()
*******************************************************************************/
short int envelope_setup( envelope* e, 
                          unsigned char ar,   unsigned char dr, 
                          unsigned char sr,   unsigned char rr, 
                          unsigned char sl,   unsigned char tl, 
                          int rks, 
                          int kc)
{
  if (e == NULL)
    return 1;

  /* set adsr indices */
  e->a_index = 2 * ar;
  e->d_index = 2 * dr;
  e->s_index = 2 * sr;
  e->r_index = 2 * (2 * rr + 1);

  /* apply rate scaling */
  /* rks 0 */
  if (rks == 0)
  {
    e->a_index += kc / 8;
    e->d_index += kc / 8;
    e->s_index += kc / 8;
    e->r_index += kc / 8;
  }
  /* rks 1 */
  else if (rks == 1)
  {
    e->a_index += kc / 4;
    e->d_index += kc / 4;
    e->s_index += kc / 4;
    e->r_index += kc / 4;
  }
  /* rks 2 */
  else if (rks == 2)
  {
    e->a_index += kc / 2;
    e->d_index += kc / 2;
    e->s_index += kc / 2;
    e->r_index += kc / 2;
  }
  /* rks 3 */
  else if (rks == 3)
  {
    e->a_index += kc;
    e->d_index += kc;
    e->s_index += kc;
    e->r_index += kc;
  }

  /* bound adsr indices */
  if (e->a_index > 63)
    e->a_index = 63;
  if (e->d_index > 63)
    e->d_index = 63;
  if (e->s_index > 63)
    e->s_index = 63;
  if (e->r_index > 63)
    e->r_index = 63;

  /* initialize attentuation */
  e->attenuation = 1023;

  /* set sustain bound (10 bit bounds) */
  if (sl == 15)
    e->sustain_bound = 1023;
  else
    e->sustain_bound = sl << 5;

  if (e->sustain_bound < 0)
    e->sustain_bound = 0;
  else if (e->sustain_bound > 1023)
    e->sustain_bound = 1023;

  /* set total bound (10 bit bounds) */
  e->total_bound = tl << 3;

  if (e->total_bound < 0)
    e->total_bound = 0;
  else if (e->total_bound > 1023)
    e->total_bound = 1023;

  /* set default state */
  e->state = ENVELOPE_STATE_RELEASE;

  return 0;
}

/*******************************************************************************
** envelope_change_state()
*******************************************************************************/
short int envelope_change_state(envelope* e, int state)
{
  if (e == NULL)
    return 1;

  /* make sure this is a valid state */
  if ((state != ENVELOPE_STATE_ATTACK)  && (state != ENVELOPE_STATE_DECAY)  &&
      (state != ENVELOPE_STATE_SUSTAIN) && (state != ENVELOPE_STATE_RELEASE))
    return 1;

  e->state = state;

  /* update period */
  if (e->state == ENVELOPE_STATE_ATTACK)
  {
    if (e->a_index <= 43)
      e->period = 1 << (11 - (e->a_index / 4));
    else
      e->period = 1;
  }
  else if (e->state == ENVELOPE_STATE_DECAY)
  {
    if (e->d_index <= 43)
      e->period = 1 << (11 - (e->d_index / 4));
    else
      e->period = 1;
  }
  else if (e->state == ENVELOPE_STATE_SUSTAIN)
  {
    if (e->s_index <= 43)
      e->period = 1 << (11 - (e->s_index / 4));
    else
      e->period = 1;
  }
  else if (e->state == ENVELOPE_STATE_RELEASE)
  {
    if (e->r_index <= 43)
      e->period = 1 << (11 - (e->r_index / 4));
    else
      e->period = 1;
  }

  /* ym2612 envelope generator clock is 1/3 the fm clock */
  e->period *= 3;

  return 0;
}

/*******************************************************************************
** envelope_update()
*******************************************************************************/
short int envelope_update(envelope* e)
{
  short int increment;

  if (e == NULL)
    return 1;

  /* increment cycles */
  e->cycles++;

  /* ym2612 jumps attack phase when attack index is 62 or 63 */
  if ((e->state == ENVELOPE_STATE_ATTACK) && (e->a_index >= 62))
  {
    e->attenuation = 0;
    envelope_change_state(e, ENVELOPE_STATE_DECAY);
  }

  while (e->cycles >= e->period)
  {
    e->cycles -= e->period;
    e->increment_index = (e->increment_index + 1) % 8;

    /* attack */
    if (e->state == ENVELOPE_STATE_ATTACK)
    {
      increment = S_ym2612_att_inc_table[e->a_index][e->increment_index];
      e->attenuation += (~e->attenuation * increment) >> 4;

      if (e->attenuation <= 0)
      {
        e->attenuation = 0;
        envelope_change_state(e, ENVELOPE_STATE_DECAY);
      }
    }
    /* decay */
    else if (e->state == ENVELOPE_STATE_DECAY)
    {
      increment = S_ym2612_att_inc_table[e->d_index][e->increment_index];
      e->attenuation += increment;

      if (e->attenuation >= e->sustain_bound)
        envelope_change_state(e, ENVELOPE_STATE_SUSTAIN);
    }
    /* sustain */
    else if (e->state == ENVELOPE_STATE_SUSTAIN)
    {
      increment = S_ym2612_att_inc_table[e->s_index][e->increment_index];
      e->attenuation += increment;

      if (e->attenuation >= 1023)
        e->attenuation = 1023;
    }
    /* release */
    else if (e->state == ENVELOPE_STATE_RELEASE)
    {
      increment = S_ym2612_att_inc_table[e->r_index][e->increment_index];
      e->attenuation += increment;

      if (e->attenuation >= 1023)
        e->attenuation = 1023;
    }
  }

  return 0;
}

