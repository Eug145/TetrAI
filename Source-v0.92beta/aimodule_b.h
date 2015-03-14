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

#ifndef AIMODULE_B_H
#define AIMODULE_B_H

#include "aimodule_b_intf.h"

#include <random>

namespace AIModule {

extern std::uniform_int_distribution<qint64> random_connection;
extern std::uniform_int_distribution<qint32> random_operand;
extern std::uniform_int_distribution<qint32> random_n_place;

constexpr BCorr operator|(BCorr const a, BCorr const b);
constexpr bool test(BCorr const a, BCorr const b);




template <typename B>
class AntiBranchIterator : public std::iterator<std::forward_iterator_tag, B>
{
    using Iter = typename QLinkedList<B>::iterator;

    Iter itr;

public:
    explicit AntiBranchIterator(Iter const i);
    TIndex & operator*() const;
    B * operator->() const;
    bool operator==(AntiBranchIterator const & a) const;
    bool operator!=(AntiBranchIterator const & a) const;
    AntiBranchIterator & operator++();
};




using Recepients = QVarLengthArray<qint32, Consts::recepients_supposed_number>;

using Operands = QVarLengthArray<qint32, Consts::operands_number_max>;

template <typename T, bool root>
class LastRecepientChecker
{
public:
    bool the_last;

public:
    template <typename ...R>
    bool operator()(qint32 const ind, SchemeFeaturer<T> const & sc_f,
                    R ...other);
};

template <typename T>
class OperandsFilter : public Operands
{
public:
    template <typename = void>
    void operator()(qint32 const ind);
};

template <typename T>
class OperandsActualID
{
public:
    qint32 actual_id;

private:
    void compare(qint32 const id);

public:
    template <typename = void>
    void operator()(qint32 const ind, SchemeFeaturer<T> const & sc_f);

    explicit OperandsActualID(qint32 const id = 0);
};

template <typename T, BCorr control>
class RecepientsInvariant
{
    TIndex t;

public:
    SubgraphKind subgraph_kind;
    TIndex string_starts_min;
    qint32 required_id;

private:
    template <FLocation place, typename ...RCC>
    void compare_req_id(qint32 const nmrot_ind,
                        SchemeFeaturer<T> const & sc_f, RCC ...rcc);
    template <FLocation place>
    void compare_indices(qint32 const nmrot_ind,
                         SchemeFeaturer<T> const & sc_f);
    template <FLocation place, typename ...RCC>
    void merge_kind(RCC ...rcc);
    template <FLocation place, typename ...RCC>
    void inspect_a(qint32 const nmrot_ind,
                   SchemeFeaturer<T> const & sc_f, RCC ...rcc);
    template <FLocation place>
    void inspect(qint32 const nmrot_ind, SchemeFeaturer<T> const & sc_f);

public:
    template <typename = void>
    bool operator()(qint32 const ind, SchemeFeaturer<T> const & sc_f);

    explicit RecepientsInvariant(SchemeFeaturer<T> const & sc_f);
};

}

#endif // AIMODULE_B_H
