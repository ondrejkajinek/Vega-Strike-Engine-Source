/*
 * vs_vector.h
 *
 * Copyright (C) 2001-2022 Daniel Horn, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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


#ifndef VEGA_STRIKE_SRC_VS_VECTOR_H_
#define VEGA_STRIKE_SRC_VS_VECTOR_H_

#include <cstddef>
#include <memory>
#include <vector>

namespace VegaStrike {

template<class MyType, class Allocator = std::allocator<MyType>>
class vs_vector : public std::vector<MyType, Allocator> {
public:
    vs_vector() = default;
    vs_vector(const vs_vector<MyType, Allocator> &) = default;
    virtual ~vs_vector() = default;
    typedef typename std::vector<MyType, Allocator>::reference vs_vector_reference;
    typedef typename std::vector<MyType, Allocator>::size_type vs_vector_size_type;

    inline vs_vector_reference operator[](vs_vector_size_type pos) {
        return this->at(pos);
    }
};

}

#endif    // VEGA_STRIKE_SRC_VS_VECTOR_H_

