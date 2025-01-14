/*
 * pilot.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "gnuhash.h"
#include <vector>
#include "preferred_types.h"

class Animation;
class Unit;

class Pilot {
    unsigned char gender; //which sound should play
    float reaction_time;
    float rank;
    int faction; //duplicate data...any way round this??
public:
    explicit Pilot(int faction);

    virtual ~Pilot() = default;

    void SetComm(Unit *comm_unit); //so we can specialize base sort of people
    vega_types::SharedPtr<Animation> getCommFace(Unit *parent, float moon, unsigned char &gender);

    float getReactionTime() {
        return reaction_time;
    }

    unsigned char getGender() {
        return gender;
    }

    float getRank() {
        return rank;
    }                          //man it's rank in here
    typedef vsUMap<const void *, float> relationmap; //non dereferencable Unit to float
    relationmap effective_relationship;
    vega_types::SharedPtr<std::vector<vega_types::SharedPtr<Animation>>> comm_face;
    float getAnger(const Unit *parent, const Unit *un) const;

    vega_types::SharedPtr<std::vector<vega_types::SharedPtr<Animation>>> getCommFaces(unsigned char &sex) {
        sex = gender;
        return comm_face;
    }

    float GetEffectiveRelationship(const Unit *parent, const Unit *target) const;
    float adjustSpecificRelationship(Unit *parent,
            void *aggressor,
            float value,
            int guessedFaction /*pass in neutral otherwise*/ );
    void DoHit(Unit *parent, void *aggressor, int guessedFaction /*pass in neutral otherwise*/ );
};

