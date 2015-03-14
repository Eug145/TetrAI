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

#include "aimodule_b.h"

#include <QtGlobal>
#include <algorithm>

namespace AIModule {

std::uniform_int_distribution<int> random_features_number;
std::uniform_int_distribution<qint64> random_connection;
std::uniform_int_distribution<qint32> random_steps_number;
std::uniform_int_distribution<qint32> random_operand;
std::uniform_int_distribution<int> random_node_class;
std::uniform_int_distribution<qint32> random_n_place;

constexpr BCorr operator|(BCorr const a, BCorr const b)
{
    return static_cast<BCorr>(static_cast<int>(a)|static_cast<int>(b));
}

constexpr bool test(BCorr const a, BCorr const b)
{
    return static_cast<int>(a)&static_cast<int>(b);
}

inline bool TIndex::operator<(TIndex const b) const
{
    return ind < b.ind;
}

inline bool TIndex::operator>(TIndex const b) const
{
    return ind > b.ind;
}

inline bool TIndex::operator<=(TIndex const b) const
{
    return ind <= b.ind;
}

inline TIndex & TIndex::operator+=(qint32 const v)
{
    ind += v;
    return *this;
}

inline TIndex TIndex::operator+(qint32 const v) const
{
    return TIndex{ind + v};
}

inline TIndex TIndex::operator-(qint32 const v) const
{
    return TIndex{ind - v};
}

template <qint32 value>
inline Offset<value>::Offset()
{
    /* static_assert(value == -1 || value == 1, "Err!"); */
}

template <qint32 value>
inline Offset<value>::Offset(qint32 const v) :
    val {v}
{
    static_assert(value == 0, "Err!");
}

template <qint32 value>
inline TIndex Offset<value>::operator()(TIndex const t) const
{
    if (value != 0) {
        return TIndex{t.ind + value};
    } else {
        return TIndex{t.ind + val};
    }
}

template <qint32 value>
inline qint32 Offset<value>::operator()(qint32 const ind) const
{
    if (value != 0) {
        return ind + value;
    } else {
        return ind + val;
    }
}

template <typename T, qint32 value>
inline void ROffset<T, value>::set_value(qint32 const v)
{
    static_assert(value == 0, "Err!");
    this->val = v;
}

template <typename T, qint32 value>
inline qint32 ROffset<T, value>::get_value() const
{
    static_assert(value == 0, "Err!");
    return this->val;
}

template <typename T, qint32 value>
inline RIndex ROffset<T, value>::operator()(RIndex const r) const
{
    static_assert(value >= -1 || value <= 1, "Err!");
    if (r.ind < T::graph_size_max && r.ind >= 0) {
        if (value > 0) {
            ++s_f->original_inds[r.ind].ind;
        } else if (value < 0){
            --s_f->original_inds[r.ind].ind;
        } else {
            s_f->original_inds[r.ind] += this->val;
        }
    } else if (r.ind > T::temporary_inds_start) {
        if (value > 0) {
            ++s_f->temporary_inds[r.ind].ind;
            ++s_f->original_inds[s_f->temporary_inds_backup[r.ind]].ind;
        } else if (value < 0) {
            --s_f->temporary_inds[r.ind].ind;
            --s_f->original_inds[s_f->temporary_inds_backup[r.ind]].ind;
        } else {
            s_f->temporary_inds[r.ind] += this->val;
            s_f->original_inds[s_f->temporary_inds_backup[r.ind]] += this->val;
        }
    }
    return r;
}

inline bool Feature::operator<(TIndex const t) const
{
    return in.t.ind < t.ind;
}

inline bool Feature::operator<(Feature const & b) const
{
    if (in.ind < b.in.ind) {
        return true;
    } else if (b.in.ind < in.ind) {
        return false;
    } else if (num < b.num) {
        return true;
    }
    /* Q_ASSERT(f_place_loc != FLocation::origs &&
             b.f_place_loc != FLocation::origs && num != b.num ||
       num/Consts::operands_number_max != b.num/Consts::operands_number_max); */
    return false;
}

inline FeatureIterator FeatureIterator::operator-(qint32 const o)
{
    return FeatureIterator{itr - o};
}

inline FeatureIterator & FeatureIterator::operator++()
{
    ++itr; return *this;
}

inline bool FeatureIterator::operator!=(FeatureIterator const & a) const
{
    return itr != a.itr;
}

inline bool FeatureIterator::operator==(FeatureIterator const & a) const
{
    return itr == a.itr;
}

inline Feature * FeatureIterator::operator->() const
{
    return itr;
}

inline TIndex & FeatureIterator::operator*() const
{
    return itr->in.t;
}

inline FeatureIterator & FeatureIterator::operator=(Iter const i)
{
    itr = i; return *this;
}

inline FeatureIterator::FeatureIterator(Iter const i) :
    itr {i}
{

}

inline FeatureIterator::FeatureIterator()
{

}

template <typename T>
inline AntiBranch<T>::AntiBranch()
{

}

template <typename T>
inline AntiBranch<T>::AntiBranch(TIndex const t)
{
    c_place.t.ind = t.ind;
}

template <typename T>
inline AntiBranch<T>::AntiBranch(AntiBranch<T> const & a)
{
    *this = a;
}

template <typename T>
inline AntiBranch<T> & AntiBranch<T>::operator=(AntiBranch<T> const & a)
{
    c_place.t.ind = a.c_place.t.ind;
    if (T::ccore_is_needed) { c_place.ccore = a.c_place.ccore; }
    return *this;
}

template <typename T>
inline bool AntiBranch<T>::operator<(TIndex const & value) const
{
    return c_place.t.ind < value.ind;
}

template <typename T>
inline bool AntiBranch<T>::operator>(TIndex const & value) const
{
    return c_place.t.ind > value.ind;
}

template <typename T>
inline void AntiBranch<T>::merge(AntiBranch const &)
{

}

template <typename B>
inline bool Greater<B>::operator()(B const & b, const TIndex t) const
{
     return b > t;
}

template <typename B>
inline bool Greater<B>::operator()(const TIndex t, B const & b) const
{
     return b < t;
}

template <typename B>
inline bool Less<B>::operator()(B const & b, const TIndex t) const
{
     return b < t;
}

inline BackwardCorrectionAntiBranch::BackwardCorrectionAntiBranch()
{

}

inline BackwardCorrectionAntiBranch::
                                 BackwardCorrectionAntiBranch(BCorr const ncorr)

{
    correction = ncorr;
}

inline void BackwardCorrectionAntiBranch::
                                   merge(BackwardCorrectionAntiBranch const & b)

{
    correction = correction|b.correction;
}

template <typename B>
inline AntiBranchIterator<B>::AntiBranchIterator(Iter const i) :
    itr {i}
{

}

template <typename B>
inline TIndex & AntiBranchIterator<B>::operator*() const
{
    return itr->c_place.t;
}

template <typename B>
inline B * AntiBranchIterator<B>::operator->() const
{
    return &*itr;
}

template <typename B>
inline bool AntiBranchIterator<B>::
                               operator==(AntiBranchIterator<B> const & a) const
{
    return itr == a.itr;
}

template <typename B>
inline bool AntiBranchIterator<B>::
                               operator!=(AntiBranchIterator<B> const & a) const
{
    return itr != a.itr;
}

template <typename B>
inline AntiBranchIterator<B> & AntiBranchIterator<B>::operator++()
{
    ++itr; return *this;
}

template <bool forward, typename B, typename T>
template <typename ...Args>
inline GraphQueue<forward, B, T>::GraphQueue(Args ...args)
{
    B b {args...};
    append(b);
    qi = begin();
}

template <bool forward, typename B, typename T>
inline void GraphQueue<forward, B, T>::sort(B const & b)
{
    Greater<B> greater {};
    Less<B> less {};
    typename GraphQueue<forward, B, T>::iterator di {((!forward) ?
                         std::lower_bound(qi + 1, end(), b.c_place.t, greater) :
                         std::lower_bound(qi + 1, end(), b.c_place.t, less))};
    if (!B::simple) {
        if (di != end() && di->c_place.t.ind == b.c_place.t.ind) {
            di->merge(b);
        } else {
            insert(di, b);
        }
    } else if (di == end() || di->c_place.t.ind != b.c_place.t.ind) {
        insert(di, b);
    }
}

template <bool forward, typename B, typename T>
template <typename ...Args>
inline bool GraphQueue<forward, B, T>::operator()(qint32 const ind,
                                   SchemeFeaturer<T> const & sc_f, Args ...args)
{
    B b {args...};
    if (ind < T::graph_size_max) {
        b.c_place.t.ind = sc_f.original_inds.at(ind).ind;
        if (B::ccore_is_needed) {
            RIndex r {sc_f.transformed_inds.at(b.c_place.t.ind).ind};
            if (r.ind < T::graph_size_max) {
                b.c_place.ccore = &sc_f.originals[r.ind];
            } else {
                b.c_place.ccore = &sc_f.temporaries.at(r.ind -
                                                       T::temporary_inds_start);
            }
        }
    } else if (ind - T::temporary_inds_start >= 0) {
        b.c_place.t.ind = sc_f.temporary_inds.at(ind -
                                                  T::temporary_inds_start).ind;
        if (B::ccore_is_needed) {
            b.c_place.ccore = &sc_f.temporaries.at(ind -
                                                   T::temporary_inds_start);
        }
    } else {
        return true;
    }
    if (!forward || forward && b.c_place.t.ind >= 0) { sort(b); }
    return true;
}

template <>
inline qint32 SchemeFeaturer<SoulTraits>::get_legal_args_inds_finish()
{
    return Consts::sensor_size - 1 + f_required_id;
}

template <>
inline qint32 SchemeFeaturer<AngelTraits>::get_legal_args_inds_finish()
{
    return Consts::sensor_size + f_required_id;
}

template <typename T>
template <FLocation featured_place>
inline qint32 * SchemeFeaturer<T>::get_next_strings(qint32 const nmr_ind) const
{
    if (featured_place == FLocation::origs ||
                                           featured_place == FLocation::temps) {
        qint32 * si {strings};
        qint32 * ei {strings + T::strings_number};
        return std::upper_bound(si, ei, nmr_ind);
    } else if (featured_place == FLocation::memory) {
        return &strings[1];
    }
}

template <>
template <>
inline qint32 * SchemeFeaturer<SoulTraits>::
                         get_next_strings<FLocation::result>(qint32 const) const
{
    return &strings[2];
}

template <>
template <>
inline qint32 * SchemeFeaturer<AngelTraits>::
                 get_next_strings<FLocation::result>(qint32 const nmr_ind) const
{
    return &strings[2 + nmr_ind];
}

template <typename T>
inline void SchemeFeaturer<T>::prepare_a()
{
    std::copy(std::begin(sample->strings), std::end(sample->strings), strings);
    std::copy(std::begin(sample->memory_inds), std::end(sample->memory_inds),
              memory_inds);
    std::copy(std::begin(sample->result_inds), std::end(sample->result_inds),
              result_inds);
    poffset.s_f = moffset.s_f = moffset_minus.s_f = moffset_plus.s_f = this;
}

template <typename T>
inline void SchemeFeaturer<T>::prepare_ba(Feature & f, qint64 const nn)
{
    if (f.num < nn) {
        f.f_place.ind = f.num/Consts::operands_number_max;
        f.f_operand = f.num%Consts::operands_number_max;
        f.in.next_strings = get_next_strings<FLocation::origs>(f.f_place.ind);
        f.in.ind = f.f_place.ind - 1;
        f.f_place_loc = FLocation::origs;
    } else if (f.num - nn < T::memory_size) {
        f.mr_ind = f.num - nn;
        f.in.next_strings = get_next_strings<FLocation::memory>(f.mr_ind);
        f.in.ind = *f.in.next_strings - 1;
        f.f_place_loc = FLocation::memory;
    } else {
        f.mr_ind = f.num - nn - T::memory_size;
        f.in.next_strings = get_next_strings<FLocation::result>(f.mr_ind);
        f.in.ind = *f.in.next_strings - 1;
        f.f_place_loc = FLocation::result;
    }
}

template <typename T>
void SchemeFeaturer<T>::prepare_b()
{
    qint64 nn {graph_size*Consts::operands_number_max};
    random_connection.param(RandQInt64P{0, nn - 1 +
                                           T::memory_size + T::result_size});
    int s {random_features_number(reng)};
    for (int i {0}; i < s; ++i) {
        Feature f;
        QVarLengthArray<Feature>::const_iterator si;
        QVarLengthArray<Feature>::const_iterator ei {features.cend()};
        //TODO change the way you get f.num
        do {
            f.num = random_connection(reng);
            prepare_ba(f, nn);
            si = std::lower_bound(features.cbegin(), ei, f);
        } while (si != ei &&
                (f.f_place_loc == FLocation::origs ||
                     si->f_place_loc == FLocation::origs || f.num == si->num) &&
                 f.num/Consts::operands_number_max ==
                                           si->num/Consts::operands_number_max);
        f.in.steps = random_steps_number(reng);
        features.insert(si, f);
    }
}

template <typename T>
inline void SchemeFeaturer<T>::make_t_inserting(TIndex & t,
                                        QVarLengthArray<Feature>::iterator & si)
{
    qint32 steps {si->in.steps};
    transformed_inds.insert(transformed_inds.cend(),
                            steps, RIndex{-1});
    Offset<> moff {steps};
    std::transform(si->in.next_strings, strings + T::strings_number,
                   si->in.next_strings, moff);
    for (qint32 i {0}; i < steps; ++i) {
        erasings.append(t); ++t.ind;
    }
    si->in.t.ind = t.ind - 1;
    ++si;
}

template <typename T>
inline void SchemeFeaturer<T>::make_rt_relation(TIndex & t, RIndex & r)
{
    ++r.ind;
    transformed_inds.append(r);
    original_inds.append(t);
    ++t.ind;
}

template <typename T>
void SchemeFeaturer<T>::prepare_c()
{
    QVarLengthArray<Feature>::iterator si {features.begin()};
    QVarLengthArray<Feature>::iterator ei {features.end()};
    TIndex t;
    RIndex r;
    qint32 s {graph_size - 1};
    t.ind = 0; r.ind = -1;
    while (si != ei && r.ind < s) {
        if (r.ind == si->in.ind) {
            make_t_inserting(t, si);
        } else {
            make_rt_relation(t, r);
        }
    }
    while (r.ind < s) {
        make_rt_relation(t, r);
    }
}

template <typename T>
SchemeFeaturer<T>::SchemeFeaturer(DaemonScheme<T> const & sa,
                                  DaemonScheme<T> * ta) :
    sample {&sa}, target {ta}, originals {sa.graph},
    strings {ta->strings},
    memory_inds {ta->memory_inds}, result_inds {ta->result_inds},
    graph_size {sa.graph_size}
{
    prepare_a(); prepare_b(); prepare_c();
}

template <>
template <ONLocation place>
constexpr qint32 SchemeFeaturer<SoulTraits>::get_actual_id(qint32 const)
{
    static_assert(place == ONLocation::memory ||
                  place == ONLocation::args, "Err!");
    if (place == ONLocation::memory) {
       return 0;
    } else {
       return 1;
    }
}

template <>
template <ONLocation place>
inline qint32 SchemeFeaturer<AngelTraits>::get_actual_id(qint32 const ind)
{
    static_assert(place == ONLocation::memory ||
                  place == ONLocation::args, "Err!");
    if (place == ONLocation::memory) {
        return 0;
    } else {
        return ind - AngelTraits::graph_size_max - AngelTraits::memory_size;
    }
}

template <>
template <FLocation featured_place>
constexpr qint32 SchemeFeaturer<SoulTraits>::get_required_id(qint32 const)
{
    static_assert(featured_place == FLocation::memory ||
                  featured_place == FLocation::result, "Err!");
    if (featured_place == FLocation::memory) {
       return 1;
    } else {
       return 0;
    }
}

template <>
template <FLocation featured_place>
inline qint32 SchemeFeaturer<AngelTraits>::get_required_id(qint32 const mr_ind)
{
    static_assert(featured_place == FLocation::memory ||
                  featured_place == FLocation::result, "Err!");
    if (featured_place == FLocation::memory) {
       return 0;
    } else {
       return mr_ind;
    }
}

template <typename T>
template <bool avoidance_is_needed>
inline RIndex SchemeFeaturer<T>::get_reflected_index(qint32 i) const
{
    RIndex r;
    if (i < T::graph_size_max) {
        r.ind = transformed_inds.at(original_inds.at(i).ind).ind;
    } else if (!avoidance_is_needed || i >= T::temporary_inds_start) {
        r.ind = i;
    } else {
        r.ind = -1;
    }
    return r;
}

template <typename T>
template <FLocation place>
inline NodeCore const * SchemeFeaturer<T>::get_node_core(GraphPlace const & p)
{
    static_assert(place == FLocation::origs ||
                  place == FLocation::temps, "Err!");
    return (place == FLocation::origs) ? p.ccore : p.core;
}

template <typename T>
template <ONLocation place>
inline NodeCore const * SchemeFeaturer<T>::get_node_core(GraphPlace const & p)
{
    /* static_assert(place == ONLocation::origs ||
                    place == ONLocation::temps, "Err!"); */
    return (place == ONLocation::origs) ? p.ccore : p.core;
}

template <typename T>
template <bool ot_is_needed, bool reflected, ONLocation place>
inline void SchemeFeaturer<T>::get_node_core(GraphPlace & p)
{
    /* static_assert(place == ONLocation::origs && reflected ||
                    place == ONLocation::temps, "Err!"); */
    if (place == ONLocation::origs) {
        p.ccore = &originals[p.r.ind];
    } else if (reflected) {
        p.core = &temporaries[p.r.ind - T::temporary_inds_start];
    } else if (ot_is_needed) {
        p.core = &temporaries[p.ot_ind = p.ind - T::temporary_inds_start];
    } else {
        p.core = &temporaries[p.ind - T::temporary_inds_start];
    }
}

template <typename T>
inline NodeCore const * SchemeFeaturer<T>::get_node_core(qint32 const ind) const
{
    if (ind < T::graph_size_max) {
        RIndex r {transformed_inds.at(original_inds.at(ind).ind).ind};
        Q_ASSERT(r.ind >= 0);
        return get_node_core(r);
    }
    Q_ASSERT(ind - T::temporary_inds_start >= 0);
    return &temporaries.at(ind - T::temporary_inds_start);
}

template <typename T>
inline NodeCore const * SchemeFeaturer<T>::get_node_core(RIndex const r) const
{
    Q_ASSERT(r.ind >= 0);
    if (r.ind < T::graph_size_max) {
        return &originals[r.ind];
    }
    Q_ASSERT(r.ind - T::temporary_inds_start >= 0);
    return &temporaries.at(r.ind - T::temporary_inds_start);
}

template <typename T>
void SchemeFeaturer<T>::set_temporary_node_b(GraphPlace & p)
{
    temporaries.append(*p.ccore);
    temporary_inds_backup.append(p.r.ind);
    temporary_inds.append(p.t);
    transformed_inds[p.t.ind].ind =
                               temporaries.size() - 1 + T::temporary_inds_start;
    p.core = &temporaries.last();
}

template <typename T>
inline void SchemeFeaturer<T>::set_temporary_node_a(GraphPlace & p)
{
    temporaries.append(originals[p.r.ind]);
    temporary_inds_backup.append(p.r.ind);
    temporary_inds.append(p.t);
    transformed_inds[p.t.ind].ind = p.r.ind =
                               temporaries.size() - 1 + T::temporary_inds_start;
    p.core = &temporaries.last();
}

template <typename T>
inline void SchemeFeaturer<T>::set_temporary_node(GraphPlace & p)
{
    p.r.ind = transformed_inds.at(p.t.ind).ind;
    Q_ASSERT(p.r.ind >= 0);
    if (p.r.ind < T::graph_size_max) {
        set_temporary_node_a(p);
    } else {
        p.core = &temporaries[p.r.ind - T::temporary_inds_start];
    }
}

template <typename T, BCorr control>
template <FLocation place, typename ...RCC>
inline void RecepientsInvariant<T, control>::
          compare_req_id(qint32 const nmrot_ind, SchemeFeaturer<T> const & sc_f,
                                                                     RCC ...rcc)
{
    qint32 id;
    if (place == FLocation::origs || place == FLocation::temps) {
        //TODO update to GCC 4.9 and use lvalue reference instead of pointer
        NodeCore const * recepient_ccore {rcc...};
        id = recepient_ccore->required_subgraph_id;
    } else if (place == FLocation::memory) {
        id = sc_f.template get_required_id<FLocation::memory>(nmrot_ind);
    } else {
        id = sc_f.template get_required_id<FLocation::result>(nmrot_ind);
    }
    if (required_id > id) {
        required_id = id;
    }
}

template <typename T, BCorr control>
template <FLocation place>
inline void RecepientsInvariant<T, control>::
         compare_indices(qint32 const nmrot_ind, SchemeFeaturer<T> const & sc_f)
{
    qint32 * rec_str;
    if ((place == FLocation::origs || place == FLocation::temps)) {
        rec_str = sc_f.template get_next_strings<place>(t.ind);
        if (rec_str == sc_f.strings) {
            string_starts_min.ind = 0;
            return;
        } else {
            --rec_str;
        }
    } else {
        rec_str = sc_f.template get_next_strings<place>(nmrot_ind) - 1;
    }
    if (string_starts_min.ind > *rec_str) {
        string_starts_min.ind = *rec_str;
    }
}

template <typename T, BCorr control>
template <FLocation place, typename ...RCC>
inline void RecepientsInvariant<T, control>::merge_kind(RCC ...rcc)
{
    if (place == FLocation::origs || place == FLocation::temps) {
        //TODO update to GCC 4.9 and use lvalue reference instead of pointer
        NodeCore const * recepient_ccore {rcc...};
        subgraph_kind = subgraph_kind|recepient_ccore->subgraph_kind;
    } else if (place == FLocation::memory) {
        subgraph_kind = subgraph_kind|SubgraphKind::memory;
    } else {
        subgraph_kind = subgraph_kind|SubgraphKind::result;
    }
}

template <typename T, BCorr control>
template <FLocation place, typename ...RCC>
inline void RecepientsInvariant<T, control>::inspect_a(qint32 const nmrot_ind,
                                     SchemeFeaturer<T> const & sc_f, RCC ...rcc)
{
    if (test(control, BCorr::kind)) {
        merge_kind<place>(rcc...);
    }
    if (test(control, BCorr::indices)) {
        compare_indices<place>(nmrot_ind, sc_f);
    }
    if (test(control, BCorr::req_id)) {
        compare_req_id<place>(nmrot_ind, sc_f, rcc...);
    }
}

template <typename T, BCorr control>
template <FLocation place>
inline void RecepientsInvariant<T, control>::inspect(qint32 const nmrot_ind,
                                                 SchemeFeaturer<T> const & sc_f)
{
    if (place == FLocation::origs) {
        t = sc_f.original_inds.at(nmrot_ind);
    } else if (place == FLocation::temps) {
        t = sc_f.temporary_inds.at(nmrot_ind);
    }
    RIndex r;
    if (place == FLocation::origs || place == FLocation::temps) {
        r = sc_f.transformed_inds.at(t.ind);
        if (r.ind == -1) { return; }
        if (place == FLocation::origs) {
            inspect_a<place>(nmrot_ind, sc_f, sc_f.get_node_core(r));
        } else {
            inspect_a<place>(nmrot_ind, sc_f, &sc_f.temporaries.at(nmrot_ind));
        }
    } else {
        inspect_a<place>(nmrot_ind, sc_f);
    }
}

template <typename T, BCorr control>
template <typename>
inline bool RecepientsInvariant<T, control>::operator()(qint32 const ind,
                                                 SchemeFeaturer<T> const & sc_f)
{
    if (ind - T::graph_size_max - T::memory_size < 0) {
        if (ind - T::graph_size_max < 0) {
            inspect<FLocation::origs>(ind, sc_f);
        } else {
            inspect<FLocation::memory>(ind - T::graph_size_max, sc_f);
        }
    } else {
        if (ind - T::temporary_inds_start < 0) {
            inspect<FLocation::result>(ind - T::graph_size_max - T::memory_size,
                                       sc_f);
        } else {
            inspect<FLocation::temps>(ind - T::temporary_inds_start, sc_f);
        }
    }
    return true;
}

template <typename T, BCorr control>
inline RecepientsInvariant<T, control>::
                             RecepientsInvariant(SchemeFeaturer<T> const & sc_f)
{
    /* static_assert(control != BCorr::none &&
                     T::strings_number >= 3, "Err!"); */
    if (test(control, BCorr::kind )) { subgraph_kind = SubgraphKind::none; }
    if (test(control, BCorr::indices)) {
        string_starts_min.ind = sc_f.strings[T::strings_number - 2];
    }
    if (test(control, BCorr::req_id)) { required_id = 0x7FFFFFFF; }
}

template <typename T>
template <BCorr ctrl>
inline RecepientsInvariant<T, ctrl> const & SchemeFeaturer<T>::get_ri_seeker()
{
    RecepientsInvariant<T, ctrl> seeker {*this};
    return seeker;
}

template <typename T>
template <BCorr control>
inline RecepientsInvariant<T, control> const & SchemeFeaturer<T>::get_ri_seeker(
                                 RecepientsInvariant<T, control> const & seeker)
{
    return seeker;
}

template <typename T>
template <qint32 value>
inline qint32 SchemeFeaturer<T>::get_v()
{
    return ((value == 0) ? v : 1);
}

template <typename T>
template <qint32 value>
inline ROffset<T, value> & SchemeFeaturer<T>::get_moffset()
{
    static_assert(value == 0, "Err!");
    return moffset;
}

template <>
template <>
inline ROffset<SoulTraits, -1> & SchemeFeaturer<SoulTraits>::get_moffset<-1>()
{
    return moffset_minus;
}

template <>
template <>
inline ROffset<SoulTraits, 1> & SchemeFeaturer<SoulTraits>::get_moffset<1>()
{
    return moffset_plus;
}

template <>
template <>
inline ROffset<AngelTraits, -1> & SchemeFeaturer<AngelTraits>::get_moffset<-1>()
{
    return moffset_minus;
}

template <>
template <>
inline ROffset<AngelTraits, 1> & SchemeFeaturer<AngelTraits>::get_moffset<1>()
{
    return moffset_plus;
}

template <typename T>
void SchemeFeaturer<T>::realize()
{
    //TODO ....
}

template <typename T>
template <qint32 value, bool buf, typename IterB, typename IterE>
inline void SchemeFeaturer<T>::scroll_b(IterB const si, IterE const ei)
{
    static_assert(value >= -1 || value <= 1, "Err!");
    using B = typename std::iterator_traits<IterB>::value_type;
    if (buf) {
        QVarLengthArray<B> buffer;
        std::copy(si, si + v, std::back_inserter(buffer));
        std::transform(si + v, ei, si, get_moffset<value>());
        std::transform(buffer.cbegin(), buffer.cend(), ei - v, poffset);
    } else if (value == 0) {
        std::transform(si + v, ei, si, moffset);
        std::fill_n(ei - v, v, B{-1});
    } else {
        B b (*si);
        std::transform(si + 1, ei, si, get_moffset<value>());
        *(ei - 1) = poffset(b);
    }
}

template <typename T>
template <qint32 value, bool reverse, bool engaged>
inline void SchemeFeaturer<T>::scroll_d(TIndex const a, TIndex const b)
{
    /* static_assert(reverse || !engaged, "Err!"); */
    qint32 * const zi {strings + T::strings_number};
    qint32 * si {((!engaged) ? std::upper_bound(strings, zi, a.ind) :
                               strings)};
    qint32 * ei {((!reverse) ? std::upper_bound(strings, zi, b.ind) :
                               std::lower_bound(strings, zi, b.ind))};
    Q_ASSERT(si == zi && ei == zi || si != zi &&
                                    (reverse || *si >= a.ind + get_v<value>())&&
                             (!reverse || *(ei - 1) <= b.ind - get_v<value>()));
    std::transform(si, ei, si, get_moffset<value>());
}

template <typename T>
template <qint32 value, bool reverse>
inline void SchemeFeaturer<T>::scroll_c(TIndex const a, TIndex const b)
{
    FeatureIterator si {std::lower_bound(ni, mi, a)};
    FeatureIterator ei {std::upper_bound(ni, mi, b)};
    Q_ASSERT((si == mi && ei == mi || si != mi && (reverse ||
                si->in.t.ind - si->in.steps + 1 >= a.ind + get_v<value>())&&
                (!reverse || (ei - 1)->in.t.ind < b.ind - get_v<value>()))&&
                    (ei == mi || ei->in.t.ind - ei->in.steps + 1 >= b.ind));
    std::transform(si, ei, si, get_moffset<value>());
    using Dir = QVarLengthArray<TIndex>::iterator;
    using Rev = std::reverse_iterator<Dir>;
    Dir di {erasings.end()};
    if (!reverse) {
        Dir si {std::lower_bound(erasings.begin(), di, a)};
        Dir ei {std::lower_bound(erasings.begin(), di, b)};
        v = std::lower_bound(erasings.begin(), di, a + get_v<value>()) - si;
        scroll_b<value, true>(si, ei);
    } else {
        Rev si {std::lower_bound(erasings.begin(), di, b)};
        Rev ei {std::lower_bound(erasings.begin(), di, a)};
        v = Rev{std::lower_bound(erasings.begin(), di, b - get_v<value>())} -
            si;
        scroll_b<value, true>(si, ei);
    }
}

template <typename T>
template <qint32 value, bool reverse, typename ...V>
inline void SchemeFeaturer<T>::scroll_a(qint32 const length, V ...val)
{
    if (value == 0 && reverse) {
        v = qint32{val...};
        moffset.set_value(qint32{val...});
        poffset.set_value(v - length);
    } else if (value == 0 && !reverse) {
        v = -qint32{val...};
        moffset.set_value(qint32{val...});
        poffset.set_value(length - v);
    } else if (value < 0) {
        poffset.set_value(length - 1);
    } else {
        poffset.set_value(1 - length);
    }
}

template <typename T>
template <qint32 value, bool buf, bool engaged, typename ...V>
inline void SchemeFeaturer<T>::scroll(TIndex const a, TIndex const b, V ...val)
{
    static_assert(value >= -1 || value <= 1, "Err!");
    qint32 length {b.ind - a.ind};
    Q_ASSERT(length >= 1 && b.ind <= transformed_inds.size() && a.ind >= 0 &&
                                           (value != 0 || qint32{val...} != 0));
    using Dir = typename TransformedInds<T>::iterator;
    using Rev = std::reverse_iterator<Dir>;
    if (value < 0 || value == 0 && qint32{val...} < 0) {
        Dir si {transformed_inds.begin() + a.ind};
        Dir ei {transformed_inds.begin() + b.ind};
        scroll_a<value, false>(length, val...);
        if (poffset.get_value() != 0) {
            scroll_b<value, ((value != 0) ? false : buf)>(si, ei);
            scroll_c<value, false>(a, b);
        }
        scroll_d<value, false, engaged>(a, b);
    } else {
        Rev si {transformed_inds.begin() + b.ind};
        Rev ei {transformed_inds.begin() + a.ind};
        scroll_a<value, true>(length, val...);
        if (poffset.get_value() != 0) {
            scroll_b<value, ((value != 0) ? false : buf)>(si, ei);
            scroll_c<value, true>(a, b);
        }
        scroll_d<value, false, engaged>(a, b);
    }
}

template <typename T>
template <bool engaged>
inline void SchemeFeaturer<T>::correct_backward_cb(BackwCorrNodes<T> & queue,
                                                   GraphPlace const & cu_place)
{
    Q_ASSERT(cu_place.t.ind - f_tail_start.ind >= 1);
    scroll<1, false, engaged>(f_tail_start, cu_place.t + 1);
    using IterQ = AntiBranchIterator<BackwardCorrectionAntiBranch>;
    Greater<IterQ::value_type> greater {};
    typename BackwCorrNodes<T>::iterator di {queue.qi + 1};
    IterQ si {di};
    IterQ ei {std::upper_bound(di, queue.end(), f_tail_start, greater)};
    std::transform(si, ei, si, moffset_plus);
    using IterI = AntiBranchIterator<SimpleAntiBranch>;
    typename Illegals<T>::iterator xi {some_illegals.begin()};
    IterI ti {xi};
    IterI zi {std::lower_bound(xi, some_illegals.end(), cu_place.t)};
    Q_ASSERT(zi == IterI{some_illegals.end()} ||
             zi->c_place.t.ind != cu_place.t.ind);
    std::transform(ti, zi, ti, moffset_plus);
    ++f_in_start.ind; ++n_place.t.ind; ++f_in_finish.ind;
}

template <typename T>
inline void SchemeFeaturer<T>::correct_backward_ca(GraphPlace const & cu_place,
                                                   TIndex const string_start)
{
    Q_ASSERT(string_start.ind - cu_place.t.ind >= 1);
    scroll<-1>(cu_place.t, string_start);
}

template <typename T>
template <bool old, ONLocation place, BCorr ncorr, typename ...S>
inline void SchemeFeaturer<T>::correct_backward_c(BackwCorrNodes<T> & queue,
                                                  GraphPlace & cu_place, S ...s)
{
    /* static_assert(ncorr != BCorr::none, "Err!"); */
    if (place == ONLocation::origs) { set_temporary_node_b(cu_place); }
    if (test(ncorr, BCorr::kind)) {
        cu_place.core->subgraph_kind = ((old) ?
                                             get_ri_seeker(s...).subgraph_kind :
                                     SubgraphKind::memory|SubgraphKind::result);
    }
    if (test(ncorr, BCorr::req_id)) {
        cu_place.core->required_subgraph_id = ((old) ?
                                               get_ri_seeker(s...).required_id :
                                                                 f_required_id);
    }
    if (old && test(ncorr, BCorr::indices)) {
        correct_backward_ca(cu_place,
                            get_ri_seeker(s...).string_starts_min);
    } else if (!old && test(ncorr, BCorr::indices)) {
        correct_backward_cb<test(ncorr, BCorr::kind)>(queue, cu_place);
    }
    cu_place.core->scan_operands_inds(queue, *this, ncorr);
}

template <typename T>
template <bool old, ONLocation place, BCorr control, BCorr ncorr, typename ...S>
inline void SchemeFeaturer<T>::correct_backward_bc(BackwCorrNodes<T> & queue,
                                                  GraphPlace & cu_place, S ...s)
{
    if (test(control, BCorr::req_id) &&
                  (old && get_node_core<place>(cu_place)->required_subgraph_id <
                                              get_ri_seeker(s...).required_id ||
                  !old && get_node_core<place>(cu_place)->required_subgraph_id >
                                                               f_required_id)) {
        correct_backward_c<old, place,
                           ncorr|BCorr::req_id>(queue, cu_place, s...);
    } else if (ncorr != BCorr::none) {
        correct_backward_c<old, place,
                           ncorr>(queue, cu_place, s...);
    }
}

template <typename T>
template <bool old, ONLocation place, BCorr control, BCorr ncorr, typename ...S>
inline void SchemeFeaturer<T>::correct_backward_bb(BackwCorrNodes<T> & queue,
                                                  GraphPlace & cu_place, S ...s)
{
    Q_ASSERT(!test(control, BCorr::indices) || old ||
                                    !old && cu_place.t.ind != f_tail_start.ind);

    if (test(control, BCorr::indices) && !(test(ncorr, BCorr::kind)) &&
                   (old && cu_place.t < get_ri_seeker(s...).string_starts_min ||
                                           !old && cu_place.t > f_tail_start)) {
        correct_backward_bc<old, place, control,
                            ncorr|BCorr::indices>(queue, cu_place, s...);
    } else {
        correct_backward_bc<old, place, control,
                            ncorr>(queue, cu_place, s...);
    }
}

template <typename T>
template <bool old, ONLocation place, BCorr control, typename ...S>
inline void SchemeFeaturer<T>::correct_backward_ba(BackwCorrNodes<T> & queue,
                                                  GraphPlace & cu_place, S ...s)
{
    if (test(control, BCorr::kind) &&
                                get_node_core<place>(cu_place)->subgraph_kind !=
                                    ((old) ? get_ri_seeker(s...).subgraph_kind :
                                   SubgraphKind::memory|SubgraphKind::result)) {
        correct_backward_bb<old, place, control,
                            BCorr::indices|BCorr::kind>(queue, cu_place, s...);
    } else {
        correct_backward_bb<old, place, control,
                            BCorr::none>(queue, cu_place, s...);
    }
}

template <typename T>
template <bool old, ONLocation place, BCorr control>
inline void SchemeFeaturer<T>::correct_backward_b(BackwCorrNodes<T> & queue,
                                                  GraphPlace & cu_place)
{
    /* static_assert(control != BCorr::none, "Err!"); */
    if (old) {
        RecepientsInvariant<T, control> seeker {*this};
        get_node_core<place>(cu_place)->template
                                     scan_recepients_inds<false>(seeker, *this);
        correct_backward_ba<old, place, control>(queue, cu_place, seeker);
    } else {
        correct_backward_ba<old, place, control>(queue, cu_place);
    }
}

template <typename T>
template <bool old, ONLocation place>
inline void SchemeFeaturer<T>::correct_backward_da(BackwCorrNodes<T> & queue,
                                     GraphPlace & cu_place, BCorr const control)
{
    Q_ASSERT(control != BCorr::none);
    switch (control) {
    case BCorr::kind:
        correct_backward_b<old, place, BCorr::kind>(queue, cu_place); break;
    case BCorr::indices:
        correct_backward_b<old, place, BCorr::indices>(queue, cu_place); break;
    case BCorr::indices|BCorr::kind:
        correct_backward_b<old, place,
                           BCorr::indices|BCorr::kind>(queue, cu_place); break;
    case BCorr::req_id:
        correct_backward_b<old, place, BCorr::req_id>(queue, cu_place); break;
    case BCorr::req_id|BCorr::kind:
        correct_backward_b<old, place,
                           BCorr::req_id|BCorr::kind>(queue, cu_place); break;
    case BCorr::req_id|BCorr::indices:
        correct_backward_b<old, place,
                          BCorr::req_id|BCorr::indices>(queue, cu_place); break;
    case BCorr::req_id|BCorr::indices|BCorr::kind:
        correct_backward_b<old, place,
                     BCorr::req_id|BCorr::indices|BCorr::kind>(queue, cu_place);
    case BCorr::none:;
    }
}

template <typename T>
template <bool old>
inline void SchemeFeaturer<T>::correct_backward_d(BackwCorrNodes<T> & queue,
                                                  GraphPlace & cu_place)
{
    cu_place.r.ind = transformed_inds.at(cu_place.t.ind).ind;
    Q_ASSERT(cu_place.r.ind >= 0);
    if (cu_place.r.ind < T::graph_size_max) {
        cu_place.ccore = &originals[cu_place.r.ind];
        correct_backward_da<old, ONLocation::origs>(queue, cu_place,
                                                    queue.qi->correction);
    } else {
        cu_place.core = &temporaries[cu_place.r.ind - T::temporary_inds_start];
        correct_backward_da<old, ONLocation::temps>(queue, cu_place,
                                                    queue.qi->correction);
    }
}

template <typename T>
template <bool old>
inline void SchemeFeaturer<T>::correct_backward_a(BackwCorrNodes<T> & queue)
{
    ++queue.qi;
    while (queue.qi != queue.end()) {
        correct_backward_d<old>(queue, queue.qi->c_place);
        ++queue.qi;
    }
}

template <typename T>
template <ONLocation place, BCorr control>
inline void SchemeFeaturer<T>::correct_backward_e(GraphPlace & p)
{
    SubgraphKind kind {get_node_core<place>(p)->subgraph_kind};
    if (kind == (SubgraphKind::memory|SubgraphKind::result)) {
        BackwCorrNodes<T> queue;
        correct_backward_b<true, place,
                           control|BCorr::kind|BCorr::indices>(queue, p);
        correct_backward_a<true>(queue);
    } else if (control != BCorr::none) {
        BackwCorrNodes<T> queue;
        correct_backward_b<true, place,
                           control>(queue, p);
        correct_backward_a<true>(queue);
    }
}

template <typename T>
template <ONLocation place>
inline void SchemeFeaturer<T>::correct_backward_f(GraphPlace & p)
{
    qint32 req_id {get_node_core<place>(p)->required_subgraph_id};
    Q_ASSERT(req_id <= f_required_id);
    if (req_id == f_required_id) {
        if (T::bc_parameter) {
            correct_backward_e<place, BCorr::req_id|BCorr::indices>(p);
        } else {
            correct_backward_e<place, BCorr::req_id>(p);
        }
    } else {
        correct_backward_e<place, BCorr::none>(p);
    }
}

template <typename T>
template <bool old, ONLocation place>
void SchemeFeaturer<T>::correct_backward(GraphPlace & p)
{
    static_assert(place == ONLocation::origs ||
                  place == ONLocation::temps, "Err!");
    if (old) {
        correct_backward_f<place>(p);
    } else {
        SubgraphKind kind {get_node_core<place>(p)->subgraph_kind};
        BackwCorrNodes<T> queue;
        if ((kind|f_subgraph_kind) != kind) {
            f_tail_start.ind = strings[0];
            correct_backward_bc<false, place, BCorr::req_id,
                                BCorr::indices|BCorr::kind>(queue, p);
        } else {
            f_tail_start = f_in_start;
            correct_backward_bb<false, place, BCorr::req_id|BCorr::indices,
                                BCorr::none>(queue, p);
        }
        correct_backward_a<false>(queue);
    }
}

template <typename T, bool root>
template <typename ...R>
inline bool LastRecepientChecker<T, root>::operator()(qint32 const ind,
                                     SchemeFeaturer<T> const & sc_f, R ...other)
{
    qint32 r_ind {sc_f.template get_reflected_index<false>(ind).ind};
    if (r_ind != -1 && (!root || r_ind != qint32{other...})) {
        the_last = false;
        return false;
    }
    return true;
}

template <typename T>
template <ONLocation place>
inline void SchemeFeaturer<T>::disconnect_tree_c(DisconnectedNodes<T> & queue,
                                                 GraphPlace & cu_place)
{
    /* static_assert(place == ONLocation::origs ||
                  place == ONLocation::temps, "Err!"); */
    Q_ASSERT(transformed_inds[cu_place.t.ind].ind != -1);
    transformed_inds[cu_place.t.ind].ind = -1;
    erasings.insert(std::lower_bound(erasings.cbegin(), erasings.cend(),
                                     cu_place.t), cu_place.t);
    get_node_core<place>(cu_place)->scan_operands_inds(queue, *this);
    --graph_size;
}

template <typename T>
template <ONLocation old_place>
inline bool SchemeFeaturer<T>::check_recipient_is_last(GraphPlace & o_place,
                                                       GraphPlace & f_place)
{
    /* static_assert(old_place == ONLocation::origs ||
                  old_place == ONLocation::temps, "Err!"); */
    LastRecepientChecker<T, true> checker {true};
    get_node_core<old_place>(o_place)->template
                      scan_recepients_inds<true>(checker, *this, f_place.r.ind);
    return checker.the_last;
}

template <typename T>
template <ONLocation place>
inline bool SchemeFeaturer<T>::check_recipient_is_last(GraphPlace & p)
{
    static_assert(place == ONLocation::origs ||
                  place == ONLocation::temps, "Err!");
    LastRecepientChecker<T, false> checker {true};
    get_node_core<place>(p)->
                            template scan_recepients_inds<true>(checker, *this);
    return checker.the_last;
}

template <typename T>
template <ONLocation old_place, FLocation featured_place>
inline void SchemeFeaturer<T>::disconnect_tree_ba(DisconnectedNodes<T> & queue,
                    GraphPlace & o_place, GraphPlace & f_place, qint32 const op)
{
    /* static_assert(place == ONLocation::origs ||
                  place == ONLocation::temps, "Err!"); */
    if (check_recipient_is_last<old_place>(o_place, f_place)) {
        disconnect_tree_c<old_place>(queue, o_place);
    } else {
        if (old_place == ONLocation::origs) { set_temporary_node_a(o_place); }
        disconnect_node<featured_place>(o_place, f_place, op);
        correct_backward<true, ONLocation::temps>(o_place);
    }
}

template <typename T>
template <ONLocation place>
inline void SchemeFeaturer<T>::disconnect_tree_b(DisconnectedNodes<T> & queue,
                                                 GraphPlace & cu_place)
{
    /* static_assert(place == ONLocation::origs ||
                  place == ONLocation::temps, "Err!"); */
    if (check_recipient_is_last<place>(cu_place)) {
        disconnect_tree_c<place>(queue, cu_place);
    } else {
        correct_backward<true, place>(cu_place);
    }
}

template <typename T>
inline void SchemeFeaturer<T>::disconnect_tree_d(DisconnectedNodes<T> & queue,
                                                 GraphPlace & cu_place)
{
    cu_place.r.ind = transformed_inds.at(cu_place.t.ind).ind;
    Q_ASSERT(cu_place.r.ind >= 0);
    if (cu_place.r.ind < T::graph_size_max) {
        cu_place.ccore = &originals[cu_place.r.ind];
        disconnect_tree_b<ONLocation::origs>(queue, cu_place);
    } else {
        cu_place.core = &temporaries[cu_place.r.ind - T::temporary_inds_start];
        disconnect_tree_b<ONLocation::temps>(queue, cu_place);
    }
}

template <typename T>
inline void SchemeFeaturer<T>::disconnect_tree_a(DisconnectedNodes<T> & queue)
{
    ++queue.qi;
    while (queue.qi != queue.end()) {
        disconnect_tree_d(queue, queue.qi->c_place);
        ++queue.qi;
    }
}

template <typename T>
template <ONLocation old_place, FLocation featured_place>
void SchemeFeaturer<T>::disconnect_tree(Feature & f)
{
    /* static_assert(old_place == ONLocation::origs ||
                  old_place == ONLocation::temps, "Err!"); */
    DisconnectedNodes<T> queue;
    disconnect_tree_ba<old_place, featured_place>(queue, f.o_place,
                                                  f.f_place, f.f_operand);
    disconnect_tree_a(queue);
}

template <typename T>
template <FLocation featured_place>
void SchemeFeaturer<T>::disconnect_node(GraphPlace & o_place,
                                   GraphPlace & f_place, qint32 const f_operand)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    RecepientsByOperand * r_indices;
    r_indices = &o_place.core->
        recipients_inds[(featured_place == FLocation::temps) ? f_operand : 0];
    RecepientsByOperand::const_iterator si {r_indices->cbegin()};
    RecepientsByOperand::const_iterator ei {r_indices->cend()};
    Q_ASSERT(si != ei);
    if (featured_place == FLocation::temps) {
        Q_ASSERT(f_place.r.ind >= T::temporary_inds_start);
        while (*si != f_place.r.ind && *si != f_place.ind) {
            ++si; Q_ASSERT(si != ei);
        }
    } else {
        while (*si != f_place.r.ind) {
            ++si; Q_ASSERT(si != ei);
        }
    }
    r_indices->erase(si);
}

