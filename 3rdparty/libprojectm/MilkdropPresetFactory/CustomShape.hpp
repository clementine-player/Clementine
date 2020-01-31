/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2007 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 */
/**
 * $Id$
 *
 * Encapsulation of a custom shape
 *
 * $Log$
 */

#ifndef _CUSTOM_SHAPE_H
#define _CUSTOM_SHAPE_H

#define CUSTOM_SHAPE_DEBUG 0
#include <map>
#include "Param.hpp"
#include "PerFrameEqn.hpp"
#include "InitCond.hpp"
#include "Renderer/Renderable.hpp"
#include <vector>

class Preset;


class CustomShape : public Shape {
public:
    /* Numerical id */
    int id;
    int per_frame_count;

    /* Parameter tree associated with this custom shape */
    std::map<std::string,Param*> param_tree;

    /* Engine variables */

    bool enabled;

    /* stupid t variables */
    float t1;
    float t2;
    float t3;
    float t4;
    float t5;
    float t6;
    float t7;
    float t8;

    /* stupider q variables */
    float q[NUM_Q_VARIABLES];

    // Data structure to hold per frame  / per frame init equations
    std::map<std::string,InitCond*>  init_cond_tree;
    std::vector<PerFrameEqn*>  per_frame_eqn_tree;
    std::map<std::string,InitCond*>  per_frame_init_eqn_tree;

    std::map<std::string, Param*> text_properties_tree;


    /// Allocate a new custom shape, including param associations, per point equations, and initial values.
    /// \param id an integer id to associate with this custom wave. Future line parsing uses this as a reference key.
    CustomShape( int id );
    CustomShape();
    ~CustomShape();

    void loadUnspecInitConds();
    void evalInitConds();

  };


#endif /** !_CUSTOM_SHAPE_H */

