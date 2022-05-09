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
#include "timer.h"

int toTimestamp(const QTime& time)
{
	return QTime(0, 0).secsTo(time);
}

int toTimestamp(int h, int m, int s)
{
	return s + 60 * (m + (h * 60));
}

bool fromTimeStamp(int timestamp, QTime& time)
{
	time.setHMS(0, 0, 0);

	if (timestamp < 1)
	{
		return false;
	}

	time = time.addSecs(timestamp);

	return true;
}

bool fromTimeStamp(int time, int* h, int* m, int* s)
{
	if (time < 0)
	{
		if (s) *s = 0;
		if (m) *m = 0;
		if (h) *h = 0;

		return false;
	}

	if (s) *s = time % 60;

	time /= 60;

	if (m) *m = time % 60;

	time /= 60;

	if (h) *h = time;

	return true;
}

bool isNull(const QTime& time)
{
	return time.isNull() || (time.hour() == 0 && time.minute() == 0 && time.second() == 0);
}

Timer::Timer():type(Type::Timer), currentDelay(0, 0), defaultDelay(0, 0),
	color(Qt::color0), timer(nullptr), timerRunning(false), notificationSent(false)
{
}

Timer::Timer(const Timer& other):name(other.name), type(other.type),
	currentAbsoluteTime(other.currentAbsoluteTime),
	currentDelay(other.currentDelay), defaultDelay(other.defaultDelay),
	color(other.color), timer(other.timer), timerRunning(other.timerRunning)
{
}

int Timer::getCurrentDelay() const
{
	if (type == Type::Alarm)
	{
		return QTime::currentTime().secsTo(currentDelay);
	}

	return toTimestamp(currentDelay);
}

QString Timer::getCurrentDelayString() const
{
	return QString("%1:%2:%3")
		.arg(currentDelay.hour(), 2, 10, QChar('0'))
		.arg(currentDelay.minute(), 2, 10, QChar('0'))
		.arg(currentDelay.second(), 2, 10, QChar('0'));
}

int Timer::getRestDelay() const
{
	if (currentAbsoluteTime.isNull())
	{
		return getCurrentDelay();
	}

	return QDateTime::currentDateTime().secsTo(currentAbsoluteTime);
}

QString Timer::getRestDelayString() const
{
	int h, m, s;

	int restDelay = getRestDelay();

	if (restDelay > -1)
	{
		if (!fromTimeStamp(restDelay, &h, &m, &s)) return QString("##:##:##");
	}
	else
	{
		h = 0;
		m = 0;
		s = 0;
	}

	return QString("%1:%2:%3")
		.arg(h, 2, 10, QChar('0'))
		.arg(m, 2, 10, QChar('0'))
		.arg(s, 2, 10, QChar('0'));
}

void Timer::updateAbsoluteTime()
{
	currentAbsoluteTime = QDateTime::currentDateTime().addSecs(getCurrentDelay());
}

QDataStream& operator << (QDataStream& stream, const Timer &timer)
{
	stream << timer.name << timer.defaultDelay.hour() << timer.defaultDelay.minute() << timer.defaultDelay.second()
		<< timer.color << timer.type;

	if (stream.device()->property("resume").toBool())
	{
		if (timer.currentAbsoluteTime.isNull())
		{
			int d = 0;

			stream << d << d << d;
		}
		else
		{
			stream << timer.currentAbsoluteTime.time().hour()
				<< timer.currentAbsoluteTime.time().minute()
				<< timer.currentAbsoluteTime.time().second();
		}
	}
	else
	{
		int d = 0;

		stream << d << d << d;
	}

	return stream;
}

static void setHMS(QDataStream& stream, QDateTime& time)
{
	int h, m, s;

	stream >> h >> m >> s;

	if (h == 0 && m == 0 && s == 0)
	{
		time = QDateTime(); // invalid
	}
	else
	{
		// today
		time = QDateTime::currentDateTime();

		// only change hour, minute and second
		time.time().setHMS(h, m, s);
	}
}

static void setHMS(QDataStream& stream, QTime& time)
{
	int h, m, s;

	stream >> h >> m >> s;

	if (h == 0 && m == 0 && s == 0)
	{
		time = QTime(); // invalid
	}
	else
	{
		time.setHMS(h, m, s);
	}
}

QDataStream& operator >> (QDataStream& stream, Timer& timer)
{
	stream >> timer.name;
	
	setHMS(stream, timer.defaultDelay);
	
	stream >> timer.color;

	if (stream.device()->property("version").toInt() >= 2)
	{
		stream >> timer.type;

		if (stream.device()->property("version").toInt() >= 3)
		{
			setHMS(stream, timer.currentAbsoluteTime);
		}
	}

	return stream;
}
