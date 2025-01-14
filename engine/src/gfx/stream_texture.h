/*
 * stream_texture.h
 *
 * Copyright (c) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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


#include "aux_texture.h"

class StreamTexture : public Texture {
    unsigned char *mutabledata;
    FILTER filtertype;
    int handle;
public:
    StreamTexture(int width, int height, enum FILTER filtertype, unsigned char *origdata);
    ~StreamTexture() override;
    unsigned char *Map();
    void UnMap(bool changed = true);
    void MakeActive(int stage) override;

    void MakeActive() override {
        MakeActive(this->stage);
    }

    vega_types::SharedPtr<Texture> Clone() override;
    vega_types::SharedPtr<Texture> Original() override;
    const vega_types::SharedPtr<const Texture> OriginalConst() const override;
};

