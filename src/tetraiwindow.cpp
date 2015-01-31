/*
 * PROJECT:  TetrAI
 * VERSION:  0.90
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
 
#include "tetraiwindow.h"

#include "ui_tetraiwindow.h"
#include "rectboardview.h"
#include <QActionGroup>
#include <QAction>
#include <QIntValidator>
#include <QCloseEvent>
#include <QApplication>
#include <QString>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QSize>
#include <QSizePolicy>

TetrAIWindow::TetrAIWindow(QWidget * parent) :
    QMainWindow(parent), ui(new Ui::TetrAIWindow),
    smode_label_text {tr("Falling interval in &slow mode: ")},
    fmode_label_text {tr("Update interval in f&ast mode: ")},
    ppower_label_text {tr("Processing powe&r in fast mode: ")},
    score_label_text {tr("Score: ")},
    lines_label_text {tr("Lines: ")},
    cycles_label_text {tr("Cycles: ")},
    overall_cycles_label_text {tr("Overall Cycles: ")},
    games_played_label_text {tr("Games Played: ")},
    average_score_label_text {tr("Average Score: ")},
    average_lines_label_text {tr("Average Lines: ")},
    ai_delay_label_text {tr("AI Module Delay: ")}
{
    ui->setupUi(this);
    game = new Tetrai{this};
    connect(game, &Tetrai::cycles_performed,
                                      ui->progressBar, &QProgressBar::setValue);
    connect(game, &Tetrai::game_stopped, this, &TetrAIWindow::press_pause);
    qRegisterMetaType<TetraiInfo>();
    connect(game, &Tetrai::game_info_changed,
                                         this, &TetrAIWindow::update_game_info);
    game_board = new RectBoardView{game};
    game_board->setWindowTitle(tr("TetrAI Game"));

    QString largest_str {smode_label_text}; QString ms {tr(" ms")};
    largest_str += QString::number(ui->slowSlider->maximum()*smode_slider_step);
    largest_str += ms;
    ui->slowSliderLabel->setMinimumSize(ui->slowSliderLabel->
                         fontMetrics().size(Qt::TextShowMnemonic, largest_str));
    largest_str = fmode_label_text;
    largest_str += QString::number(ui->fastSlider->maximum()*fmode_slider_step);
    largest_str += ms;
    ui->fastSliderLabel->setMinimumSize(ui->fastSliderLabel->
                         fontMetrics().size(Qt::TextShowMnemonic, largest_str));
    largest_str = ppower_label_text;
    largest_str += QString::number(
                                ui->ppowerSlider->maximum()*ppower_slider_step);
    largest_str += "%";
    ui->ppowerSliderLabel->setMinimumSize(ui->ppowerSliderLabel->
                         fontMetrics().size(Qt::TextShowMnemonic, largest_str));
    on_slowSlider_valueChanged(ui->slowSlider->value());
    on_fastSlider_valueChanged(ui->fastSlider->value());
    on_ppowerSlider_valueChanged(ui->ppowerSlider->value());

    largest_str = QString::number(cycles_max);
    ui->cyclesLineEdit->setMinimumSize(ui->cyclesLineEdit->
                                            fontMetrics().size(0, largest_str));
    ui->cyclesLineEdit->setValidator(new QIntValidator{0, cycles_max, this});
    on_cyclesLineEdit_textChanged(ui->cyclesLineEdit->text());

    largest_str = score_label_text + "4294967296";
    ui->scoreInfoLabel->setMinimumSize(ui->scoreInfoLabel->
                                            fontMetrics().size(0, largest_str));
    largest_str = lines_label_text + "2147483648";
    ui->linesInfoLabel->setMinimumSize(ui->linesInfoLabel->
                                            fontMetrics().size(0, largest_str));
    largest_str = cycles_label_text + tr("2147483648 of 2147483648");
    ui->cyclesInfoLabel->setMinimumSize(ui->cyclesInfoLabel->
                                            fontMetrics().size(0, largest_str));
    largest_str = overall_cycles_label_text + "18446744073709551616";
    ui->overallcyclesInfoLabel->setMinimumSize(ui->overallcyclesInfoLabel->
                                            fontMetrics().size(0, largest_str));
    largest_str = games_played_label_text + "2147483648";
    ui->gamesplayedInfoLabel->setMinimumSize(ui->gamesplayedInfoLabel->
                                            fontMetrics().size(0, largest_str));
    largest_str = average_score_label_text + "4294967296";
    ui->averagescoreInfoLabel->setMinimumSize(ui->averagescoreInfoLabel->
                                            fontMetrics().size(0, largest_str));
    largest_str = average_lines_label_text + "2147483648";
    ui->averagelinesInfoLabel->setMinimumSize(ui->averagelinesInfoLabel->
                                            fontMetrics().size(0, largest_str));
    largest_str = ai_delay_label_text + tr("18446744073709551616 us");
    ui->aidelayInfoLabel->setMinimumSize(ui->aidelayInfoLabel->
                                            fontMetrics().size(0, largest_str));

    create_new_training();
    nextfig_model = new NextFigureModel{game};
    nextfig_board = new RectBoardView{nextfig_model, ui->nextfigBox};
    ui->nextfigLayout->addWidget(nextfig_board, 0, Qt::AlignCenter);
    nextfig_board->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    nextfig_board->setAutoFillBackground(true);
    nextfig_board->zeros_rendering = false;

    QSize largest_figure {nextfig_model->get_largest_sprite_size()};
    largest_figure.rwidth() += ui->nextfigLayout->contentsMargins().left();
    largest_figure.rwidth() += ui->nextfigLayout->contentsMargins().right();
    largest_figure.rheight() += ui->nextfigLayout->contentsMargins().top();
    largest_figure.rheight() += ui->nextfigLayout->contentsMargins().bottom();
    largest_figure.rwidth() += ui->nextfigBox->contentsMargins().left();
    largest_figure.rwidth() += ui->nextfigBox->contentsMargins().right();
    largest_figure.rheight() += ui->nextfigBox->contentsMargins().top();
    largest_figure.rheight() += ui->nextfigBox->contentsMargins().bottom();
    ui->nextfigBox->setMinimumSize(largest_figure);

    speed_mode_ui = new QActionGroup{this};
    speed_mode_ui->addAction(ui->actionStartSlow);
    speed_mode_ui->addAction(ui->actionStartFast);
    speed_mode_ui->addAction(ui->actionPause);
    speed_mode_ui->addAction(ui->actionStop);

    ui->buttonStartSlow->setDefaultAction(ui->actionStartSlow);
    ui->buttonStartFast->setDefaultAction(ui->actionStartFast);
    ui->buttonPause->setDefaultAction(ui->actionPause);
    ui->buttonStop->setDefaultAction(ui->actionStop);
    ui->buttonSwitchHumanAI->setDefaultAction(ui->actionSwitchHumanAI);

    game_board->setFixedSize(game_board->sizeHint());
    game_board->setAttribute(Qt::WA_NoSystemBackground);
    game_board->setFocusPolicy(Qt::StrongFocus);
    game_board->addAction(ui->actionStartSlow);
    game_board->addAction(ui->actionStartFast);
    game_board->addAction(ui->actionPause);
    game_board->addAction(ui->actionStop);
    game_board->addAction(ui->actionSwitchHumanAI);
    game_board->addAction(ui->actionExit);
    game_board->installEventFilter(this);
    game_board->installEventFilter(game);
    game_board->show();
    ui->actionToggleGameWindow->setChecked(true);
}

TetrAIWindow::~TetrAIWindow()
{
    delete game_board;
    delete ui;
}

bool TetrAIWindow::eventFilter(QObject *, QEvent * ev)
{
    if (ev->type() == QEvent::Close) {
        ui->actionToggleGameWindow->setChecked(false);
    }
    return false;
}

void TetrAIWindow::closeEvent(QCloseEvent * ev)
{
    game->stop_tetrai_blocking(true);
    if (!suggest_to_save()) {
        ev->ignore();
        return;
    }

    game_board->close(); ev->accept();
    QCoreApplication::exit(0);
}

void TetrAIWindow::on_actionExit_triggered(bool)
{
    game->stop_tetrai_blocking(true);
    if (!suggest_to_save()) { return; }

    game_board->close();
    QCoreApplication::exit(0);
}

void TetrAIWindow::on_actionStartSlow_triggered(bool)
{
    game->start_tetrai(SpeedMode::slow);
    setWindowModified(true);
    ui->stackedWidget->setCurrentWidget(ui->nextfigPage);
}

void TetrAIWindow::on_actionStartFast_triggered(bool)
{
    game->start_tetrai(SpeedMode::fast);
    setWindowModified(true);
    ui->stackedWidget->setCurrentWidget(ui->blankPage);
}

void TetrAIWindow::on_actionPause_triggered(bool)
{
    game->stop_tetrai(false);
    ui->stackedWidget->setCurrentWidget(ui->nextfigPage);
}

void TetrAIWindow::on_actionStop_triggered(bool)
{
    game->stop_tetrai(false);
    game->reset_cycles_count();
    ui->stackedWidget->setCurrentWidget(ui->nextfigPage);
}

void TetrAIWindow::on_actionSwitchHumanAI_triggered(bool pl)
{
    QAction * qact {ui->actionSwitchHumanAI};
    if (pl) {
        qact->setText(tr("Sw&itch to AI"));
        qact->setToolTip(tr("Switch to \"AI\" player mode"));
    } else {
        qact->setText(tr("Sw&itch to Human"));
        qact->setToolTip(tr("Switch to \"Human\" player mode"));
    }
    game->set_player_mode(pl);
}

void TetrAIWindow::on_actionToggleGameWindow_triggered(bool vis)
{
    if (vis) {
        game_board->show();
    } else {
        game_board->hide();
    }
}

void TetrAIWindow::on_slowSlider_valueChanged(int value)
{
    QString new_text {smode_label_text};
    new_text += QString::number(value*smode_slider_step) + tr(" ms");
    ui->slowSliderLabel->setText(new_text);
    game->set_slow_mode_interval(value*smode_slider_step);
}

void TetrAIWindow::on_fastSlider_valueChanged(int value)
{
    QString new_text {fmode_label_text};
    new_text += QString::number(value*fmode_slider_step) + tr(" ms");
    ui->fastSliderLabel->setText(new_text);
    game->set_fast_mode_interval(value*fmode_slider_step);
}

void TetrAIWindow::on_ppowerSlider_valueChanged(int value)
{
    QString new_text {ppower_label_text};
    new_text += QString::number(value*ppower_slider_step) + "%";
    ui->ppowerSliderLabel->setText(new_text);
    game->set_processing_power(value*ppower_slider_step);
}

void TetrAIWindow::on_cyclesLineEdit_textChanged(const QString & txt)
{
    game->set_cycles_count_max(txt.toInt());
}

void TetrAIWindow::press_pause()
{
    ui->actionPause->setChecked(true);
    if (game->is_game_over()) {
        ui->stackedWidget->setCurrentWidget(ui->blankPage);
    } else {
        ui->stackedWidget->setCurrentWidget(ui->nextfigPage);
    }
}

void TetrAIWindow::update_game_info(TetraiInfo info)
{
    ui->scoreInfoLabel->setText(score_label_text + QString::number(info.score));
    ui->linesInfoLabel->setText(lines_label_text
                                           + QString::number(info.lines_count));
    ui->cyclesInfoLabel->setText(cycles_label_text
                               + QString::number(info.cycles_count) + tr(" of ")
                                      + QString::number(info.cycles_count_max));
    ui->overallcyclesInfoLabel->setText(overall_cycles_label_text
                                  + QString::number(info.overall_cycles_count));
    ui->gamesplayedInfoLabel->setText(games_played_label_text
                                           + QString::number(info.games_count));
    ui->averagescoreInfoLabel->setText(average_score_label_text
                                             + QString::number(info.score_avg));
    ui->averagelinesInfoLabel->setText(average_lines_label_text
                                       + QString::number(info.lines_count_avg));
    ui->aidelayInfoLabel->setText(ai_delay_label_text
                           + QString::number(info.ai_module_delay) + tr(" us"));
}

void TetrAIWindow::on_actionCreateNew_triggered()
{
    game->stop_tetrai_blocking(true);
    if (!suggest_to_save()) { return; }

    create_new_training();
    ui->actionStop->setChecked(true);
}

void TetrAIWindow::create_new_training()
{
    game->create(); training_name = tr("noname"); new_training_name = true;
    QString suffix {}; QDir app_dir {}; int i {2};
    while (app_dir.exists(training_name + suffix + ".ai")) {
        suffix = "(" + QString::number(i) + ")"; ++i;
    }
    training_name += suffix + ".ai";
    training_name = app_dir.absoluteFilePath(training_name); refresh_title();
}

int TetrAIWindow::suggest_to_save()
{
    if (isWindowModified()) {
        QMessageBox msg;
        msg.setText(tr("During training new results have been obtained."));
        msg.setInformativeText(tr("Do you want to save them?"));
        msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard
                                                         | QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Save);
        msg.setIcon(QMessageBox::Question);
        int answer {msg.exec()};
        if (answer == QMessageBox::Save) {
            on_actionSave_triggered();
        } else if (answer == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void TetrAIWindow::refresh_title()
{
    QFileInfo temp {training_name}; QString training_title {temp.fileName()};
    setWindowTitle(training_title + "[*] - " + tr("TetrAI Controls"));
    setWindowModified(false);
}

void TetrAIWindow::on_actionOpen_triggered()
{
    game->stop_tetrai_blocking(true);
    if (!suggest_to_save()) { return; }

    QFileDialog dlg {this};
    dlg.setWindowTitle(tr("Open"));
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setNameFilter(tr("TetrAI files (*.ai);;All files (*.*)"));
    dlg.setViewMode(QFileDialog::Detail);
    if (dlg.exec() && dlg.selectedFiles().size() > 0) {
        QString selected {dlg.selectedFiles().at(0)};
        QFile fl {selected};
        if (game->load(fl)) {
            training_name = selected;
            refresh_title();
        } else {
            QMessageBox msg;
            msg.setText(tr("Cannot open selected file."));
            msg.setInformativeText(selected);
            msg.setIcon(QMessageBox::Critical);
            msg.exec();
        }
    }
}

void TetrAIWindow::on_actionSave_triggered()
{
    game->stop_tetrai_blocking(true);
    if (!new_training_name) {
        QFile fl {training_name};
        game->save(fl);
        setWindowModified(false);
    } else {
        QFileDialog dlg {this};
        dlg.setWindowTitle(tr("Save New"));
        dlg.setFileMode(QFileDialog::AnyFile);
        dlg.setNameFilter(tr("TetrAI files (*.ai);;All files (*.*)"));
        dlg.setDefaultSuffix("ai");
        dlg.setViewMode(QFileDialog::Detail);
        QFileInfo temp {training_name};
        dlg.setDirectory(temp.path());
        dlg.selectFile(training_name);
        if (dlg.exec() && dlg.selectedFiles().size() > 0) {
            training_name = dlg.selectedFiles().at(0);
            new_training_name = false;
            QFile fl {training_name};
            game->save(fl); refresh_title();
        }
    }
}

void TetrAIWindow::on_actionSaveAs_triggered()
{
    game->stop_tetrai_blocking(true);
    QFileDialog dlg {this};
    dlg.setWindowTitle(tr("Save As"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setNameFilter(tr("TetrAI files (*.ai);;All files (*.*)"));
    dlg.setDefaultSuffix("ai");
    dlg.setViewMode(QFileDialog::Detail);
    if (dlg.exec() && dlg.selectedFiles().size() > 0) {
        training_name = dlg.selectedFiles().at(0);
        new_training_name = false;
        QFile fl {training_name};
        game->save(fl); refresh_title();
    }
}