template <typename T>
inline void OperandsActualID<T>::compare(qint32 const id)
{
    if (actual_id < id) {
        actual_id = id;
    }
}

template <typename T>
template <typename>
inline void OperandsActualID<T>::operator()(qint32 const ind,
                                            SchemeFeaturer<T> const & sc_f)
{
    if (ind < T::graph_size_max + T::memory_size) {
        if (ind < T::graph_size_max) {
            RIndex r {sc_f.transformed_inds.at(sc_f.original_inds.at(ind)
                                                .ind).ind};
            Q_ASSERT(r.ind >= 0);
            compare(sc_f.get_node_core(r)->actual_subgraph_id);
        } else {
            compare(sc_f.template get_actual_id<ONLocation::memory>(ind));
        }
    } else {
        if (ind - T::temporary_inds_start < 0) {
            compare(sc_f.template get_actual_id<ONLocation::args>(ind));
        } else {
            compare(sc_f.temporaries.
                          at(ind - T::temporary_inds_start).actual_subgraph_id);
        }
    }
}

template <typename T>
inline OperandsActualID<T>::OperandsActualID(qint32 const id) :
    actual_id {id}
{

}

template <typename T>
inline void SchemeFeaturer<T>::correct_forward_e(GraphPlace const & p)
{
    OperandsActualID<T> seeker;
    p.core->scan_operands_inds(seeker, *this);
    p.core->actual_subgraph_id = seeker.actual_id;
}

