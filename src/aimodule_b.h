/*
 * PROJECT:  AIModule
 * VERSION:  0.06
 * LICENSE:  GNU Lesser GPL v3 (../LICENSE.txt)
 * AUTHOR:  (c) 2015 Eugene Zavidovsky
 * LINK:  https://github.com/Eug145/TetrAI
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License 
 *  along with this program. If not, see: <http://www.gnu.org/licenses/>.
 */
 
#ifndef AIMODULE_B_H
#define AIMODULE_B_H

#include "aimodule_b_intf_aa.h"
#include "aimodule_b_intf_ab.h"

#include <cstddef>

namespace AIModule {

extern std::uniform_int_distribution<std::size_t> random_nodes_number;

}

#endif // AIMODULE_B_H
