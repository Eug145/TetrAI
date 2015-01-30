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

#ifndef AIMODULE_B_INTF_AB_H
#define AIMODULE_B_INTF_AB_H

#include "aimodule_a_intf_bb.h"
#include <QVector>
#include <QVarLengthArray>
#include <cstddef>

namespace AIModule {

enum class ONLocation {nodes, args, memory};

enum class FLocation {nodes, result, memory};

class TIndex
{
public:
    int ind;
};

class RIndex
{
public:
    int ind;

public:
    bool operator==(RIndex b);
};

struct GraphPlace
{
    RIndex r;
    TIndex t;
    NodeCore * core;
};

template <typename T>
class SchemeFeaturer
{
    DaemonScheme<T> const * sample;
    DaemonScheme<T> * target;
    Node<T> const * const originals;
    QVarLengthArray<NodeCore, T::graph_size_max> temporaries;
    int * strings, * result_inds, * memory_inds;

    QVarLengthArray<TIndex, T::graph_size_max> original_inds;
    QVarLengthArray<RIndex, T::graph_size_max> transformed_inds;
    QVarLengthArray<TIndex, T::graph_size_max> temporary_inds;
    QVarLengthArray<int, T::graph_size_max> temporary_inds_backup;

    std::size_t graph_size, n, nn, q, q_count;
    int f_num, f_operand, f_required_id;
    GraphPlace f_place, o_place, n_place;
    QVector<RIndex> some_illegals;

public:
    explicit SchemeFeaturer(DaemonScheme<T> const & sa, DaemonScheme<T> * ta);

    void make();

private:
    void prepare();
    void make_single_feature();
    template <FLocation featured_place>
    void make_single_feature_a();
    template <FLocation featured_place, ONLocation old_place>
    void make_single_feature_b();
    void choose_connection();
    void set_temporary_node(GraphPlace & p);
    void set_temporary_node_a(GraphPlace & p);
    template <FLocation featured_place>
    void get_required_id();
    void get_some_illegals();
    NodeCore const * get_node_core(RIndex r);
    RIndex get_reflected_index(int i);
    template <FLocation featured_place, ONLocation old_place>
    void skip_nodes(std::size_t i);
    template <FLocation featured_place, ONLocation old_place>
    void insert_nodes(std::size_t i);
    template <FLocation featured_place = FLocation::nodes, typename ...Op>
    void connect_abroad(GraphPlace & p, GraphPlace & fulcrum, Op ...op);
    bool set_checked_temporary_node(GraphPlace & p);
    void take_next_q();
    template <FLocation featured_place, ONLocation new_place>
    void connect_nodes(GraphPlace & p, GraphPlace & fulcrum, int const op);
    template <FLocation featured_place, ONLocation new_place>
    void connect_nodes(GraphPlace & p, GraphPlace & fulcrum);
    void disconnect_node();
};

template <>
template <FLocation featured_place>
void SchemeFeaturer<SoulTraits>::get_required_id();
template <>
template <FLocation featured_place>
void SchemeFeaturer<AngelTraits>::get_required_id();

}

#endif // AIMODULE_B_INTF_AB_H