template <typename T>
void SchemeFeaturer<T>::correct_forward(GraphPlace & p,
                                        InsertedNodes<T> const & queue)
{
    correct_forward_e(p);
    typename InsertedNodes<T>::const_iterator si {queue.cend()};
    typename InsertedNodes<T>::const_iterator ei {queue.cbegin()};
    if (si == ei) { goto it_looks_like; }
    do {
        correct_forward_e(*--si);
    } while (si != ei);
    p.core = ei->core;
    it_looks_like:;
}

template <typename T>
template <FLocation place, bool bigger, typename ...Args>
inline void SchemeFeaturer<T>::correct_forward_c(ForwCorrNodes<T> & queue,
                                            GraphPlace & cu_place, Args ...args)
{
    if (place == FLocation::origs) { set_temporary_node_b(cu_place); }
    cu_place.core->actual_subgraph_id = (bigger) ? f_actual_id :
                                                   qint32{args...};
    cu_place.core->scan_recepients_inds<false>(queue, *this);
}

template <typename T>
template <FLocation place, bool bigger>
inline void SchemeFeaturer<T>::correct_forward_b(ForwCorrNodes<T> & queue,
                                                 GraphPlace & cu_place)
{
    qint32 actual_id {get_node_core<place>(cu_place)->actual_subgraph_id};
    if (bigger) {
        if (actual_id < f_actual_id) {
            correct_forward_c<place, true>(queue, cu_place);
        }
    } else {
        OperandsActualID<T> seeker;
        get_node_core<place>(cu_place)->scan_operands_inds(seeker, *this);
        Q_ASSERT(actual_id >= seeker.actual_id);
        if (actual_id > seeker.actual_id) {
            correct_forward_c<place, false>(queue, cu_place, seeker.actual_id);
        }
    }
}

