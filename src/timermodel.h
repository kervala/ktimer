/*
 *  kTimer is a timers manager
 *  Copyright (C) 2021  Cedric OCHS
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

#ifndef TIMERNMODEL_H
#define TIMERMODEL_H

#include "timer.h"

#include <QAbstractTableModel>

class TimerModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	TimerModel(QObject* parent = nullptr);
	virtual ~TimerModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool insertRows(int position, int rows, const QModelIndex& index = QModelIndex()) override;
	bool removeRows(int position, int rows, const QModelIndex& index = QModelIndex()) override;

	bool newTimer();
	bool removeTimer(int row);
	void updateTimer(int row);

	Timer& getTimer(int row);

	bool startTimer(int row);
	bool stopTimer(int row);
	bool resetTimer(int row);
	bool isTimerRunning(int row);

	void reset();

	bool load(const QString& filename);
	bool save(const QString& filename, bool resume = false);

	QString getFilename() const;

signals:
	void timerFinished(int row);

public slots:
	void onTimeout();

private:
	QVector<Timer> m_timers;

	QString m_filename;
};

#endif
