/*
 * PROJECT:  AIModule
 * VERSION:  0.07
 * LICENSE:  GNU Lesser GPL v3 (../LICENSE.txt, ../GPLv3.txt)
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

#ifndef AIMODULE_A_INTF_BA_C_H
#define AIMODULE_A_INTF_BA_C_H

#include "tetrai.h"
#include <limits>

namespace AIModule {

struct Consts
{
    static int const score_addr {0};
    static int const next_figure_addr {1};
    static int const new_game_addr {2};
    static int const player_mode_addr {3};
    static int const human_act_addr {4};
    static qint32 const sensor_a_size {5};
    static qint32 const sensor_b_size {Tetrai::board_size};
    static qint32 const sensor_size {sensor_a_size + sensor_b_size};

    static qint32 const any_act {0};

    static unsigned int const planning_interval {32};
    static unsigned int const projecting_interval {planning_interval};
    static int const new_prj_count_max {4};
    static double const age_group_tolerance;

    static int const node_classes_number {4};
    static qint32 const recepients_supposed_number {16};
    static qint32 const operands_number_max {2};
    static qint32 const operands_number_min {2};
    static qint32 const operands_numbers[4];    
};

class Soul;

struct SoulTraits
{
    using DaemonType = Soul;
    using ResultType = qint32;

    static int const cluster_size {32};

    static qint32 const graph_size_max {1024};
    static qint32 const memory_size {64};
    static qint32 const result_size {Consts::planning_interval};

    static ResultType const result_mask {0x00000007};
    static qint32 const strings_number {3};

    static qint32 const all_args_size {Consts::sensor_size + 1};
    static qint32 const temporary_inds_start {graph_size_max + memory_size +
                                              qMax(all_args_size, result_size)};
    static int const features_number_max {4};
    static qint32 const steps_number_max {graph_size_max/128};
    static constexpr bool bc_parameter {false};

    static int const history_capacity {Consts::planning_interval + 1};
};

class Angel;

struct AngelTraits
{
    using DaemonType = Angel;
    using ResultType = quint32;

    static int const cluster_size {32};

    static qint32 const graph_size_max {1024};
    static qint32 const memory_size {128};
    static qint32 const result_size {Consts::projecting_interval};

    static ResultType const result_mask {0xFFFFFFFF};
    static qint32 const strings_number {result_size + 2};

    static qint32 const all_args_size {Consts::sensor_size +
                                                       SoulTraits::result_size};
    static qint32 const temporary_inds_start {graph_size_max + memory_size +
                                              qMax(all_args_size, result_size)};
    static int const features_number_max {4};
    static qint32 const steps_number_max {graph_size_max/128};
    static constexpr bool bc_parameter {true};

    static int const history_capacity {2};
};




using RandIntP = std::uniform_int_distribution<int>::param_type;

using RandQInt32P = std::uniform_int_distribution<qint32>::param_type;

using RandQInt64P = std::uniform_int_distribution<qint64>::param_type;

}
#endif // AIMODULE_A_INTF_BA_C_H
