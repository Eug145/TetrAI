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

#ifndef AIMODULE_C_H
#define AIMODULE_C_H

#include "aimodule_a_intf_ba_c.h"
#include <QVector>
#include <QVarLengthArray>

namespace AIModule {

enum class SubgraphKind : qint32 {none = 0, memory = 1, result = 2};




enum class NodeClass : qint32 {notc, orc, shlc, shrc};

using RecepientsByOperand = QVarLengthArray<qint32,
            Consts::recepients_supposed_number/Consts::operands_number_min + 1>;

class NodeCore
{
public:
    NodeClass n_class;
    SubgraphKind subgraph_kind;
    qint32 actual_subgraph_id, required_subgraph_id;
    qint32 operands_inds[Consts::operands_number_max];
    RecepientsByOperand recipients_inds[Consts::operands_number_max];

public:
    qint32 get_operands_number() const;
    template <typename ScanAction, typename ...Args>
    void scan_operands_inds(ScanAction & a, Args const & ...args) const;
    template <bool breakable, typename ScanAction, typename ...Args>
    void scan_recepients_inds(ScanAction & a, Args const & ...args) const;
};

template <typename T>
class DaemonSchemeData;

template <typename T>
class Node : public NodeCore
{
public:
    qint32 ind;
    qint32 (Node::* calculate)(DaemonSchemeData<T> & hs);

public:
    qint32 calculate_not(DaemonSchemeData<T> & hs);
    qint32 calculate_or(DaemonSchemeData<T> & hs);
    qint32 calculate_shl(DaemonSchemeData<T> & hs);
    qint32 calculate_shr(DaemonSchemeData<T> & hs);
};

template <typename T>
class DaemonSchemeData
{
public:
    QVector<qint32> args;
    qint32 memory[T::memory_size];

    QVarLengthArray<qint32, 1> results;

public:
    explicit DaemonSchemeData(qint32 sz = 0);
};




constexpr SubgraphKind operator|(SubgraphKind const a, SubgraphKind const b)
{
    return static_cast<SubgraphKind>(static_cast<int>(a)|static_cast<int>(b));
}

constexpr bool test(SubgraphKind const a, SubgraphKind const b)
{
    return static_cast<int>(a)&static_cast<int>(b);
}

inline qint32 NodeCore::get_operands_number() const
{
    return Consts::operands_numbers[static_cast<qint32>(n_class)];
}

template <typename ScanAction, typename ...Args>
inline void NodeCore::scan_operands_inds(ScanAction & a,
                                         Args const & ...args) const
{
    int op_number {get_operands_number()};
    Q_ASSERT(op_number >= 1);
    qint32 const * si {std::begin(operands_inds)};
    qint32 const * ei {si + op_number};
    while (si != ei) {
        a.template operator()(*si, args...); ++si;
    }
}

template <bool breakable, typename ScanAction, typename ...Args>
inline void NodeCore::scan_recepients_inds(ScanAction & a,
                                           Args const & ...args) const
{
    RecepientsByOperand const * ti {std::begin(recipients_inds)};
    RecepientsByOperand const * zi {std::end(recipients_inds)};
    while (ti != zi) {
        RecepientsByOperand::const_iterator si {ti->cbegin()};
        RecepientsByOperand::const_iterator ei {ti->cend()};
        if (breakable) {
            while (si != ei) {
                if (!a.template operator()(*si, args...)) {
                    goto who_likes_goto_now_srsly;
                }
                ++si;
            }
        } else {
            while (si != ei) {
                a.template operator()(*si, args...); ++si;
            }
        }
        ++ti;
    }
    who_likes_goto_now_srsly:;
}

}

#endif // AIMODULE_C_H
