/*
 * unit_xml.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "unit_xml.h"
#include "audiolib.h"
#include "xml_support.h"
#include <fstream>
#include <expat.h>
#include <float.h>
#include <limits.h>
#include "configxml.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include <assert.h>
#include "images.h"
#include "xml_serializer.h"
#include "vsfilesystem.h"
#include "gfx/cockpit_generic.h"
#include "unit_collide.h"
#include "unit_generic.h"
#include "gfx/sphere.h"
#include "role_bitmask.h"
#include "cmd/collide2/Stdafx.h"
#include "cmd/collide2/CSopcodecollider.h"
#include "vs_math.h"
#include "mount_size.h"
#include "weapon_info.h"

using namespace XMLSupport;

/*ADDED FOR extensible use of unit pretty print and unit load */
UNITLOADTYPE current_unit_load_mode = DEFAULT;
extern float getFuelConversion();

string KillQuadZeros(string inp) {
    std::string::size_type text = 0;
    while ((text = inp.find(".000000", text)) != string::npos) {
        inp = inp.substr(0, text) + inp.substr(text + 7);
    }
    return inp;
}

string MakeUnitXMLPretty(string str, Unit *un) {
    string writestr;
    if (un) {
        writestr += "Name: " + un->name;
        writestr += " " + un->getFullname();
        Flightgroup *fg = un->getFlightgroup();
        if (fg) {
            writestr += " Designation " + fg->name + " " + XMLSupport::tostring(un->getFgSubnumber());
        }
        writestr += "\n";
    }
    static std::set<string> lookfor;
    if (lookfor.empty()) {
        lookfor.insert("Shie");
        lookfor.insert("Armo");
//lookfor.insert ("Hull");
        lookfor.insert("Reac");
        lookfor.insert("Moun");
        lookfor.insert("Comp");
//lookfor.insert ("Desc");
        lookfor.insert("Engi");
        lookfor.insert("Mane");
        lookfor.insert("Jump");
//lookfor.insert ("Defe");
        lookfor.insert("Stat");
        lookfor.insert("Engi");
//lookfor.insert ("Hold");
        lookfor.insert("Rada");
    }
    std::string::size_type foundpos;
    while ((foundpos = str.find("<")) != string::npos) {
        if (str.size() <= foundpos + 1) {
            break;
        }
        str = str.substr(foundpos + 1);
        if (str.size() > 3) {
            char mycomp[5] = {str[0], str[1], str[2], str[3], 0};
            if (lookfor.find(mycomp) != lookfor.end()) {
                int newline = str.find(">");
                if (newline > 0) {
                    if (str[newline - 1] == '/') {
                        newline -= 1;
                    }
                }
                writestr += KillQuadZeros(str.substr(0, newline) + "\n");
            }
        }
    }
    return writestr;
}

int GetModeFromName(const char *input_buffer) {
    if (strlen(input_buffer) > 3) {
        if (input_buffer[0] == 'a'
                && input_buffer[1] == 'd'
                && input_buffer[2] == 'd') {
            return 1;
        }
        if (input_buffer[0] == 'm'
                && input_buffer[1] == 'u'
                && input_buffer[2] == 'l') {
            return 2;
        }
    }
    return 0;
}

extern bool CheckAccessory(Unit *);

void addShieldMesh(Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg) {
    static bool forceit = XMLSupport::parse_bool(vs_config->getVariable("graphics", "forceOneOneShieldBlend", "true"));
    xml->shieldmesh = Mesh::LoadMesh(filename, Vector(scale, scale, scale), faction, fg);
    if (xml->shieldmesh && forceit) {
        xml->shieldmesh->SetBlendMode(ONE, ONE, true);
        xml->shieldmesh->setEnvMap(false, true);
        xml->shieldmesh->setLighting(true, true);
    }
}

void addRapidMesh(Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg) {
    xml->rapidmesh = Mesh::LoadMesh(filename, Vector(scale, scale, scale), faction, fg);
}

void pushMesh(vega_types::SequenceContainer<vega_types::SharedPtr<Mesh>> &mesh,
              float &randomstartframe,
              float &randomstartseconds,
              const char *filename,
              const float scale,
              int faction,
              class Flightgroup *fg,
              int startframe,
              double texturestarttime) {
    vega_types::SequenceContainer<vega_types::SharedPtr<Mesh>> m = Mesh::LoadMeshes(filename, Vector(scale, scale, scale), faction, fg);
    for (unsigned int i = 0; i < m.size(); ++i) {
        mesh.push_back(m[i]);
        if (startframe >= 0) {
            mesh.back()->setCurrentFrame(startframe);
        } else if (startframe == -2) {
            float r = ((float) rand()) / RAND_MAX;
            mesh.back()->setCurrentFrame(r * mesh.back()->getFramesPerSecond());
        } else if (startframe == -1) {
            if (randomstartseconds == 0) {
                randomstartseconds =
                        randomstartframe * mesh.back()->getNumLOD() / mesh.back()->getFramesPerSecond();
            }
            mesh.back()->setCurrentFrame(randomstartseconds * mesh.back()->getFramesPerSecond());
        }
        if (texturestarttime > 0) {
            mesh.back()->setTextureCumulativeTime(texturestarttime);
        } else {
            float fps = mesh.back()->getTextureFramesPerSecond();
            int frames = mesh.back()->getNumTextureFrames();
            double ran = randomstartframe;
            if (fps > 0 && frames > 1) {
                ran *= frames / fps;
            } else {
                ran *= 1000;
            }
            mesh.back()->setTextureCumulativeTime(ran);
        }
    }
}
