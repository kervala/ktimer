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
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool insertRows(int position, int rows, const QModelIndex& index = QModelIndex()) override;
	bool removeRows(int position, int rows, const QModelIndex& index = QModelIndex()) override;

	Qt::DropActions supportedDropActions() const override;
	QStringList mimeTypes() const override;
	QMimeData* mimeData(const QModelIndexList& indexes) const override;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

	bool addTimer(const Timer& timer);
	bool removeTimer(int row);

	Timer getTimer(int row) const;
	void setTimer(int row, const Timer& timer);

	bool startTimer(int row);
	bool stopTimer(int row);
	bool isTimerStarted(int row);

	void reset();
	void resetCount();

	bool load(const QString& filename);
	bool save(const QString& filename);

	QString getFilename() const;

	TimerModel* clone(QObject* parent = nullptr) const;

signals:
	void timerFinished(int row);

public slots:
	void onTimeout();

private:
	QVector<Timer> m_timers;

	QString m_filename;
};

#endif
