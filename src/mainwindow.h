/*
 *  AutoClicker is a tool to click automatically
 *  Copyright (C) 2017-2019  Cedric OCHS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "systrayicon.h"
#include "timer.h"

class QWinTaskbarButton;
class Updater;
class TimerModel;

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();

public slots:
	// buttons
	void onAddClicked();
	void onRemoveClicked();
	void onStartClicked();
	void onStopClicked();

	void onNameChanged(const QString &name);
	void onTimerSelected(const QItemSelection& selected, const QItemSelection& deselected);

	// file menu
	void onNew();
	void onOpen();
	void onSave();
	void onSaveAs();

	// help menu
	void onCheckUpdates();
	void onAbout();
	void onAboutQt();

	// other
	void onMinimize();
	void onRestore();
	void onSystrayAction(SystrayIcon::SystrayAction action);

	// signals from OAuth2
	void onNewVersion(const QString &url, const QString &date, uint size, const QString &version);
	void onNoNewVersion();
	void onProgress(qint64 readBytes, qint64 totalBytes);

	void onDelayChanged(int delay);
	void onTimerFinished(int row);

	void onChangeSystrayIcon();

protected:
	void showEvent(QShowEvent *e);
	void closeEvent(QCloseEvent *e);
	void resizeEvent(QResizeEvent *e);
	void moveEvent(QMoveEvent *e);
	bool event(QEvent *e);

	void displayTimer(int i);
	void updateButtons();

	void updateTimerFromCurrent();
	void updateCurrentFromTimer();

	int toTimestamp();
	bool fromTimeStamp(int time);

	QWinTaskbarButton *m_button;

	Ui::MainWindow *m_ui;

	Updater *m_updater;

	Timer m_currentTimer;

	int m_selectedTimer;

	TimerModel* m_model;
};

#endif
