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

#ifndef TETRAIWORKER_H
#define TETRAIWORKER_H

#include "tetrai.h"
#include <random>
#include <QObject>
#include <QVector>

class TetraiWorker : public QObject
{
    Q_OBJECT

public:
    TetraiState mr;

private:
    struct Borders {
        QVector<int> top {};
        QVector<int> bottom {};
    };

    static QVector<QVector<int>> figures;
    static QVector<int> figure_start_xpos;
    static QVector<int> figure_start_ypos;

    std::default_random_engine reng;
    std::uniform_int_distribution<> random_figure_index;

    int ny_one_third, ny_two_thirds;
    int figure_xpos, figure_ypos, figure_rpos;
    int next_figure, figure;
    int occupied_lines;
    int mcycles_count;
    int heap_cleaner_scoreq;
    int clump_number;
    bool heap_cleaner_dropping;
    bool new_game;
    QVector<int> figures_heap, clump_mask;
    QVector<Borders> clumps_borders;
    QVector<int> full_lines;
    QVector<int> clumps;

public:
    TetraiWorker();
    ~TetraiWorker();

signals:
    void cycles_count_incremented(int percent);
    void tetrai_stopped(bool stopsig);
    void new_game_prepared();
    void state_refreshed_a();
    void tetrai_smode_updated();
    void state_refreshed_b();

public slots:
    void create();
    void load(QFile & fl);
    void save(QFile & fl);
    void prepare_new_game();
    void refresh_state_a();
    void refresh_state_b();
    void update_tetrai_smode();
    void update_tetrai_fmode();
    void stop_tetrai(bool stopsig);
    void stop_tetrai_blocking();

private:
    void prepare_new_game_aux();
    void roll_next_figure();
    void refresh_next_figure_sprite();
    void update_tetrai();
    void increment_gameover_counts();
    void stop_tetrai_aux();
    void increment_cycles_count();
    void increment_mcycles_count();
    void do_void_mcycle();
    void do_inter_mcycle();
    PlayerAct do_ai_cycle();
    void perform_player_act(PlayerAct ai_act);
    void respite();
    bool rotate();
    bool move_left();
    bool move_right();
    bool move_down();
    void add_move_down_score();
    int count_occupied_lines();
    bool check_full_lines();
    void clean_figures_heap();
    void add_lines_score();
    void clean_full_lines();
    void find_clumps();
    void mark_clump(int curr_i);
    bool drop_clumps_smode();
    bool drop_clumps_fmode();
    bool check_clump_collision(int clump_n);
    void move_clumps_down();
    bool check_collision();
    void add_figure_to_heap();
    void refresh_board();
    template <typename ScanAction>
    bool scan_figure(ScanAction a);
};

#endif // TETRAIWORKER_H
