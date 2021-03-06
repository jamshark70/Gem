/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

        Kill particles that are too slow

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_KILLSLOW_H_
#define _INCLUDE__GEM_PARTICLES_PART_KILLSLOW_H_

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

        part_killslow

        Kill particles that are too slow

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN part_killslow : public partlib_base
{
  CPPEXTERN_HEADER(part_killslow, partlib_base);

public:

  //////////
  // Constructor
  part_killslow(t_floatarg num);

  //////////
  virtual void    renderParticles(GemState *state);

protected:

  //////////
  // Destructor
  virtual ~part_killslow(void);

  //////////
  void                      speedMess(float num);

  //////////
  float                     m_killSpeed;
};

#endif  // for header file