template <typename T>
template <bool bigger>
inline void SchemeFeaturer<T>::correct_forward_d(ForwCorrNodes<T> & queue,
                                                 GraphPlace & cu_place)
{
    cu_place.r.ind = transformed_inds.at(cu_place.t.ind).ind;
    Q_ASSERT(cu_place.r.ind >= 0);
    if (cu_place.r.ind < T::graph_size_max) {
        cu_place.ccore = &originals[cu_place.r.ind];
        correct_forward_b<FLocation::origs, bigger>(queue, cu_place);
    } else {
        cu_place.core = &temporaries[cu_place.r.ind - T::temporary_inds_start];
        correct_forward_b<FLocation::temps, bigger>(queue, cu_place);
    }
}

template <typename T>
template <bool bigger>
inline void SchemeFeaturer<T>::correct_forward_a(ForwCorrNodes<T> & queue)
{
    ++queue.qi;
    while (queue.qi != queue.end()) {
        correct_forward_d<bigger>(queue, queue.qi->c_place);
        ++queue.qi;
    }
}

template <typename T>
template <bool reflected, ONLocation place>
inline qint32 SchemeFeaturer<T>::get_actual_id(GraphPlace & p)
{
    if (place == ONLocation::origs) {
        return p.ccore->actual_subgraph_id;
    } else if (place == ONLocation::memory) {
        return get_actual_id<ONLocation::memory>(p.ind);
    } else if (place == ONLocation::args) {
        return get_actual_id<ONLocation::args>(p.ind);
    } else {
        if (!reflected) { get_node_core<false, false, place>(p); }
        return p.core->actual_subgraph_id;
    }
}

