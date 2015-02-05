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

#include "aimodule_b.h"

#include <QtGlobal>
#include <random>

namespace AIModule {

std::uniform_int_distribution<qint32> random_nodes_number;

template <typename T>
SchemeFeaturer<T>::SchemeFeaturer(DaemonScheme<T> const & sa,
                                  DaemonScheme<T> * ta) :
    sample {&sa}, target {ta}, originals {sa.graph}, strings {ta->strings},
    result_inds {ta->result_inds}, memory_inds {ta->memory_inds},
    graph_size {sa.graph_size}, n {sa.graph_size_n}, nn {sa.graph_size_nn}
    /* n {s.graph_size + T::all_args_size + T::memory_size},
    nn {s.graph_size + T::result_size + T::memory_size}
    nn {s.graph_size*Consts::operand_number_max} */
{
    for (qint32 i {0}; i < graph_size; ++i) {
        TIndex t;
        RIndex r;
        t.ind = r.ind = i;
        original_inds.append(t);
        transformed_inds.append(r);
    }
    prepare();
}

template <typename T>
inline void SchemeFeaturer<T>::prepare()
{
    std::copy(sample->strings, sample->strings + T::strings_number, strings);
    std::copy(sample->result_inds, sample->result_inds + T::result_size,
              result_inds);
    std::copy(sample->memory_inds, sample->memory_inds + T::memory_size,
              memory_inds);
}

template <typename T>
void SchemeFeaturer<T>::SchemeFeaturer::make()
{
    static_assert(std::numeric_limits<unsigned int>::max() >
                  (T::graph_size_max + T::result_size + T::memory_size)*
                  Consts::operands_number_max, "Consts are too large (f_num).");
    static_assert(std::numeric_limits<unsigned int>::max() >
                  (T::graph_size_max + T::all_args_size + T::memory_size),
                  "Consts are too large (n_place).");

    for (unsigned int i {random_u_integer(reng)%
                         T::features_number}; i > 0; --i) {
        make_single_feature();
    }
}

template <typename T>
inline void SchemeFeaturer<T>::make_single_feature()
{
    choose_connection();
    some_illegals.clear();
    if (f_place.t.ind < graph_size /* f_num < nn */) {
        /* f_place = f_num/Consts::operand_number_max;
        f_operand = f_num%Consts::operand_number_max; */
        set_temporary_node(f_place);
        //TODO remove the next two lines
        f_num /= nn;
        f_operand = f_num%f_place.core->get_operands_number();
        o_place.r.ind = f_place.core->operands_inds[f_operand];
        get_required_id<FLocation::nodes>();
        get_some_illegals();
        make_single_feature_a<FLocation::nodes>();
    } else if ((f_place.t.ind -= graph_size/*= f_num - nn*/) < T::result_size) {
        o_place.r.ind = result_inds[f_place.t.ind];
        get_required_id<FLocation::result>();
        make_single_feature_a<FLocation::result>();
    } else {
        f_place.t.ind -= T::result_size;
        o_place.r.ind = memory_inds[f_place.t.ind];
        get_required_id<FLocation::memory>();
        make_single_feature_a<FLocation::memory>();
    }
}

template <typename T>
inline void SchemeFeaturer<T>::choose_connection()
{
    //TODO change the way you get f_num
    f_num = random_u_integer(reng);
    /* f_num = .... */
    //TODO remove this line
    f_place.t.ind = f_num%nn;
}

template <typename T>
inline void SchemeFeaturer<T>::set_temporary_node(GraphPlace & p)
{
    p.r = transformed_inds.at(p.t.ind);
    if (p.r.ind < T::graph_size_max) {
        set_temporary_node_a(p);
    } else {
        p.core = &temporaries[p.r.ind - T::temporary_inds_start];
    }
}

template <typename T>
inline void SchemeFeaturer<T>::set_temporary_node_a(GraphPlace & p)
{
    temporaries.append(originals[p.r.ind]);
    temporary_inds_backup.append(p.r.ind);
    temporary_inds.append(p.t);
    RIndex corr_r;
    corr_r.ind = temporaries.size() - 1 + T::temporary_inds_start;
    transformed_inds[p.t.ind] = corr_r;
    p.r = corr_r;
    p.core = &temporaries.last();
}

template <>
template <FLocation featured_place>
inline void SchemeFeaturer<SoulTraits>::get_required_id()
{
    //TODO ....
    /* f_required_id = */
}

template <>
template <FLocation featured_place>
inline void SchemeFeaturer<AngelTraits>::get_required_id()
{
    //TODO ....
    /* f_required_id = */
}

template <typename T>
void SchemeFeaturer<T>::get_some_illegals()
{
    QVarLengthArray<RIndex> queue;
    RIndex r {f_place.r.ind};
    do {
        if (r.ind != -1) {
            some_illegals.append(r);
            NodeCore const * const curr_core {get_node_core(r)};
            if (curr_core->actual_subgraph_id <= f_required_id) {
                for (qint32 i {0}; i < Consts::operands_number_max; ++i) {
                    QVector<qint32>::const_iterator si {curr_core->
                                                   recipients_inds[i].cbegin()};
                    QVector<qint32>::const_iterator ei {curr_core->
                                                   recipients_inds[i].end()};
                    while (si != ei) {
                        queue.append(get_reflected_index(*si));
                        ++si;
                    }
                }
            }
        }
        if (queue.isEmpty()) { break; }
        r.ind = queue.last().ind; queue.removeLast();
    } while (true);
}

template <typename T>
inline NodeCore const * SchemeFeaturer<T>::get_node_core(RIndex r)
{
    Q_ASSERT(r.ind >= 0);
    if (r.ind < T::graph_size_max) {
        return &originals[r.ind];
    }
    Q_ASSERT(r.ind >= T::temporary_inds_start);
    return &temporaries.at(r.ind - T::temporary_inds_start);
}

template <typename T>
inline RIndex SchemeFeaturer<T>::get_reflected_index(qint32 i)
{
    RIndex r;
    if (i < T::graph_size_max) {
        qint32 t {original_inds.at(i).ind};
        if (t < 0) {
            r.ind = -1;
        } else {
            r = transformed_inds.at(t);
        }
    } else {
        r.ind = i;
    }
    return r;
}

/* template <typename T>
inline TIndex SchemeFeaturer<T>::get_transformed_index(RIndex r)
{
    TIndex t;
    if (r.ind < T::graph_size_max) {
        t.ind = original_inds.at(r.ind).ind;
    } else {
        t.ind = temporary_inds.at(r.ind - T::temporary_inds_start);
    }
    return t;
} */

template <typename T>
template <FLocation featured_place>
inline void SchemeFeaturer<T>::make_single_feature_a()
{
    q = random_u_integer(reng);
    q_count = std::numeric_limits<unsigned int>::max()/n;

    static_assert(T::nodes_skip_max >= 1 && T::nodes_insert_max >= 1, "Err!");

    if (graph_size == T::graph_size_max) {
        skip_nodes<featured_place>(random_nodes_number(reng,
                            std::uniform_int_distribution<qint32>::param_type{1,
                                                           T::nodes_skip_max}));
    } else {
        Q_ASSERT(graph_size < T::graph_size_max);
        qint32 i {random_nodes_number(reng,
                            std::uniform_int_distribution<qint32>::param_type{1,
                           T::nodes_skip_max + qMin(qint32{T::nodes_insert_max},
                                             T::graph_size_max - graph_size)})};
        if(i <= T::nodes_skip_max) {
            skip_nodes<featured_place>(i);
        } else {
            insert_nodes<featured_place>(i - T::nodes_skip_max);
        }
    }
}

template <typename T>
template <FLocation featured_place>
void SchemeFeaturer<T>::skip_nodes(qint32 i)
{
    //TODO .... think about it
    connect_abroad(o_place, n_place, static_cast<int>(i));

    if (featured_place == FLocation::nodes) {
        disconnect_node_string();
    }
}

template <typename T>
void SchemeFeaturer<T>::disconnect_node_string()
{
    QVarLengthArray<RIndex> queue;
    GraphPlace a {o_place};
    Connection b {f_place.r, f_operand};
    disconnect_node_string_a(a, b, queue);
    while (!queue.isEmpty()) {
        b.node = queue.last(); queue.removeLast();
        int s {get_node_core(b.node)->get_operands_number()};
        for (int i {0}; i < s; ++i) {
            //TODO ....
        }
    }
}

template <typename T>
inline void SchemeFeaturer<T>::disconnect_node_string_a(GraphPlace & a,
                                Connection & b, QVarLengthArray<RIndex> & queue)
{
    if (a.r.ind < T::graph_size_max) {
        a.t = original_inds.at(a.r.ind);
        Q_ASSERT(a.t.ind >= 0);
        a.r = transformed_inds.at(a.t.ind);
        if (a.r.ind < T::graph_size_max) {
            disconnect_node_string_b<ONLocation::origs>(a, b, queue);
        } else {
            disconnect_node_string_b<ONLocation::temps>(a, b, queue);
        }
    }
    if (a.r.ind >= T::temporary_inds_start) {
        disconnect_node_string_b<ONLocation::temps>(a, b, queue);
    }
}

template <typename T>
template <ONLocation a_place>
inline void SchemeFeaturer<T>::disconnect_node_string_b<a_place>(GraphPlace & a,
                                Connection & b, QVarLengthArray<RIndex> & queue)
{
    Q_ASSERT(a_place == ONLocation::origs || a_place == ONLocation::temps);
    NodeCore const * a_ccore;
    NodeCore * a_core;
    if (a_place == ONLocation::origs) {
        a_ccore = &originals[a.r.ind];
    } else {
        int i {a.r.ind - T::temporary_inds_start};
        a_ccore = a_core = &temporaries[i];
    }
    Q_ASSERT(a_ccore->recepients_number >= 1);
    if (a_ccore->recepients_number == 1) {
        //TODO ....
    } else {
        if (a_place == ONLocation::origs) {
            set_temporary_node_a();
            disconnect_node(a.core, b);
        } else {
            disconnect_node(a_core, b);
        }
    }
}

template <typename T>
inline void SchemeFeaturer<T>::disconnect_node(NodeCore * const a_core,
                                               Connection & b)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    QVarLengthArray<qint32, 1> * r_indices {&a_core->
                                            recipients_inds[b.operand]};
    QVarLengthArray<qint32, 1>::const_iterator si {r_indices->cbegin()};
    QVarLengthArray<qint32, 1>::const_iterator ei {r_indices->cbegin()};
    Q_ASSERT(b.node.ind >= T::temporary_inds_start);
    qint32 i {temporary_inds_backup.at(b.node.ind -
                                       T::temporary_inds_start)};
    while (*si != i && *si != b.node.ind) {
        Q_ASSERT(si != ei);
        ++si;
    }
    r_indices->erase(si);
}

