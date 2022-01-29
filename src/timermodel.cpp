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

#include "common.h"
#include "timermodel.h"

struct SMagicHeader
{
	union
	{
		char str[5];
		quint32 num;
	};
};

SMagicHeader s_header = { "TIFK" };

// version 1:
// - initial version
// 
// version 2:
// - added "type" variable

quint32 s_version = 2;

TimerModel::TimerModel(QObject* parent) : QAbstractTableModel(parent)
{
}

TimerModel::~TimerModel()
{
}

int TimerModel::rowCount(const QModelIndex &parent) const
{
	return m_timers.size();
}

int TimerModel::columnCount(const QModelIndex &parent) const
{
	return 1;
}

QVariant TimerModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	const Timer& timer = m_timers[index.row()];

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return timer.getRestString();
	}

	if (role == Qt::ToolTipRole)
	{
		return timer.name;
	}

	if (role == Qt::ForegroundRole)
	{
		if (timer.timerRunning && timer.restDelay < 0 && (timer.restDelay % 2 == 0))
		{
			return QColor(Qt::color1);
		}

		return timer.color;
	}

	return QVariant();
}

bool TimerModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	bool insertAtTheEnd = position == -1;

	if (position == -1) position = rowCount();

	beginInsertRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
	{
		Timer timer;
		timer.name = tr("Timer #%1").arg(rowCount() + 1);

		if (insertAtTheEnd)
		{
			m_timers.push_back(timer);
		}
		else
		{
			m_timers.insert(row + position, timer);
		}
	}

	endInsertRows();
	return true;
}

bool TimerModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
	{
		m_timers.removeAt(position + row);
	}

	endRemoveRows();
	return true;
}

Timer& TimerModel::getTimer(int row)
{
	return m_timers[row];
}

bool TimerModel::startTimer(int row)
{
	Timer& timer = m_timers[row];

	if (timer.timer) return false;

	if (timer.currentDelayHours == 0 && timer.currentDelayMinutes == 0 && timer.currentDelaySeconds == 0)
	{
		if (timer.type == Timer::Type::Alarm)
		{
			// compute delay
			QTime current = QTime::currentTime();

			int delay = toTimestamp(timer.defaultDelayHours, timer.defaultDelayMinutes, timer.defaultDelaySeconds) - toTimestamp(current.hour(), current.minute(), current.second());

			// next day
			if (delay < 0)
			{
				delay += 24 * 3600; // 24 hours
			}

			fromTimeStamp(delay, &timer.currentDelayHours, &timer.currentDelayMinutes, &timer.currentDelaySeconds);
		}
		else
		{
			timer.currentDelayHours = timer.defaultDelayHours;
			timer.currentDelayMinutes = timer.defaultDelayMinutes;
			timer.currentDelaySeconds = timer.defaultDelaySeconds;
		}
	}

	timer.restDelay = toTimestamp(timer.currentDelayHours, timer.currentDelayMinutes, timer.currentDelaySeconds);
	timer.timerRunning = true;

	timer.timer = new QTimer(this);
	timer.timer->setInterval(1000);
	timer.timer->start();
	timer.timer->setProperty("row", row);

	emit dataChanged(index(row, 0), index(row, 0), { Qt::DisplayRole });

	connect(timer.timer, &QTimer::timeout, this, &TimerModel::onTimeout);

	return true;
}

bool TimerModel::stopTimer(int row)
{
	Timer& timer = m_timers[row];

	timer.timerRunning = false;

	if (!timer.timer) return false;

	timer.timer->stop();
	timer.timer->deleteLater();
	timer.timer = nullptr;

	return true;
}

bool TimerModel::isTimerRunning(int row)
{
	if (row < 0 || row >= m_timers.size()) return false;

	return m_timers[row].timerRunning;
}

void TimerModel::onTimeout()
{
	QObject* t = sender();

	if (!t || !t->property("row").isValid()) return;

	int row = t->property("row").toInt();

	Timer& timer = m_timers[row];

	timer.decreaseRestDelay();
	timer.updateCurrentDelay();

	emit dataChanged(index(row, 0), index(row, 0), { Qt::DisplayRole });

	if (timer.restDelay == 0)
	{
		emit timerFinished(row);
	}
}

void TimerModel::reset()
{
	m_filename.clear();

	beginResetModel();

	m_timers.clear();

	endResetModel();
}

bool TimerModel::load(const QString& filename)
{
	if (filename.isEmpty()) return false;

	QFile file(filename);

	if (!file.open(QIODevice::ReadOnly)) return false;

	QDataStream stream(&file);

	// Read and check the header
	SMagicHeader header;

	stream >> header.num;

	if (header.num != s_header.num) return false;

	// Read the version
	quint32 version;
	stream >> version;

	if (version > s_version) return false;

	// define version for items and other serialized objects
	stream.device()->setProperty("version", version);

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
	stream.setVersion(QDataStream::Qt_5_4);
#else
	stream.setVersion(QDataStream::Qt_5_6);
#endif

	beginResetModel();

	// timers
	stream >> m_timers;

	endResetModel();

	m_filename = filename;

	for (int i = 0, ilen = m_timers.size(); i < ilen; ++i)
	{
		// automatically start alarms
		if (m_timers[i].type == Timer::Type::Alarm)
		{
			startTimer(i);
		}
	}

	return true;
}

bool TimerModel::save(const QString& filename)
{
	if (filename.isEmpty()) return false;

	QFile file(filename);

	if (!file.open(QIODevice::WriteOnly)) return false;

	QDataStream stream(&file);

	// Write a header with a "magic number" and a version
	stream << s_header.num;
	stream << s_version;

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
	stream.setVersion(QDataStream::Qt_5_4);
#else
	stream.setVersion(QDataStream::Qt_5_6);
#endif

	stream << m_timers;

	m_filename = filename;

	return true;
}

bool TimerModel::newTimer()
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());

	m_timers.append(Timer());

	endInsertRows();
	return true;
}

bool TimerModel::removeTimer(int i)
{
	beginRemoveRows(QModelIndex(), i, i);

	stopTimer(i);

	m_timers.removeAt(i);

	endRemoveRows();
	return true;
}

void TimerModel::updateTimer(int row)
{
	emit dataChanged(index(row, 0), index(row, 0), { Qt::DisplayRole, Qt::EditRole, Qt::ToolTipRole, Qt::ForegroundRole });
}

QString TimerModel::getFilename() const
{
	return m_filename;
}
