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

#ifndef AIMODULE_A_H
#define AIMODULE_A_H

#include "aimodule_a_intf_a.h"
#include "aimodule_a_intf_ba.h"
#include "aimodule_a_intf_bb.h"

#include <QVarLengthArray>
#include <QVector>
#include <QtGlobal>

namespace AIModule
{

extern quint32 sensor_score;
extern QVector<qint32> sensor;

extern Angel angels[AngelTraits::cluster_size];
extern Ptrs<AngelTraits> angels_ptrs;
extern Soul souls[SoulTraits::cluster_size];
extern Ptrs<SoulTraits> souls_ptrs;

extern QVector<qint32> args_with_plan;
extern QVector<qint32> args_with_act;
extern Angel * best_angel;
extern Soul * best_soul;
extern QVector<qint32> current_plan;
extern qint32 current_act;

void evaluate_angels_projections();
void evaluate_angels();
void get_best_angel(Ptrs<AngelTraits> const & all_ptrs);

void evaluate_souls_plans();
void evaluate_souls();
void get_best_soul(Ptrs<SoulTraits> const & all_ptrs);

template <typename T>
void sort_ptrs_by_age(Ptrs<T> & all_ptrs);
template <typename T, typename R>
void evaluate_in_age_groups(Ptrs<T> const & all_ptrs_sorted,
                            R & rating_comparing);
template <typename T, typename R>
void transform_daemons(Ptrs<T> & age_grouped_ptrs,
                       R & rating_comparing);
template <typename T, typename R>
Dmn<T> * take_the_best_daemon(Ptrs<T> & age_grouped_ptrs,
                              R & rating_comparing);

void update_current_plan();
void update_current_act();
void update_souls();
void update_angels();

QVector<qint32> create_args(qint32 args_size);
void copy_plan_to_args(QVector<qint32> plan, QVector<qint32> & args);

template <typename T>
void init_daemons_inds(Dmn<T> daemons[]);
template <typename T>
Ptrs<T> create_daemons_ptrs(Dmn<T> * cluster);

}

#endif // AIMODULE_A_H