template <typename T>
template <FLocation featured_place>
void SchemeFeaturer<T>::insert_nodes(qint32 i)
{
    if (o_place.r.ind < T::graph_size_max) {
        o_place.t = original_inds.at(o_place.r.ind);
        Q_ASSERT(o_place.t.ind >= 0);
        set_temporary_node(o_place);
        //TODO .... think about it

    } else if (o_place.r.ind >= T::temporary_inds_start) {
        Q_ASSERT(o_place.r.ind >= T::temporary_inds_start);
        qint32 i {o_place.r.ind - T::temporary_inds_start};
        o_place.t = temporary_inds.at(i);
        o_place.core = &temporaries[i];

        //TODO .... think about it
    }


    connect_abroad(o_place, n_place, static_cast<int>(i));

    if ((old_place == ONLocation::origs || old_place == ONLocation::temps) &&
                                           featured_place == FLocation::nodes) {
        disconnect_node();
    }
}

template <typename T>
template <FLocation featured_place, typename ...Op>
inline void SchemeFeaturer<T>::connect_abroad(GraphPlace & p,
                                              GraphPlace & fulcrum, Op ...op)
{
    do {
        p.t.ind = q%n;
        if (p.t.ind < graph_size) {
            if (set_checked_temporary_node(p)) {
                connect_nodes<featured_place,
                              ONLocation::temps>(p, fulcrum, op...);
                break;
            }
        } else if ((p.r.ind = p.t.ind - graph_size) < T::all_args_size) {
            p.r.ind += T::graph_size_max;
            connect_nodes<featured_place,
                          ONLocation::args>(p, fulcrum, op...);
            break;
        } else {
            p.r.ind += T::graph_size_max;
            connect_nodes<featured_place,
                          ONLocation::memory>(p, fulcrum, op...);
            break;
        }
        take_next_q();
    } while (true);
}

