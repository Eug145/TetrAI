/*
 
* PROJECT:  AIModule
 
* VERSION:  0.06
 
* LICENSE:  GNU Lesser GPL v3 (../LICENSE.txt)
 
* AUTHOR:  (c) 2015 Eugene Zavidovsky
 
* LINK:  https://github.com/Eug145/TetrAI
 
*
 
*  This program is free software: you can redistribute it and/or modify
 
*  it under the terms of the GNU Lesser General Public License as 
*  published by
 the Free Software Foundation, either version 3 of 
*  the License, or
 (at your option) any later version.
 
*
 
*  This program is distributed in the hope that it will be useful,
 
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
 
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 
*  GNU Lesser General Public License for more details.
 
*
 
*  You should have received a copy of the GNU Lesser General Public
*  License
 along with this program. If not, see:
*  <http://www.gnu.org/licenses/>.
 
*/

#ifndef AIMODULE_A_INTF_A_H
#define AIMODULE_A_INTF_A_H

#include <QVector>

namespace AIModule
{

void init();

void set_sensor(unsigned int score, int next_figure, bool new_game,
                int player_mode, int human_act, QVector<int> board);

int do_ai_cycle();

}

#endif // AIMODULE_A_INTF_A_H