template <typename T>
template <bool reflected, ONLocation new_place, ONLocation old_place>
void SchemeFeaturer<T>::correct_forward(GraphPlace & p,
                                        Feature & f)
{
    static_assert(new_place != ONLocation::origs, "Err!");
    qint32 old_actual_id {get_actual_id<true, old_place>(f.o_place)};
    qint32 new_actual_id {get_actual_id<reflected, new_place>(p)};
    qint32 curr_actual_id {f.f_place.core->actual_subgraph_id};
    Q_ASSERT(curr_actual_id >= old_actual_id);
    if (new_actual_id > curr_actual_id) {
        f_actual_id = new_actual_id;
        ForwCorrNodes<T> queue;
        correct_forward_c<FLocation::temps, true>(queue, f.f_place);
        correct_forward_a<true>(queue);
    } else if (curr_actual_id == old_actual_id &&
               curr_actual_id > new_actual_id) {
        ForwCorrNodes<T> queue;
        correct_forward_b<FLocation::temps, false>(queue, f.f_place);
        correct_forward_a<false>(queue);
    }
}

template <typename T>
template <bool reflected, ONLocation new_place, FLocation featured_place>
inline void SchemeFeaturer<T>::connect_nodes(GraphPlace & a,
                                             Feature const & f)
{
    /* static_assert(featured_place != FLocation::temps &&
                  new_place != ONLocation::origs, "Err!"); */
    if (featured_place == FLocation::memory) {
        memory_inds[f.mr_ind] = (reflected) ? a.r.ind : a.ind;
    } else {
        result_inds[f.mr_ind] = (reflected) ? a.r.ind : a.ind;
    }
    if (new_place == ONLocation::temps) {
        a.core->recipients_inds[0].append(f.f_place.r.ind);
    }
}

