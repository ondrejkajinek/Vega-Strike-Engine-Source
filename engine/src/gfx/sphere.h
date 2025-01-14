/*
 * sphere.h
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


#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "mesh.h"
#include "quaternion.h"
#include <cassert>
#include <string>

#ifndef M_PI
#define M_PI (3.1415926536F)
#endif

class SphereMesh : public Mesh {
//no local vars allowed
protected:
    virtual float GetT(float rho, float rho_min, float rho_max) const;
    virtual float GetS(float theta, float theta_min, float theta_max) const;

//    virtual Mesh *AllocNewMeshesEachInSizeofMeshSpace(int num) {
//        static_assert(sizeof(Mesh) == sizeof(*this), "Are SphereMesh and Mesh the same size in memory?");
//        return new SphereMesh[num];
//    }

public:
    SphereMesh() : Mesh() {
        setConvex(true);
    }

    virtual int MeshType() const {
        return 1;
    }

    virtual void SelectCullFace(int whichdrawqueue);
    virtual void RestoreCullFace(int whichdrawqueue);

public:
    static std::string truncateByPipe(std::string &input);
    static vega_types::SharedPtr<SphereMesh> createSphereMesh(float radius,
                                                              int stacks,
                                                              int slices,
                                                              const char *texture,
                                                              const std::string &technique,
                                                              const char *alpha = NULL,
                                                              bool inside_out = false,
                                                              const BLENDFUNC a = ONE,
                                                              const BLENDFUNC b = ZERO,
                                                              bool env_map = false,
                                                              float rho_min = 0.0,
                                                              float rho_max = M_PI,
                                                              float theta_min = 0.0,
                                                              float theta_max = 2 * M_PI,
                                                              FILTER mipmap = MIPMAP,
                                                              bool reverse_normals = false);

    void Draw(float lod, bool centered = false, const Matrix &m = identity_matrix);
    virtual void ProcessDrawQueue(size_t whichpass, int which, bool zsort, const QVector &sortctr);

protected:
    static vega_types::SharedPtr<SphereMesh> constructSphereMesh(vega_types::SharedPtr<SphereMesh> mesh,
                                                                 float radius,
                                                                 int stacks,
                                                                 int slices,
                                                                 const char *texture,
                                                                 const std::string &technique,
                                                                 const char *alpha = NULL,
                                                                 bool inside_out = false,
                                                                 const BLENDFUNC a = ONE,
                                                                 const BLENDFUNC b = ZERO,
                                                                 bool env_map = false,
                                                                 float rho_min = 0.0,
                                                                 float rho_max = M_PI,
                                                                 float theta_min = 0.0,
                                                                 float theta_max = 2 * M_PI,
                                                                 FILTER mipmap = MIPMAP,
                                                                 bool reverse_normals = false,
                                                                 bool subclass = false);
    static std::string const
    calculateHashName(const char *texture, const std::string &technique, int stacks, int slices, const BLENDFUNC a,
                      const BLENDFUNC b, float rho_min, float rho_max);
    static uint64_t const calculateHowManyLevelsOfDetail(int stacks, int slices);
    static vega_types::SharedPtr<Mesh> loadFreshLevelOfDetail(vega_types::SharedPtr<SphereMesh> mesh,
                                                              uint64_t l,
                                                              float radius,
                                                              int &stacks,
                                                              int &slices,
                                                              const char *texture,
                                                              const string &technique,
                                                              const char *alpha,
                                                              bool inside_out,
                                                              const BLENDFUNC a,
                                                              const BLENDFUNC b,
                                                              bool env_map,
                                                              float rho_min,
                                                              float rho_max,
                                                              float theta_min,
                                                              float theta_max,
                                                              FILTER mipmap,
                                                              bool reverse_normals,
                                                              bool subclass);
};

class CityLights : public SphereMesh {
//no local vars allowed
//these VARS BELOW ARE STATIC...change it and DIE
    static float wrapx;
    static float wrapy;
protected:
    virtual float GetT(float rho, float rho_min, float rho_max) const;
    virtual float GetS(float theta, float theta_min, float theta_max) const;

//    Mesh *AllocNewMeshesEachInSizeofMeshSpace(int num) override {
//        static_assert(sizeof(Mesh) == sizeof(*this), "Are CityLights and Mesh the same size in memory?");
//        return new CityLights[num];
//    }

public:
    CityLights() : SphereMesh() {
    }

    virtual void ProcessDrawQueue(size_t whichpass, int which, bool zsort, const QVector &sortctr);

    static vega_types::SharedPtr<CityLights> createCityLights(float radius,
                                                              int stacks,
                                                              int slices,
                                                              const char *texture,
                                                              int zzwrapx,
                                                              int zzwrapy,
                                                              bool insideout,
                                                              const BLENDFUNC a,
                                                              const BLENDFUNC b,
                                                              bool envMap,
                                                              float rho_min,
                                                              float rho_max,
                                                              float theta_min,
                                                              float theta_max,
                                                              bool reversed_normals);

protected:
    static vega_types::SharedPtr<CityLights> constructCityLights(vega_types::SharedPtr<CityLights> city_lights,
                                                                 float radius,
                                                                 int stacks,
                                                                 int slices,
                                                                 const char *texture,
                                                                 int zzwrapx,
                                                                 int zzwrapy,
                                                                 bool insideout,
                                                                 const BLENDFUNC a,
                                                                 const BLENDFUNC b,
                                                                 bool envMap,
                                                                 float rho_min,
                                                                 float rho_max,
                                                                 float theta_min,
                                                                 float theta_max,
                                                                 bool reversed_normals,
                                                                 bool subclass);
};
#endif

