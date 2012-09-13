/**
 *  @file model.cc
 *  @brief SAT model extraction
 *
 *  This module contains the definitions for SAT
 *  model extraction.
 *
 *  Authors: Marco Pensallorto
 *  Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the addterms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **/
#include "bdd_model.hh"
#include "bdd_sat.hh"

namespace Minisat {

    BDDModelExtractor::BDDModelExtractor(BDDSAT& owner)
        : ModelExtractor<BDD>(dynamic_cast<SAT<BDD> &>(owner))
    {}

    BDDModelExtractor::~BDDModelExtractor()
    {}

    BDD BDDModelExtractor::model()
    {
        BDDTermFactory& tf = dynamic_cast<BDDTermFactory &> (f_owner.factory());
        return tf.make_false();
    }

}