template <typename T>
template <bool reflected, ONLocation new_place>
inline void SchemeFeaturer<T>::connect_nodes(GraphPlace & a,
                                             GraphPlace & b, qint32 const op)
{
    /* static_assert(new_place != ONLocation::origs, "Err!"); */
    b.core->operands_inds[op] = (reflected) ? a.r.ind : a.ind;
    if (new_place == ONLocation::temps) {
        a.core->recipients_inds[op].append(b.r.ind);
    }
}

template <typename T>
bool SchemeFeaturer<T>::check_place_is_legal_a(GraphPlace const & p)
{
    typename Illegals<T>::iterator di {std::lower_bound(some_illegals.begin(),
                                                        some_illegals.end(),
                                                        p.t)};
    return (di == some_illegals.end() || di->c_place.t.ind != p.t.ind) &&
           (p.t < f_in_start || f_in_finish < p.t);
}

template <typename T>
template <ONLocation place>
inline bool SchemeFeaturer<T>::check_place_is_legal(GraphPlace const & p)
{
    if (place == ONLocation::origs) {
        if (originals[p.r.ind].actual_subgraph_id <= f_required_id &&
                                                    check_place_is_legal_a(p)) {
            return true;
        }
        return false;
    } else if (place == ONLocation::temps) {
        if (temporaries.at(p.ot_ind).actual_subgraph_id <= f_required_id &&
                                                    check_place_is_legal_a(p)) {
            return true;
        }
        return false;
    } else if (place == ONLocation::memory) {
        return get_actual_id<ONLocation::memory>(p.ind) <= f_required_id;
    } else {
        return get_actual_id<ONLocation::args>(p.ind) <= f_required_id;
    }
}

