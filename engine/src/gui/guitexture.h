/*
 * guitexture.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, Mike Byron, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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

#ifndef __GUITEXTURE_H__
#define __GUITEXTURE_H__
#include <string>
#include <gfx/aux_texture.h>
#include "guidefs.h"

#include "vegastrike.h"                 //For OpenGL/gl.h -> GLuint.
#include "preferred_types.h"

//The GuiTexture class encapsulates an OpenGL 2D texture.
//There is a cache so that a texture is only read and bound once.
class GuiTexture {
public:
//Read a texture from a file and bind it.
    bool read(const std::string &fileName);

//Draw this texture, stretching to fit the rect.
    void draw(const Rect &rect) const;

//CONSTRUCTION
    GuiTexture();
    ~GuiTexture();

protected:
//INTERNAL IMPLEMENTATION

protected:
//VARIABLES
    vega_types::SharedPtr<Texture> m_texture{};
/*
 *  GLuint m_glName;		// Unique ID for this texture used in OpenGL.
 *  std::string m_fileName;	// Name of the file this came from.
 *  int m_width, m_height;	// Original size of image.
 */
};

#endif   //__GUITEXTURE_H__

