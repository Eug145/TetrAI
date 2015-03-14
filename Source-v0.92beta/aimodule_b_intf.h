/*
 * PROJECT:  AIModule
 * VERSION:  0.08
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

#ifndef AIMODULE_B_INTF_H
#define AIMODULE_B_INTF_H

#include "aimodule_a_intf_bb.h"
#include "aimodule_c.h"
#include <QVector>
#include <QVarLengthArray>
#include <QLinkedList>
#include <iterator>

namespace AIModule {

extern std::uniform_int_distribution<int> random_features_number;
extern std::uniform_int_distribution<qint32> random_steps_number;
extern std::uniform_int_distribution<int> random_node_class;




enum class FLocation {origs, memory, result, temps};

enum class ONLocation {origs, memory, args, temps};




class TIndex
{
public:
    qint32 ind;

public:
    bool operator<(TIndex const b) const;
    bool operator>(TIndex const b) const;
    bool operator<=(TIndex const b) const;

    TIndex & operator+=(qint32 const v);

    TIndex operator+(qint32 const v) const;
    TIndex operator-(qint32 const v) const;
};

struct RIndex
{
    qint32 ind;
};

template <qint32 value = 0>
class Offset
{
protected:
    qint32 val;

public:
    Offset();
    explicit Offset(qint32 const v);

    qint32 operator()(qint32 const ind) const;
    TIndex operator()(TIndex const t) const;
};

template <typename T>
class SchemeFeaturer;

template <typename T, qint32 value = 0>
class ROffset : public Offset<value>
{
public:
    SchemeFeaturer<T> * s_f;

public:
    void set_value(qint32 const v);
    qint32 get_value() const;
    RIndex operator()(RIndex const r) const;
    using Offset<value>::operator();
};

struct GraphPlace
{
    RIndex r;
    TIndex t;
    qint32 ind;
    qint32 ot_ind;
    NodeCore * core;
    NodeCore const * ccore;
};

struct Inserting {
    qint32 ind;
    qint32 * next_strings;
    TIndex t;
    qint32 steps;
};

class Feature
{
public:
    qint64 num;
    FLocation f_place_loc;
    qint32 f_operand, mr_ind;
    GraphPlace f_place, o_place;
    Inserting in;

public:
    bool operator<(TIndex const b) const;
    bool operator<(Feature const & b) const;
};




template <typename T>
class AntiBranch : public T
{
public:
    GraphPlace c_place;

public:
    AntiBranch();
    AntiBranch(TIndex const t);
    AntiBranch(AntiBranch const & a);
    AntiBranch & operator=(AntiBranch const & a);

    bool operator<(TIndex const & value) const;
    bool operator>(TIndex const & value) const;

    void merge(AntiBranch const &);
};

template <typename B>
class Greater
{
public:
    bool operator()(B const & b, TIndex const t) const;
    bool operator()(TIndex const t, B const & b) const;
};

template <typename B>
class Less
{
public:
    bool operator()(B const & b, TIndex const t) const;
};

struct SimpleABTraits
{
    static constexpr bool simple {true};
    static constexpr bool ccore_is_needed {true};
};

using SimpleAntiBranch = AntiBranch<SimpleABTraits>;

struct OrdinaryABTraits
{
    static constexpr bool simple {true};
    static constexpr bool ccore_is_needed {false};
};

using OrdinaryAntiBranch = AntiBranch<OrdinaryABTraits>;

enum class BCorr {none = 0, kind = 1, indices = 2, req_id = 4};

struct BackwCABTraits
{
    static constexpr bool simple {false};
    static constexpr bool ccore_is_needed {false};
};

class BackwardCorrectionAntiBranch : public AntiBranch<BackwCABTraits>
{
public:    
    BCorr correction;

public:
    BackwardCorrectionAntiBranch();
    explicit BackwardCorrectionAntiBranch(BCorr const ncorr);

    using AntiBranch<BackwCABTraits>::operator<;

    void merge(BackwardCorrectionAntiBranch const & b);
};

template <bool forward, typename B, typename T>
class GraphQueue : public QLinkedList<B>
{
public:
    typename QLinkedList<B>::iterator qi;

private:
    using QLinkedList<B>::insert;

    void sort(B const & b);

public:
    using QLinkedList<B>::append;
    using QLinkedList<B>::begin;
    using QLinkedList<B>::end;

    template <typename ...Args>
    explicit GraphQueue(Args ...args);

    template <typename ...Args>
    bool operator()(qint32 const ind,
                    SchemeFeaturer<T> const & sc_f, Args ...args);
};

template <typename T>
using Illegals = GraphQueue<true, SimpleAntiBranch, T>;

template <typename T>
using DisconnectedNodes = GraphQueue<false, OrdinaryAntiBranch, T>;

template <typename T>
using ForwCorrNodes = GraphQueue<true, OrdinaryAntiBranch, T>;

template <typename T>
using BackwCorrNodes = GraphQueue<false, BackwardCorrectionAntiBranch, T>;




class FeatureIterator : public std::iterator<std::forward_iterator_tag, Feature>
{
    using Iter = typename QVarLengthArray<Feature>::iterator;

    Iter itr;

public:
    FeatureIterator();
    explicit FeatureIterator(Iter const i);
    FeatureIterator & operator=(Iter const i);
    TIndex & operator*() const;
    Feature * operator->() const;
    bool operator==(FeatureIterator const & a) const;
    bool operator!=(FeatureIterator const & a) const;
    FeatureIterator & operator++();
    FeatureIterator operator-(qint32 const o);
};




template <typename T>
using InsertedNodes = QVarLengthArray<GraphPlace, T::steps_number_max - 1>;

template <typename T>
using TransformedInds = QVarLengthArray<RIndex, T::graph_size_max +
                                    T::features_number_max*T::steps_number_max>;

template <typename T, bool root>
class LastRecepientChecker;

template <typename T>
class OperandsActualID;

template <typename T, BCorr control>
class RecepientsInvariant;

template <typename T>
class SchemeFeaturer
{    
    DaemonScheme<T> const * const sample;
    DaemonScheme<T> * const target;
    Node<T> const * const originals;
    qint32 * const strings, * const memory_inds, * const result_inds;
    QVarLengthArray<NodeCore, T::graph_size_max> temporaries;

    QVarLengthArray<TIndex, T::graph_size_max> original_inds;
    TransformedInds<T> transformed_inds;
    QVarLengthArray<TIndex, T::graph_size_max> temporary_inds;
    QVarLengthArray<qint32, T::graph_size_max> temporary_inds_backup;
    QVarLengthArray<TIndex> erasings;
    QVarLengthArray<TIndex>::const_iterator ri;
    QVarLengthArray<Feature> features;
    FeatureIterator ni, mi;

    Illegals<T> some_illegals;
    GraphPlace c_place, n_place;
    qint32 graph_size;
    qint32 f_required_id, f_actual_id;
    SubgraphKind f_subgraph_kind;
    TIndex f_tail_start, f_in_start, f_in_finish;
    ROffset<T> moffset, poffset;
    ROffset<T, -1> moffset_minus;
    ROffset<T, 1> moffset_plus;
    qint32 v;

private:
    qint32 get_legal_args_inds_finish();
    template <FLocation featured_place>
    qint32 * get_next_strings(qint32 const nmr_ind) const;

    void make_t_inserting(TIndex & t, QVarLengthArray<Feature>::iterator & si);
    void make_rt_relation(TIndex & t, RIndex & r);

    void prepare_a();
    void prepare_ba(Feature & f, qint64 const nn);
    void prepare_b();
    void prepare_c();

    template <ONLocation place>
    static qint32 get_actual_id(qint32 const ind);
    template <FLocation featured_place>
    static qint32 get_required_id(qint32 const ind);

    template <bool avoidance_is_needed>
    RIndex get_reflected_index(qint32 i) const;
    template <FLocation place>
    NodeCore const * get_node_core(GraphPlace const & p);
    template <ONLocation place>
    NodeCore const * get_node_core(GraphPlace const & p);
    template <bool ot_is_needed, bool reflected, ONLocation place>
    void get_node_core(GraphPlace & p);
    NodeCore const * get_node_core(qint32 const ind) const;
    NodeCore const * get_node_core(RIndex const r) const;
    void set_temporary_node_b(GraphPlace & p);
    void set_temporary_node_a(GraphPlace & p);
    void set_temporary_node(GraphPlace & p);

    template <BCorr ctrl = BCorr::none>
    RecepientsInvariant<T, ctrl> const & get_ri_seeker();
    template <BCorr control>
    RecepientsInvariant<T, control> const & get_ri_seeker(
                                RecepientsInvariant<T, control> const & seeker);
    template <qint32 value>
    qint32 get_v();
    template <qint32 value>
    ROffset<T, value> & get_moffset();

    template <qint32 value, bool buf, typename IterB, typename IterE>
    void scroll_b(IterB const si, IterE const ei);
    template <qint32 value, bool reverse, bool engaged>
    void scroll_d(TIndex const a, TIndex const b);
    template <qint32 value, bool reverse>
    void scroll_c(TIndex const a, TIndex const b);
    template <qint32 value, bool reverse, typename ...V>
    void scroll_a(qint32 const length, V ...val);
    template <qint32 value = 0,
              bool buf = false, bool engaged = false, typename ...V>
    void scroll(TIndex const a, TIndex const b, V ...val);

    template <bool engaged>
    void correct_backward_cb(BackwCorrNodes<T> & queue,
                             GraphPlace const & cu_place);
    void correct_backward_ca(GraphPlace const & cu_place,
                             TIndex const string_start);
    template <bool old, ONLocation place, BCorr ncorr, typename ...S>
    void correct_backward_c(BackwCorrNodes<T> & queue,
                            GraphPlace & cu_place, S ...s);
    template <bool old, ONLocation place, BCorr control,
              BCorr ncorr, typename ...S>
    void correct_backward_bc(BackwCorrNodes<T> & queue,
                             GraphPlace & cu_place, S ...s);
    template <bool old, ONLocation place, BCorr control,
              BCorr ncorr, typename ...S>
    void correct_backward_bb(BackwCorrNodes<T> & queue,
                             GraphPlace & cu_place, S ...s);
    template <bool old, ONLocation place, BCorr control, typename ...S>
    void correct_backward_ba(BackwCorrNodes<T> & queue,
                             GraphPlace & cu_place, S ...s);
    template <bool old, ONLocation place, BCorr control>
    void correct_backward_b(BackwCorrNodes<T> & queue, GraphPlace & cu_place);
    template <bool old, ONLocation place>
    void correct_backward_da(BackwCorrNodes<T> & queue,
                             GraphPlace & cu_place, BCorr const control);
    template <bool old>
    void correct_backward_d(BackwCorrNodes<T> & queue, GraphPlace & cu_place);
    template <bool old>
    void correct_backward_a(BackwCorrNodes<T> & queue);
    template <ONLocation place, BCorr control>
    void correct_backward_e(GraphPlace & p);
    template <ONLocation place>
    void correct_backward_f(GraphPlace & p);
    template <bool old, ONLocation place>
    void correct_backward(GraphPlace & p);

    template <ONLocation place>
    void disconnect_tree_c(DisconnectedNodes<T> & queue, GraphPlace & cu_place);
    template <ONLocation old_place>
    bool check_recipient_is_last(GraphPlace & o_place, GraphPlace & f_place);
    template <ONLocation place>
    bool check_recipient_is_last(GraphPlace & p);
    template <ONLocation old_place, FLocation featured_place>
    void disconnect_tree_ba(DisconnectedNodes<T> & queue, GraphPlace & o_place,
                            GraphPlace & f_place, qint32 const op);
    template <ONLocation place>
    void disconnect_tree_b(DisconnectedNodes<T> & queue,
                           GraphPlace & cu_place);
    void disconnect_tree_d(DisconnectedNodes<T> & queue, GraphPlace & cu_place);
    void disconnect_tree_a(DisconnectedNodes<T> & queue);
    template <ONLocation old_place, FLocation featured_place>
    void disconnect_tree(Feature & f);
    template <FLocation featured_place>
    void disconnect_node(GraphPlace & a, GraphPlace & b, qint32 const op);

    void correct_forward_e(GraphPlace const & p);
    void correct_forward(GraphPlace & p, InsertedNodes<T> const & queue);
    template <FLocation place, bool bigger, typename ...Args>
    void correct_forward_c(ForwCorrNodes<T> & queue,
                           GraphPlace & cu_place, Args ...args);
    template <FLocation place, bool bigger>
    void correct_forward_b(ForwCorrNodes<T> & queue,
                           GraphPlace & cu_place);
    template <bool bigger>
    void correct_forward_d(ForwCorrNodes<T> & queue,
                           GraphPlace & cu_place);
    template <bool bigger>
    void correct_forward_a(ForwCorrNodes<T> & queue);
    template <bool reflected, ONLocation new_place>
    qint32 get_actual_id(GraphPlace & p);
    template <bool reflected, ONLocation new_place, ONLocation old_place>
    void correct_forward(GraphPlace & p, Feature & f);

    template <bool reflected, ONLocation new_place, FLocation featured_place>
    void connect_nodes(GraphPlace & a, Feature const & f);
    template <bool reflected, ONLocation new_place>
    void connect_nodes(GraphPlace & a, GraphPlace & b, qint32 const op);

    bool check_place_is_legal_a(GraphPlace const & p);
    template <ONLocation place>
    bool check_place_is_legal(GraphPlace const & p);
    bool set_checked_temporary_node(GraphPlace & p);
    void connect_abroad(qint32 const op);
    template <FLocation featured_place>
    void connect_insert(Feature & f);
    template <ONLocation old_place, FLocation featured_place>
    void insert_nodes_e(Feature & f, InsertedNodes<T> const & queue);
    void insert_nodes_d();
    void insert_nodes_c();
    void insert_nodes_b();
    template <bool root = false>
    void get_some_illegals_a(GraphPlace const & cu_place);
    template <FLocation featured_place>
    void get_some_illegals(Feature const & f);
    template <FLocation featured_place>
    void insert_nodes_a(Feature const & f);
    template <ONLocation old_place, FLocation featured_place>
    void insert_nodes(Feature & f);

    template <bool reflected, ONLocation new_place,
              ONLocation old_place, FLocation featured_place>
    void connect_skip(Feature & f);
    template <ONLocation old_place, FLocation featured_place>
    void connect_memarg(Feature & f);
    template <ONLocation old_place, FLocation featured_place>
    void skip_nodes_a(Feature & f);
    template <ONLocation old_place,  FLocation featured_place>
    void skip_nodes(Feature & f);

    template <ONLocation old_place, FLocation featured_place>
    void make_single_feature_b(Feature & f);
    template <FLocation featured_place>
    void make_single_feature_a(Feature & f);
    void make_single_feature(Feature & f);

public:
    SchemeFeaturer(DaemonScheme<T> const & sa, DaemonScheme<T> * ta);
    SchemeFeaturer(SchemeFeaturer<T> const &) =delete;
    SchemeFeaturer(SchemeFeaturer<T> &&) =delete;
    SchemeFeaturer & operator=(SchemeFeaturer<T> const &) =delete;
    SchemeFeaturer & operator=(SchemeFeaturer<T> &&) =delete;

    void realize();
    void make();

    template <typename U, BCorr control>
    friend class RecepientsInvariant;

    friend class OperandsActualID<T>;

    template <typename U, bool root>
    friend class LastRecepientChecker;

    template <bool forward, typename B, typename U>
    friend class GraphQueue;

    template <typename U, qint32 value>
    friend class ROffset;
};

template <>
template <ONLocation place>
qint32 SchemeFeaturer<SoulTraits>::get_actual_id(qint32 const);
template <>
template <ONLocation place>
qint32 SchemeFeaturer<AngelTraits>::get_actual_id(qint32 const ind);

template <>
template <FLocation featured_place>
qint32 SchemeFeaturer<SoulTraits>::get_required_id(qint32 const);
template <>
template <FLocation featured_place>
qint32 SchemeFeaturer<AngelTraits>::get_required_id(qint32 const ind);

template <>
qint32 SchemeFeaturer<SoulTraits>::get_legal_args_inds_finish();
template <>
qint32 SchemeFeaturer<AngelTraits>::get_legal_args_inds_finish();

template <>
template <>
qint32 *  SchemeFeaturer<SoulTraits>::
                get_next_strings<FLocation::result>(qint32 const nmr_ind) const;
template <>
template <>
qint32 *  SchemeFeaturer<AngelTraits>::
                get_next_strings<FLocation::result>(qint32 const nmr_ind) const;

template <>
template <>
ROffset<SoulTraits, -1> & SchemeFeaturer<SoulTraits>::get_moffset<-1>();

template <>
template <>
ROffset<SoulTraits, 1> & SchemeFeaturer<SoulTraits>::get_moffset<1>();

template <>
template <>
ROffset<AngelTraits, -1> & SchemeFeaturer<AngelTraits>::get_moffset<-1>();

template <>
template <>
ROffset<AngelTraits, 1> & SchemeFeaturer<AngelTraits>::get_moffset<1>();

}

#endif // AIMODULE_B_INTF_H
