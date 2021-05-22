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

quint32 s_version = 1;

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
	// name, type, original position, delay, duration, last position, count
	return TimerColumnLast;
}

QVariant TimerModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch (index.column())
		{
			case TimerColumnName: return m_timers[index.row()].name;
			case TimerColumnDelayHours: return m_timers[index.row()].delayHours;
			case TimerColumnDelayMinutes: return m_timers[index.row()].delayMinutes;
			case TimerColumnDelaySeconds: return m_timers[index.row()].delaySeconds;
		}
	}
	
	return QVariant();
}

bool TimerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
		if (!checkIndex(index)) return false;
#endif

		// save value
		switch (index.column())
		{
			case TimerColumnName: m_timers[index.row()].name = value.toString(); break;
			case TimerColumnDelayHours: m_timers[index.row()].delayHours = value.toInt(); break;
			case TimerColumnDelayMinutes: m_timers[index.row()].delayMinutes = value.toInt(); break;
			case TimerColumnDelaySeconds: m_timers[index.row()].delaySeconds = value.toInt(); break;
			default: return false;
		}

		return true;

	}

	return false;
}

Qt::ItemFlags TimerModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = Qt::ItemIsDropEnabled | QAbstractTableModel::flags(index);

	if (index.isValid()) flags |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled;

	return flags;
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

Qt::DropActions TimerModel::supportedDropActions() const
{
	return Qt::MoveAction | Qt::CopyAction;
}

QStringList TimerModel::mimeTypes() const
{
	QStringList types;
	types << "application/x-autoclicker";
	return types;
}

QMimeData* TimerModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData* mimeData = new QMimeData();
	QByteArray encodedData;

	QDataStream stream(&encodedData, QIODevice::WriteOnly);

	foreach(const QModelIndex &index, indexes)
	{
		if (index.isValid())
		{
			stream << m_timers[index.row()];
		}
	}

	mimeData->setData("application/x-autoclicker", encodedData);

	return mimeData;
}

bool TimerModel::dropMimeData(const QMimeData* data, Qt::DropAction timerType, int row, int column, const QModelIndex& parent)
{
	if (!data->hasFormat("application/x-autoclicker")) return false;

	if (timerType == Qt::IgnoreAction) return true;

	bool insertAtTheEnd = row == -1;

	if (row == -1) row = rowCount();

	QByteArray encodedData = data->data("application/x-autoclicker");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	Timer timer;

	stream >> timer;

	beginInsertRows(QModelIndex(), row, row);

	if (insertAtTheEnd)
	{
		m_timers.push_back(timer);
	}
	else
	{
		m_timers.insert(row, timer);
	}

	endInsertRows();

	return true;
}

Timer TimerModel::getTimer(int row) const
{
	return m_timers[row];
}

void TimerModel::setTimer(int row, const Timer& timer)
{
	m_timers[row] = timer;

	emit dataChanged(index(row, 0), index(row, TimerColumnLast-1), { Qt::DisplayRole, Qt::EditRole });
}

void TimerModel::reset()
{
	m_filename.clear();

	beginResetModel();

	m_timers.clear();

	endResetModel();
}

void TimerModel::resetCount()
{
	for (int i = 0; i < m_timers.size(); ++i)
	{
		Timer& timer = m_timers[i];
	}
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

bool TimerModel::addTimer(const Timer& timer)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());

	m_timers.append(timer);

	endInsertRows();
	return true;
}

bool TimerModel::removeTimer(int i)
{
	beginRemoveRows(QModelIndex(), i, i);

	m_timers.removeAt(i);

	endRemoveRows();
	return true;
}

QString TimerModel::getFilename() const
{
	return m_filename;
}

TimerModel* TimerModel::clone(QObject *parent) const
{
	TimerModel* res = new TimerModel(parent);

	res->m_timers = m_timers;
	res->m_filename = m_filename;

	return res;
}