template <typename T>
inline bool SchemeFeaturer<T>::set_checked_temporary_node(GraphPlace & p)
{
    p.r = transformed_inds.at(p.t.ind);
    if (p.r.ind < T::graph_size_max) {
        if (originals[p.r.ind].actual_subgraph_id <= f_required_id &&
                                             some_illegals.indexOf(p.r) == -1) {
            set_temporary_node_a(p);
            return true;
        }
    } else {
        qint32 i {p.r.ind - T::temporary_inds_start};
        if (temporaries.at(i).actual_subgraph_id <= f_required_id &&
                                             some_illegals.indexOf(p.r) == -1) {
            p.core = &temporaries[i];
            return true;
        }
    }
    return false;
}

template <typename T>
inline void SchemeFeaturer<T>::take_next_q()
{
    q /= n; --q_count;
    if (q_count == 0) {
        q = random_u_integer(reng);
        q_count = std::numeric_limits<unsigned int>::max()/n;
    }
}

template <typename T>
template <FLocation featured_place, ONLocation new_place>
inline void SchemeFeaturer<T>::connect_nodes(GraphPlace & a,
                                             GraphPlace & b, qint32 const op)
{
    Q_ASSERT(featured_place == FLocation::nodes);
    b.core->operands_inds[op] = a.r.ind;
    if (new_place == ONLocation::temps) {
        a.core->recipients_inds[op].append(b.r.ind);
    }
}

template <typename T>
template <FLocation featured_place, ONLocation new_place>
inline void SchemeFeaturer<T>::connect_nodes(GraphPlace & a,
                                             GraphPlace & b)
{
    Q_ASSERT(featured_place != FLocation::nodes);
    if (featured_place == FLocation::result) {
        result_inds[b.t.ind] = a.r.ind;
    } else {
        memory_inds[b.t.ind] = a.r.ind;
    }
}

inline bool RIndex::operator==(RIndex b)
{
    return ind == b.ind;
}

inline qint32 NodeCore::get_operands_number() const
{
    return Consts::operands_numbers[static_cast<qint32>(n_class) - 1];
}

template
class SchemeFeaturer<AngelTraits>;
template
class SchemeFeaturer<SoulTraits>;

}
