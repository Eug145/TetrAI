/*
 * PROJECT:  AIModule
 * VERSION:  0.06-B001
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
   
#ifndef AIMODULE_B_INTF_AB_H
#define AIMODULE_B_INTF_AB_H

#include "aimodule_a_intf_bb.h"
#include <QVector>
#include <QVarLengthArray>

namespace AIModule {

enum class ONLocation {origs, temps, args, memory};

enum class FLocation {nodes, result, memory};

class TIndex
{
public:
    qint32 ind;
};

class RIndex
{
public:
    qint32 ind;

public:
    bool operator==(RIndex b);
};

struct GraphPlace
{
    RIndex r;
    TIndex t;
    NodeCore * core;
};

struct Connection
{
    RIndex node;
    qint32 operand;
};

template <typename T>
class SchemeFeaturer
{
    DaemonScheme<T> const * sample;
    DaemonScheme<T> * target;
    Node<T> const * const originals;
    QVarLengthArray<NodeCore, T::graph_size_max> temporaries;
    qint32 * strings, * result_inds, * memory_inds;

    QVarLengthArray<TIndex, T::graph_size_max> original_inds;
    QVarLengthArray<RIndex, T::graph_size_max> transformed_inds;
    QVarLengthArray<TIndex, T::graph_size_max> temporary_inds;
    QVarLengthArray<int, T::graph_size_max> temporary_inds_backup;

    qint32 graph_size, n, nn, q, q_count;
    int f_num;
    qint32 f_operand, f_required_id;
    GraphPlace f_place, o_place, n_place;
    QVector<RIndex> some_illegals;

public:
    explicit SchemeFeaturer(DaemonScheme<T> const & sa, DaemonScheme<T> * ta);

    void make();

private:
    void prepare();
    void make_single_feature();
    void choose_connection();
    void set_temporary_node(GraphPlace & p);
    void set_temporary_node_a(GraphPlace & p);
    template <FLocation featured_place>
    void get_required_id();
    void get_some_illegals();
    NodeCore const * get_node_core(RIndex r);
    RIndex get_reflected_index(qint32 i);
    template <FLocation featured_place>
    void make_single_feature_a();
    template <FLocation featured_place>
    void skip_nodes(qint32 i);
    void disconnect_node_string();
    void disconnect_node_string_a(GraphPlace & a, Connection & b,
                                  QVarLengthArray<RIndex> & queue);
    template <ONLocation a_place>
    void disconnect_node_string_b(GraphPlace & a, Connection & b,
                                  QVarLengthArray<RIndex> & queue);
    void disconnect_node(NodeCore * const a_core, Connection & b);
    template <FLocation featured_place>
    void insert_nodes(qint32 i);
    template <FLocation featured_place = FLocation::nodes, typename ...Op>
    void connect_abroad(GraphPlace & p, GraphPlace & fulcrum, Op ...op);
    bool set_checked_temporary_node(GraphPlace & p);
    void take_next_q();
    template <FLocation featured_place, ONLocation new_place>
    void connect_nodes(GraphPlace & p, GraphPlace & fulcrum, qint32 const op);
    template <FLocation featured_place, ONLocation new_place>
    void connect_nodes(GraphPlace & p, GraphPlace & fulcrum);
};

template <>
template <FLocation featured_place>
void SchemeFeaturer<SoulTraits>::get_required_id();
template <>
template <FLocation featured_place>
void SchemeFeaturer<AngelTraits>::get_required_id();

}

#endif // AIMODULE_B_INTF_AB_H