template <typename T>
inline bool SchemeFeaturer<T>::set_checked_temporary_node(GraphPlace & p)
{
    p.r.ind = transformed_inds.at(p.t.ind).ind;
    Q_ASSERT(p.r.ind >= 0 && p.r.ind < T::graph_size_max ||
             p.r.ind >= T::temporary_inds_start);
    if (p.r.ind < T::graph_size_max) {
        if (check_place_is_legal<ONLocation::origs>(p)) {
            set_temporary_node_a(p);
            return true;
        }
    } else {
        p.ot_ind = p.r.ind - T::temporary_inds_start;
        if (check_place_is_legal<ONLocation::temps>(p)) {
            p.core = &temporaries[p.ot_ind];
            return true;
        }
    }
    return false;
}

template <typename T>
inline void SchemeFeaturer<T>::connect_abroad(qint32 const op)
{
    GraphPlace p;
    QVarLengthArray<TIndex>::const_iterator si {erasings.cbegin()};
    QVarLengthArray<TIndex>::const_iterator ei {erasings.end()};
    do {
        p.ind = random_n_place(reng);
        if (p.ind < graph_size) {
            p.t.ind = p.ind + std::upper_bound(si, ei, p.t) - si;
            if (set_checked_temporary_node(p)) {
                connect_nodes<true, ONLocation::temps>(p, n_place, op);
                correct_backward<false, ONLocation::temps>(p);
                break;
            }
        } else {
            p.ind += T::graph_size_max - graph_size;
            if (p.ind < T::graph_size_max + T::memory_size) {
                if (check_place_is_legal<ONLocation::memory>(p)) {
                    connect_nodes<false, ONLocation::memory>(p, n_place, op);
                    break;
                }
            } else if (check_place_is_legal<ONLocation::args>(p)) {
                connect_nodes<false, ONLocation::args>(p, n_place, op);
                break;
            }
        }
    } while (true);
}

template <typename T>
template <FLocation featured_place>
inline void SchemeFeaturer<T>::connect_insert(Feature & f)
{
    if (featured_place == FLocation::temps) {
        connect_nodes<true, ONLocation::temps>(n_place, f.f_place, f.f_operand);
    } else {
        connect_nodes<true, ONLocation::temps, featured_place>(n_place, f);
    }
}

template <typename T>
template <ONLocation old_place, FLocation featured_place>
inline void SchemeFeaturer<T>::insert_nodes_e(Feature & f,
                                              InsertedNodes<T> const & queue)
{
    if (old_place == ONLocation::origs || old_place == ONLocation::temps) {
        if (old_place == ONLocation::origs) { set_temporary_node_a(f.o_place); }
        disconnect_node<featured_place>(f.o_place, f.f_place, f.f_operand);
        connect_nodes<true, ONLocation::temps>(f.o_place, n_place, 0);
    } else {
        connect_nodes<false, old_place>(f.o_place, n_place, 0);
    }
    correct_forward(n_place, queue);
    if (featured_place == FLocation::temps) {
        correct_forward<true, ONLocation::temps,
                      (old_place == ONLocation::origs) ? ONLocation::temps :
                                                         old_place>(n_place, f);
    }
}

template <typename T>
void SchemeFeaturer<T>::insert_nodes_c()
{
    static_assert(Consts::operands_number_max >= 1, "Err!");

    random_n_place.param(RandQInt32P{0, graph_size +
                                T::memory_size + get_legal_args_inds_finish()});
    int op_number {n_place.core->get_operands_number()};
    for (qint32 i {1}; i < op_number; ++i) {
        connect_abroad(i);
    }
}

template <typename T>
void SchemeFeaturer<T>::insert_nodes_b()
{
    temporaries.resize(temporaries.size() + 1);
    NodeCore * nc {&temporaries.last()};
    nc->n_class = static_cast<NodeClass>(random_node_class(reng));
    nc->required_subgraph_id = f_required_id;
    nc->subgraph_kind = f_subgraph_kind;
    //TODO ....
    temporary_inds.append(n_place.t);
    transformed_inds[n_place.t.ind].ind = n_place.r.ind =
                                                      temporary_inds.size() - 1;
    n_place.core = &temporaries[n_place.r.ind];
    n_place.r.ind += T::temporary_inds_start;
    ri = erasings.erase(ri); --ri;
    some_illegals.append(SimpleAntiBranch{n_place.t});
    ++graph_size;
}

template <typename T>
inline void SchemeFeaturer<T>::insert_nodes_d()
{
    c_place.core = n_place.core;
    c_place.r.ind = n_place.r.ind;
    --n_place.t.ind;
    insert_nodes_b();
    connect_nodes<true, ONLocation::temps>(n_place, c_place, 0);
    insert_nodes_c();
}

template <typename T>
template <bool root>
inline void SchemeFeaturer<T>::get_some_illegals_a(GraphPlace const & cu_place)
{
    if (root) {
        cu_place.core->scan_recepients_inds<false>(some_illegals, *this);
    } else if (cu_place.ccore->actual_subgraph_id <= f_required_id) {
        cu_place.ccore->scan_recepients_inds<false>(some_illegals, *this);
    }
}

