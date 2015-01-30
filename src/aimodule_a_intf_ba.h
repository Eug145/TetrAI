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

#ifndef AIMODULE_A_INTF_BA_H
#define AIMODULE_A_INTF_BA_H

#include "tetrai.h"
#include <cstddef>
#include <limits>

namespace AIModule {

struct Consts
{
    static int const score_addr {0};
    static int const next_figure_addr {1};
    static int const new_game_addr {2};
    static int const player_mode_addr {3};
    static int const human_act_addr {4};
    static std::size_t const sensor_a_size {5};
    static std::size_t const sensor_b_size {Tetrai::board_size};
    static std::size_t const sensor_size {sensor_a_size + sensor_b_size};

    static qint32 const any_act {0};

    static unsigned int const planning_interval {32};
    static unsigned int const projecting_interval {planning_interval};
    static std::size_t const operands_number_max {2};
    static std::size_t const operands_numbers[4];
    static std::size_t const bits_number_a {std::numeric_limits<unsigned int>::
                                            digits};
    static std::size_t const bits_number_b {2};

    static double const age_group_tolerance;
};

class Soul;

struct SoulTraits
{
    using DaemonType = Soul;
    using ResultType = qint32;

    static std::size_t const cluster_size {32};

    static std::size_t const graph_size_max {1024};
    static std::size_t const memory_size {64};
    static std::size_t const result_size {Consts::planning_interval};

    static ResultType const result_mask {0x00000007};
    static std::size_t const strings_number {3};

    static std::size_t const all_args_size {Consts::sensor_size + 1};
    static int const temporary_inds_start {graph_size_max +
                                           all_args_size + memory_size};
    static std::size_t const features_number {4};
    static std::size_t const nodes_skip_max {graph_size_max/128};
    static std::size_t const nodes_insert_max {nodes_skip_max};

    static std::size_t const history_capacity {Consts::planning_interval + 1};
};

class Angel;

struct AngelTraits
{
    using DaemonType = Angel;
    using ResultType = quint32;

    static std::size_t const cluster_size {32};

    static std::size_t const graph_size_max {1024};
    static std::size_t const memory_size {128};
    static std::size_t const result_size {Consts::projecting_interval};

    static ResultType const result_mask {0xFFFFFFFF};
    static std::size_t const strings_number {result_size + 2};

    static std::size_t const all_args_size {Consts::sensor_size +
                                                       SoulTraits::result_size};
    static int const temporary_inds_start {graph_size_max +
                                           all_args_size + memory_size};
    static std::size_t const features_number {4};
    static std::size_t const nodes_skip_max {graph_size_max/128};
    static std::size_t const nodes_insert_max {nodes_skip_max};

    static std::size_t const history_capacity {2};
};




template <typename T>
class DaemonSchemeData
{
public:
    QVector<qint32> args;
    qint32 memory[T::memory_size];

    QVarLengthArray<qint32, 1> results;

public:
    explicit DaemonSchemeData(std::size_t sz = 0);
};

}
#endif // AIMODULE_A_INTF_BA_H
