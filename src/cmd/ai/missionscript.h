/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  AI for mission scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#ifndef _CMD_AI_MISSION_SCRIPT_H_
#define _CMD_AI_MISSION_SCRIPT_H_

#include "order.h"
#include "navigation.h"
#include "flybywire.h"

//class AImissionScript : public Order {
class AImissionScript : public FlyByWire {
public:
  ///saves scriptname in the filename var
  AImissionScript(string modulename);
  ~AImissionScript();
  ///Loads the AI script from the hard drive, or executes if loaded
  void Execute();

 protected:
  string modulename;
  unsigned int classid;
  bool first_run;
};

class AIFlyToWaypoint : public AImissionScript {
 public:
  AIFlyToWaypoint(Vector waypoint,float vel,float range);

  Vector waypoint;
  float vel;
  float range;
};

#endif
