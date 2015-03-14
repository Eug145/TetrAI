/*
 * PROJECT:  TetrAI
 * VERSION:  0.92beta
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

#ifndef TETRAIWINDOW_H
#define TETRAIWINDOW_H

#include "tetrai.h"
#include <QMainWindow>
#include <QString>
#include <QLabel>

namespace Ui
{
class TetrAIWindow;
}

class QObject;
class QWidget;
class QActionGroup;
class QCloseEvent;
class QEvent;
class Tetrai;
class RectBoardView;
class NextFigureModel;

class TetrAIWindow : public QMainWindow
{
    Q_OBJECT

private:
    static int const smode_slider_step {10};
    static int const fmode_slider_step {100};
    static int const ppower_slider_step {10};
    static int const cycles_max {1048575};

    Ui::TetrAIWindow * ui;
    QActionGroup * speed_mode_ui;

    Tetrai * game;
    RectBoardView * game_board;
    NextFigureModel * nextfig_model;
    RectBoardView * nextfig_board;
    QLabel * a;

    QString smode_label_text;
    QString fmode_label_text;
    QString ppower_label_text;
    QString score_label_text;
    QString lines_label_text;
    QString cycles_label_text;
    QString overall_cycles_label_text;
    QString games_played_label_text;
    QString average_score_label_text;
    QString average_lines_label_text;
    QString ai_delay_label_text;

    QString training_name;
    bool new_training_name;

public:
    explicit TetrAIWindow(QWidget * parent = 0);
    ~TetrAIWindow() override;

    bool eventFilter(QObject *, QEvent * ev) override;

protected:
    void closeEvent(QCloseEvent * ev) override;

private slots:
    void on_actionExit_triggered(bool);
    void on_actionStartSlow_triggered(bool);
    void on_actionStartFast_triggered(bool);
    void on_actionPause_triggered(bool);
    void on_actionStop_triggered(bool);
    void on_actionSwitchHumanAI_triggered(bool pl);
    void on_actionToggleGameWindow_triggered(bool vis);
    void on_slowSlider_valueChanged(int value);
    void on_fastSlider_valueChanged(int value);
    void on_ppowerSlider_valueChanged(int value);
    void on_cyclesLineEdit_textChanged(QString const & txt);
    void press_pause();
    void update_game_info(TetraiInfo info);
    void on_actionCreateNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();

private:
    void create_new_training();
    int suggest_to_save();
    void refresh_title();
};

#endif // TETRAIWINDOW_H
