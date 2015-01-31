/*
 * PROJECT:  TetrAI
 * VERSION:  0.90
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
 
#include "tetraiworker.h"

QVector<QVector<int>> TetraiWorker::figures
{
    {0, 1, 1, 1, -1,             // 1 1 0
     1, 1, -1,                   // 0 1 0
     1, 1},                      // 0 1 0

    {2, 1, -1,                   // 0 0 1
     0, 1, 1, 1, 1, 1            // 1 1 1
    },                           // 0 0 0

    {1, 1, -1,                   // 0 1 0
     1, 1, -1,                   // 0 1 0
     1, 1, 1, 1},                // 0 1 1

    {-1,                         // 0 0 0
     0, 1, 1, 1, 1, 1, -1,       // 1 1 1
     0, 1},                      // 1 0 0

    {1, 2, 1, 2, -1,             // 0 2 2
     1, 2, -1,                   // 0 2 0
     1, 2},                      // 0 2 0

    {-1,                         // 0 0 0
     0, 2, 1, 2, 1, 2, -1,       // 2 2 2
     2, 2},                      // 0 0 2

    {1, 2, -1,                   // 0 2 0
     1, 2, -1,                   // 0 2 0
     0, 2, 1, 2},                // 2 2 0

    {0, 2, -1,                   // 2 0 0
     0, 2, 1, 2, 1, 2            // 2 2 2
    },                           // 0 0 0

    {0, 3, -1,                   // 3 0 0
     0, 3, 1, 3, -1,             // 3 3 0
     1, 3},                      // 0 3 0

    {1, 3, 1, 3, -1,             // 0 3 3
     0, 3, 1, 3                  // 3 3 0
    },                           // 0 0 0

    {1, 3, -1,                   // 0 3 0
     1, 3, 1, 3, -1,             // 0 3 3
     2, 3},                      // 0 0 3

    {-1,                         // 0 0 0
     1, 3, 1, 3, -1,             // 0 3 3
     0, 3, 1, 3},                // 3 3 0

    {1, 4, -1,                   // 0 4 0
     0, 4, 1, 4, -1,             // 4 4 0
     0, 4},                      // 4 0 0

    {0, 4, 1, 4, -1,             // 4 4 0
     1, 4, 1, 4                  // 0 4 4
    },                           // 0 0 0

    {2, 4, -1,                   // 0 0 4
     1, 4, 1, 4, -1,             // 0 4 4
     1, 4},                      // 0 4 0

    {-1,                         // 0 0 0
     0, 4, 1, 4, -1,             // 4 4 0
     1, 4, 1, 4},                // 0 4 4

    {1, 5, -1,                   // 0 5 0
     0, 5, 1, 5, 1, 5            // 5 5 5
    },                           // 0 0 0

    {1, 5, -1,                   // 0 5 0
     1, 5, 1, 5, -1,             // 0 5 5
     1, 5},                      // 0 5 0

    {-1,                         // 0 0 0
     0, 5, 1, 5, 1, 5, -1,       // 5 5 5
     1, 5},                      // 0 5 0

    {1, 5, -1,                   // 0 5 0
     0, 5, 1, 5, -1,             // 5 5 0
     1, 5},                      // 0 5 0

    {0, 6, 1, 6, -1,             // 6 6
     0, 6, 1, 6},                // 6 6

    {0, 6, 1, 6, -1,             // 6 6
     0, 6, 1, 6},                // 6 6

    {0, 6, 1, 6, -1,             // 6 6
     0, 6, 1, 6},                // 6 6

    {0, 6, 1, 6, -1,             // 6 6
     0, 6, 1, 6},                // 6 6

    {1, 7, -1,                   // 0 7 0 0
     1, 7, -1,                   // 0 7 0 0
     1, 7, -1,                   // 0 7 0 0
     1, 7},                      // 0 7 0 0

    {-1,                         // 0 0 0 0
     0, 7, 1, 7, 1, 7, 1, 7      // 7 7 7 7
                                 // 0 0 0 0
    },                           // 0 0 0 0

    {2, 7, -1,                   // 0 0 7 0
     2, 7, -1,                   // 0 0 7 0
     2, 7, -1,                   // 0 0 7 0
     2, 7},                      // 0 0 7 0

    {-1,                         // 0 0 0 0
     -1,                         // 0 0 0 0
     0, 7, 1, 7, 1, 7, 1, 7      // 7 7 7 7
    },                           // 0 0 0 0
};

QVector<int> TetraiWorker::figure_start_xpos
{
    -1, -2, -1, -1, -2, -1, -2
};

QVector<int> TetraiWorker::figure_start_ypos
{
    -3, -3, -3, -3, -2, -2, -4
};

