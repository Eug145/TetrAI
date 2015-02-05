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

#ifndef AIMODULE_B_INTF_AA_H
#define AIMODULE_B_INTF_AA_H

#include "aimodule_a_intf_ba.h"
#include <QVector>
#include <QVarLengthArray>

namespace AIModule {

enum class NodeClass : qint32 {dummy, adder, subtractor, multiplier, divider};

class NodeCore
{
public:
    NodeClass n_class;
    qint32 actual_subgraph_id, required_subgraph_id;
    qint32 operands_inds[Consts::operands_number_max];
    QVarLengthArray<qint32, 1> recipients_inds[Consts::operands_number_max];
    qint32 recepients_number;

public:
    qint32 get_operands_number() const;
};

template <typename T>
class Node : public NodeCore
{
public:
    qint32 ind;
    qint32 (Node::* calculate)(DaemonSchemeData<T> & hs);

public:
    qint32 calculate_addition(DaemonSchemeData<T> & hs);
    qint32 calculate_subtraction(DaemonSchemeData<T> & hs);
    qint32 calculate_multiplication(DaemonSchemeData<T> & hs);
    qint32 calculate_division(DaemonSchemeData<T> & hs);
};

}

#endif // AIMODULE_B_INTF_AA_H
