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
 
#include "aimodule_b.h"

#include <QtGlobal>
#include <random>

namespace AIModule {

std::uniform_int_distribution<std::size_t> random_nodes_number;

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
    for (int i {0}; i < graph_size; ++i) {
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
    static_assert(std::numeric_limits<std::size_t>::max() >
                  (T::graph_size_max + T::result_size + T::memory_size)*
                  Consts::operands_number_max, "Consts are too large (f_num).");
    static_assert(std::numeric_limits<std::size_t>::max() >
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
template <FLocation featured_place>
inline void SchemeFeaturer<T>::make_single_feature_a()
{
    if (o_place.r.ind < T::graph_size_max) {
        o_place.t = original_inds.at(o_place.r.ind);
        Q_ASSERT(o_place.t.ind >= 0);
        set_temporary_node(o_place);
        make_single_feature_b<featured_place, ONLocation::nodes>();
    } else if (o_place.r.ind < T::graph_size_max + T::all_args_size) {
        make_single_feature_b<featured_place, ONLocation::args>();
    } else if (o_place.r.ind < T::graph_size_max +
                               T::all_args_size + T::memory_size) {
        make_single_feature_b<featured_place, ONLocation::memory>();
    } else {
        Q_ASSERT(o_place.r.ind >= T::temporary_inds_start);
        int i {o_place.r.ind - T::temporary_inds_start};
        o_place.t = temporary_inds.at(i);
        o_place.core = &temporaries[i];
        make_single_feature_b<featured_place, ONLocation::nodes>();
    }
}

template <typename T>
template <FLocation featured_place, ONLocation old_place>
void SchemeFeaturer<T>::make_single_feature_b()
{
    q = random_u_integer(reng);
    q_count = std::numeric_limits<unsigned int>::max()/n;

    static_assert(T::nodes_skip_max >= 1 && T::nodes_insert_max >= 1, "Err!");

    if (graph_size == T::graph_size_max) {
        skip_nodes<featured_place, old_place>(random_nodes_number(reng,
                       std::uniform_int_distribution<std::size_t>::param_type{1,
                                                           T::nodes_skip_max}));
    } else {
        Q_ASSERT(graph_size < T::graph_size_max);
        std::size_t i {random_nodes_number(reng,
                       std::uniform_int_distribution<std::size_t>::param_type{1,
                      T::nodes_skip_max + qMin(std::size_t{T::nodes_insert_max},
                                             T::graph_size_max - graph_size)})};
        if(i <= T::nodes_skip_max) {
            skip_nodes<featured_place, old_place>(i);
        } else {
            insert_nodes<featured_place, old_place>(i - T::nodes_skip_max);
        }
    }
    if (old_place == ONLocation::nodes && featured_place == FLocation::nodes) {
        disconnect_node();
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
        p.core = &temporaries.last();
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
inline void SchemeFeaturer<T>::get_some_illegals()
{
    QVarLengthArray<RIndex> queue;
    RIndex r;
    queue.append(f_place.r);
    while (!queue.isEmpty()) {
        r.ind = queue.last().ind; queue.removeLast();
        if (r.ind == -1) { continue; }
        some_illegals.append(r);
        NodeCore const * const curr_core {get_node_core(r)};
        if (curr_core->actual_subgraph_id > f_required_id) { continue; }
        for (int i {0}; i < Consts::operands_number_max; ++i) {
            QVector<int>::const_iterator si {curr_core->
                                             recipients_inds[i].cbegin()};
            QVector<int>::const_iterator ei {curr_core->
                                             recipients_inds[i].end()};
            while (si != ei) {
                queue.append(get_reflected_index(*si));
                ++si;
            }
        }
    }
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
inline RIndex SchemeFeaturer<T>::get_reflected_index(int i)
{
    RIndex r;
    if (i < T::graph_size_max) {
        int t {original_inds.at(i).ind};
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

template <typename T>
template <FLocation featured_place, ONLocation old_place>
inline void SchemeFeaturer<T>::skip_nodes(std::size_t i)
{
    //TODO .... think about it
    connect_abroad(o_place, n_place, static_cast<int>(i));
}

template <typename T>
template <FLocation featured_place, ONLocation old_place>
inline void SchemeFeaturer<T>::insert_nodes(std::size_t i)
{
    //TODO .... think about it
    connect_abroad(o_place, n_place, static_cast<int>(i));
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
                              ONLocation::nodes>(p, fulcrum, op...);
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
            p.core = &temporaries.last();
            return true;
        }
    } else {
        int i {p.r.ind - T::temporary_inds_start};
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
                                             GraphPlace & b, int const op)
{
    Q_ASSERT(featured_place == FLocation::nodes);
    b.core->operands_inds[op] = a.r.ind;
    if (new_place == ONLocation::nodes) {
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

template <typename T>
inline void SchemeFeaturer<T>::disconnect_node()
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    QVarLengthArray<int, 1> * r_indices {&o_place.core->
                                         recipients_inds[f_operand]};
    QVarLengthArray<int, 1>::const_iterator si {r_indices->cbegin()};
    QVarLengthArray<int, 1>::const_iterator ei {r_indices->cbegin()};
    Q_ASSERT(f_place.r.ind >= T::temporary_inds_start);
    int f_place_i {temporary_inds_backup.at(f_place.r.ind -
                                            T::temporary_inds_start)};
    while (si != ei) {
        if (*si == f_place_i || *si == f_place.r.ind) { break; }
        ++si;
    }
    Q_ASSERT(si != ei);
    r_indices->erase(si);
}

inline bool RIndex::operator==(RIndex b)
{
    return ind == b.ind;
}

inline int NodeCore::get_operands_number() const
{
    return Consts::operands_numbers[static_cast<int>(n_class) - 1];
}

template
class SchemeFeaturer<AngelTraits>;
template
class SchemeFeaturer<SoulTraits>;

}