template <typename T>
template <FLocation featured_place>
inline void SchemeFeaturer<T>::get_some_illegals(Feature const & f)
{
    some_illegals.clear();
    if (featured_place != FLocation::temps) {
        some_illegals.qi = some_illegals.end();
    } else {
        some_illegals.append(SimpleAntiBranch{f.f_place.t});
        some_illegals.qi = some_illegals.begin();
        get_some_illegals_a<true>(f.f_place);
        do {
            ++some_illegals.qi;

            if (some_illegals.qi == some_illegals.end()) { break; }
            get_some_illegals_a(some_illegals.qi->c_place);
        } while (true);
    }
}

template <typename T>
template <FLocation featured_place>
inline void SchemeFeaturer<T>::insert_nodes_a(Feature const & f)
{
    Q_ASSERT(f.in.steps >= 1);
    if (featured_place == FLocation::temps) {
        Q_ASSERT(f.f_place.t <= f.in.t - f.in.steps || f.in.t < f.f_place.t);
        if (f.in.t < f.f_place.t) {
            scroll(f.in.t - f.in.steps + 1, f.f_place.t, -f.in.steps);
        } else {
            scroll(f.f_place.t, f.in.t + 1, f.in.steps);
        }
        f_in_finish = n_place.t = f.f_place.t - 1;
    } else {
        f_in_finish = n_place.t = f.in.t;
    }
    f_in_start = f_in_finish - f.in.steps + 1;
    f_subgraph_kind = f.f_place.core->subgraph_kind;
    ri = std::lower_bound(erasings.cbegin(), erasings.cend(), n_place.t);
    Q_ASSERT(ri != erasings.cend() && ri->ind == n_place.t.ind);
}

template <typename T>
template <ONLocation old_place, FLocation featured_place>
inline void SchemeFeaturer<T>::insert_nodes(Feature & f)
{
    insert_nodes_a<featured_place>(f);
    get_some_illegals<featured_place>(f);
    insert_nodes_b();
    connect_insert<featured_place>(f);
    insert_nodes_c();
    InsertedNodes<T> queue;
    if (f.in.steps >= 2) {
        queue.append(n_place);
        for (qint32 i {2}; i < f.in.steps; ++i) {
            insert_nodes_d();
            queue.append(n_place);
        }
        insert_nodes_d();
    }
    insert_nodes_e<old_place, featured_place>(f, queue);
}

template <typename T>
template <bool reflected, ONLocation new_place,
          ONLocation old_place, FLocation featured_place>
inline void SchemeFeaturer<T>::connect_skip(Feature & f)
{
    static_assert(new_place != ONLocation::origs, "Err!");
    if (featured_place == FLocation::temps) {
        connect_nodes<reflected, new_place>(c_place, f.f_place, f.f_operand);
        correct_forward<reflected, new_place, old_place>(c_place, f);
    } else {
        connect_nodes<reflected, new_place, featured_place>(c_place, f);
    }
}

template <typename T>
template <ONLocation old_place, FLocation featured_place>
inline void SchemeFeaturer<T>::connect_memarg(Feature & f)
{
    random_n_place.param(RandQInt32P{T::graph_size_max, T::graph_size_max +
                                T::memory_size + get_legal_args_inds_finish()});
    c_place.ind = random_n_place(reng);
    if (c_place.ind < T::graph_size_max + T::memory_size) {
        connect_skip<false, ONLocation::memory, old_place, featured_place>(f);
    } else {
        connect_skip<false, ONLocation::args, old_place, featured_place>(f);
    }
}

template <typename T>
template <ONLocation old_place, FLocation featured_place>
inline void SchemeFeaturer<T>::skip_nodes_a(Feature & f)
{
    Q_ASSERT(c_place.ind < T::graph_size_max ||
             c_place.ind >= T::temporary_inds_start);
    if (c_place.ind < T::graph_size_max) {
        set_temporary_node(c_place);
        connect_skip<true, ONLocation::temps, old_place, featured_place>(f);
    } else {
        connect_skip<false, ONLocation::temps, old_place, featured_place>(f);
    }
}

template <typename T>
template <typename>
inline void OperandsFilter<T>::operator()(qint32 const ind)
{
    if (ind < T::graph_size_max || ind >= T::temporary_inds_start) {
        append(ind);
    }
}

template <typename T>
template <ONLocation old_place, FLocation featured_place>
inline void SchemeFeaturer<T>::skip_nodes(Feature & f)
{
    if (old_place == ONLocation::memory || old_place == ONLocation::args) {
        connect_memarg<old_place, featured_place>(f);
    } else {
        c_place.ccore = get_node_core<old_place>(f.o_place);
        OperandsFilter<T> wanted_inds;
        Q_ASSERT(f.in.steps >= 1);
        for (qint32 i {0}; i < f.in.steps; ++i) {
            wanted_inds.clear();
            c_place.ccore->scan_operands_inds(wanted_inds);
            if (wanted_inds.size() == 0) {
                connect_memarg<old_place, featured_place>(f);
                disconnect_tree<old_place, featured_place>(f);
                return;
            }
            random_operand.param(RandQInt32P{0, wanted_inds.size() - 1});
            c_place.ind = wanted_inds.at(random_operand(reng));
            c_place.ccore = get_node_core(c_place.ind);
        }
        skip_nodes_a<old_place, featured_place>(f);
        disconnect_tree<old_place, featured_place>(f);
    }
}

template <typename T>
template <ONLocation old_place, FLocation featured_place>
inline void SchemeFeaturer<T>::make_single_feature_b(Feature & f)
{
    static_assert(T::steps_number_max >= 1, "Err!");

    if (old_place == ONLocation::temps || old_place == ONLocation::origs) {
        get_node_core<false, true, old_place>(f.o_place);
    }
    if (graph_size + f.in.steps > T::graph_size_max) {
        skip_nodes<old_place, featured_place>(f);
    } else {
        insert_nodes<old_place, featured_place>(f);
    }
}

template <typename T>
template <FLocation featured_place>
inline void SchemeFeaturer<T>::make_single_feature_a(Feature & f)
{
    Q_ASSERT(f.o_place.ind < T::temporary_inds_start);
    if (f.o_place.ind < T::graph_size_max) {
        f.o_place.t.ind = original_inds.at(f.o_place.ind).ind;
        f.o_place.r.ind = transformed_inds.at(f.o_place.t.ind).ind;
        Q_ASSERT(f.o_place.r.ind >= 0);
        if (f.o_place.r.ind - T::graph_size_max < 0) {
            make_single_feature_b<ONLocation::origs, featured_place>(f);
        } else {
            make_single_feature_b<ONLocation::temps, featured_place>(f);
        }
    } else if (f.o_place.ind < T::graph_size_max + T::memory_size) {
        make_single_feature_b<ONLocation::memory, featured_place>(f);
    } else {
        make_single_feature_b<ONLocation::args, featured_place>(f);
    }
}

template <typename T>
inline void SchemeFeaturer<T>::make_single_feature(Feature & f)
{
    Q_ASSERT(f.f_place_loc != FLocation::temps);
    switch (f.f_place_loc) {
    case FLocation::origs:
    case FLocation::temps:
        f.f_place.t.ind = original_inds.at(f.f_place.ind).ind;
        set_temporary_node(f.f_place);
        f.o_place.ind = f.f_place.core->operands_inds[f.f_operand];
        f_required_id = f.f_place.core->required_subgraph_id;
        make_single_feature_a<FLocation::temps>(f); break;
    case FLocation::memory:
        f.f_place.ind = T::graph_size_max + f.mr_ind;
        f.o_place.ind = memory_inds[f.mr_ind];
        f_required_id = get_required_id<FLocation::memory>(f.mr_ind);
        make_single_feature_a<FLocation::memory>(f); break;
    case FLocation::result:
        f.f_place.ind = T::graph_size_max + T::memory_size + f.mr_ind;
        f.o_place.ind = result_inds[f.mr_ind];
        f_required_id = get_required_id<FLocation::result>(f.mr_ind);
        make_single_feature_a<FLocation::result>(f);
    }
}

template <typename T>
void SchemeFeaturer<T>::make()
{
    ni = features.begin(); mi = features.end();
    for (Feature & f : features) {
        ++ni;
        make_single_feature(f);
    }
}

template
SchemeFeaturer<SoulTraits>::SchemeFeaturer(DaemonScheme<SoulTraits> const &,
                                           DaemonScheme<SoulTraits> *);
template
void SchemeFeaturer<SoulTraits>::make();

template
SchemeFeaturer<AngelTraits>::SchemeFeaturer(DaemonScheme<AngelTraits> const &,
                                            DaemonScheme<AngelTraits> *);
template
void SchemeFeaturer<AngelTraits>::make();

}












