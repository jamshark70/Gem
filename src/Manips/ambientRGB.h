/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    ambient a gem object

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

// 1307:forum::für::umläute:2000

#ifndef _INCLUDE__GEM_MANIPS_AMBIENTRGB_H_
#define _INCLUDE__GEM_MANIPS_AMBIENTRGB_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    ambientRGB

    ambient a gem object

DESCRIPTION

    Inlet for R - "rVal"
    Inlet for G - "gVal"
    Inlet for B - "bVal"

-----------------------------------------------------------------*/
class GEM_EXTERN ambientRGB : public GemBase
{
  CPPEXTERN_HEADER(ambientRGB, GemBase);

public:

  //////////
  // Constructor
  ambientRGB(int argc, t_atom *argv);

protected:

  //////////
  // Destructor
  virtual ~ambientRGB();

  //////////
  // Turn back on the color material
  virtual void    postrender(GemState *state);

  //////////
  // When rendering occurs
  virtual void    render(GemState *state);

  //////////
  // The translation vector (r, g, b, a)
  float           m_vector[4];

  //////////
  // R value changed
  void            rMess(float val);

  //////////
  // G value changed
  void            gMess(float val);

  //////////
  // B value changed
  void            bMess(float val);

  //////////
  // A value changed
  void            aMess(float val);
};

#endif  // for header file
