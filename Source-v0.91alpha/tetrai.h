/*
 * PROJECT:  TetrAI
 * VERSION:  0.91alpha
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

#ifndef TETRAI_H
#define TETRAI_H

#include "rectboardview.h"
#include "history.h"
#include <QObject>
#include <QVector>
#include <QImage>
#include <QRegion>
#include <QSize>
#include <QMutex>
#include <QString>
#include <QFile>

class QTimer;
class QKeyEvent;
class NextFigureModel;
class QThread;
class TetraiWorker;

enum class SpeedMode {stop, slow, fast};

enum class PlayerMode {human, ai};

enum class PlayerAct {any, none, rotate, left, right, down, drop};

struct TetraiInfo
{
    unsigned int score {0};
    unsigned int score_avg {0};
    int lines_count {0};
    int lines_count_avg {0};
    qint32 cycles_count {0};
    qint32 cycles_count_max {0};
    quint64 overall_cycles_count {0};
    qint32 games_count {0};
    long ai_module_delay {0}; //microseconds
};

Q_DECLARE_METATYPE(TetraiInfo)




struct TetraiState
{
    QMutex general {QMutex::Recursive};

    SpeedMode speed {SpeedMode::stop};
    PlayerMode player {PlayerMode::ai};
    PlayerAct act {PlayerAct::none};
    int nx {0}, ny {0};
    QVector<int> board {};
    bool next_figure_is_rolled {false};
    QSize next_figure_size {};
    QVector<int> next_figure_sprite {};
    TetraiInfo info {};
    History<unsigned int> score_history {10};
    History<int> lines_count_history {10};
    long smode_wait_interval {0}; //microseconds
    float fmode_respite_ratio {0};
    bool void_mcycle {false};
    bool dropping {false};
    bool cleaning {false};
    bool game_over {false};
};




class Tetrai : public QObject, public RectBoardModel
{
    Q_OBJECT

public:
    static int const inter_mcycle_number {4};

    static int const board_size_x {10};
    static int const board_size_y {25};
    static int const board_size {board_size_x*board_size_y};

    static int const largest_sprite_nx {3};
    static int const largest_sprite_ny {4};
    static int const largest_sprite_size {largest_sprite_nx*largest_sprite_ny};

    static int const score_history_size {10};
    static int const lines_count_history_size {10};

private:
    static int const void_mcycle_number {0};
    static int const dropping_iratio {4};

    RectBoardView * tetrai_view;
    QVector<int> * view_board;

    NextFigureModel * next_fig_model;
    QTimer * update_timer;

    QThread * worker_thread;
    TetraiWorker * tetrai_worker;
    TetraiState * mr;

    QVector<QImage> chips;

    int smode_interval, smode_interval_remains, fmode_interval; //milliseconds
    SpeedMode spe;

public:
    explicit Tetrai(QObject * parent = 0);
    ~Tetrai() override;

    void set_view(RectBoardView * rbv) override;

    bool eventFilter(QObject *, QEvent * ev) override;

    void set_next_figure_model(NextFigureModel * nfm);
    void set_slow_mode_interval(int smode_i);
    void set_fast_mode_interval(int fmode_i);
    void set_processing_power(int percent);
    void set_player_mode(bool plm);
    void set_cycles_count_max(int cycm);
    void reset_cycles_count();
    int get_xsize();
    int get_ysize();
    QVector<QImage> * get_chips();
    bool check_game_over();

    void start_tetrai(SpeedMode spd);
    void stop_tetrai(bool stopsig);
    void stop_tetrai_blocking(bool stopsig);
    void create();
    bool load(QFile & fl);
    void save(QFile & fl);

signals:
    void new_game_requested();
    void refresh_requested_a();
    void tetrai_smode_update_requested();
    void tetrai_fmode_update_requested();
    void refresh_requested_b();
    void tetrai_stop_requested(bool stopsig);
    void tetrai_blocking_stop_requested();
    void create_new_training_requested();
    void load_training_requested(QFile & fl);
    void save_training_requested(QFile & fl);
    void cycles_performed(int percent);
    void game_stopped();
    void game_info_changed(TetraiInfo info);

private:
    void emit_cycles_performed(qint32 cyc, qint32 cycm);
    void update_next_fig_model();

private slots:
    void start_tetrai_aux();
    void update_tetrai_view();
    void update_tetrai_view_smode();
    void update_tetrai_view_fmode();
    void update_tetrai_view_a();
    void stop_tetrai_aux(bool stopsig);
};




class NextFigureModel : public QObject, public RectBoardModel
{
    Q_OBJECT

private:    
    Tetrai * game;
    RectBoardView * next_figure_view;
    QVector<int> * view_board;

    QSize next_figure_size;
    QVector<int> next_figure_sprite;

    QSize chip_size;

public:
    explicit NextFigureModel(Tetrai * gm);
    ~NextFigureModel() override;

    void set_view(RectBoardView * rbv) override;

    void update(QSize nfs, QVector<int> fig_spr);
    QSize get_largest_sprite_size();

private:
    void update_view();
};

#endif // TETRAI_H
