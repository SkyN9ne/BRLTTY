/*
 * BRLTTY - Access software for Unix for a blind person
 *          using a soft Braille terminal
 *
 * Copyright (C) 1995-2000 by The BRLTTY Team, All rights reserved.
 *
 * Web Page: http://www.cam.org/~nico/brltty
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation.  Please see the file COPYING for details.
 */

/* CombiBraille/speech.c - Speech library
 * For Tieman B.V.'s CombiBraille (serial interface only)
 * Maintained by Nikhil Nair <nn201@cus.cam.ac.uk>
 * $Id: speech.c,v 1.2 1996/09/24 01:04:29 nn201 Exp $
 */

#define SPEECH_C 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "brlconf.h"
#include "speech.h"		/* for speech definitions */
#include "../spk.h"
#include "../spk_driver.h"

/* These are shared with CombiBraille/brl.c: */
extern int brl_fd;
extern unsigned char *rawdata;


/* charset conversion table from iso latin-1 == iso 8859-1 to cp437==ibmpc
 * for chars >=128. 
 */
static unsigned char latin2cp437[128] =
  {199, 252, 233, 226, 228, 224, 229, 231,
   234, 235, 232, 239, 238, 236, 196, 197,
   201, 181, 198, 244, 247, 242, 251, 249,
   223, 214, 220, 243, 183, 209, 158, 159,
   255, 173, 155, 156, 177, 157, 188, 21,
   191, 169, 166, 174, 170, 237, 189, 187,
   248, 241, 253, 179, 180, 230, 20, 250,
   184, 185, 167, 175, 172, 171, 190, 168,
   192, 193, 194, 195, 142, 143, 146, 128,
   200, 144, 202, 203, 204, 205, 206, 207,
   208, 165, 210, 211, 212, 213, 153, 215,
   216, 217, 218, 219, 154, 221, 222, 225,
   133, 160, 131, 227, 132, 134, 145, 135,
   138, 130, 136, 137, 141, 161, 140, 139,
   240, 164, 149, 162, 147, 245, 148, 246,
   176, 151, 163, 150, 129, 178, 254, 152};


static void
identspk (void)
{
  puts ("Using the MultiBraille's in-built speech.\n");
}


static void
initspk (void)
{
}


static void
say (unsigned char *buffer, int len)
{
  unsigned char *pre_speech = PRE_SPEECH;
  unsigned char *post_speech = POST_SPEECH;
  unsigned char c;
  int i;

  if (pre_speech[0])
    {
      memcpy (rawdata, pre_speech + 1, pre_speech[0]);
      write (brl_fd, rawdata, pre_speech[0]);
    }
  for (i = 0; i < len; i++)
    {
      c = buffer[i];
      if (c >= 128) c = latin2cp437[c];
      if (c < 33)	/* space or control character */
	{
	  rawdata[0] = ' ';
	  write (brl_fd, rawdata, 1);
	}
      else if (c > MAX_TRANS)
	write (brl_fd, &c, 1);
      else
	{
	  memcpy (rawdata, vocab[c - 33], strlen (vocab[c - 33]));
	  write (brl_fd, rawdata, strlen (vocab[c - 33]));
	}
    }
  if (post_speech[0])
    {
      memcpy (rawdata, post_speech + 1, post_speech[0]);
      write (brl_fd, rawdata, post_speech[0]);
    }
}


static void
mutespk (void)
{
  unsigned char *mute_seq = MUTE_SEQ;

  memcpy (rawdata, mute_seq + 1, mute_seq[0]);
  write (brl_fd, rawdata, mute_seq[0]);
}


static void
closespk (void)
{
}
