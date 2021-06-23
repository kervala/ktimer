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

int toTimestamp(int h, int m, int s)
{
	return s + 60 * (m + (h * 60));
}

bool fromTimeStamp(int time, int* h, int* m, int* s)
{
	if (s) *s = time % 60;

	time /= 60;

	if (m) *m = time % 60;

	time /= 60;

	if (h) *h = time;

	return true;
}

Timer::Timer():type(Type::Timer), currentDelayHours(0), currentDelayMinutes(0), currentDelaySeconds(0), defaultDelayHours(0), defaultDelayMinutes(0), defaultDelaySeconds(0),
	color(Qt::color0), restDelay(0), timer(nullptr), timerRunning(false)
{
}

Timer::Timer(const Timer& other):name(other.name), type(other.type),
	currentDelayHours(other.currentDelayHours), currentDelayMinutes(other.currentDelayMinutes), currentDelaySeconds(other.currentDelaySeconds),
	defaultDelayHours(other.defaultDelayHours), defaultDelayMinutes(other.defaultDelayMinutes), defaultDelaySeconds(other.defaultDelaySeconds),
	color(other.color), restDelay(other.restDelay), timer(other.timer), timerRunning(other.timerRunning)
{
}

QString Timer::getDelayString() const
{
	return QString("%1:%2:%3").arg(delayHours, 2, 10, QChar('0')).arg(delayMinutes, 2, 10, QChar('0')).arg(delaySeconds, 2, 10, QChar('0'));
}

QString Timer::getRestString() const
{
	int h, m, s;

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

	return QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

void Timer::updateRestDelay()
{
	restDelay = toTimestamp(delayHours, delayMinutes, delaySeconds);
}

QDataStream& operator << (QDataStream& stream, const Timer &timer)
{
	stream << timer.name << timer.defaultDelayHours << timer.defaultDelayMinutes << timer.defaultDelaySeconds << timer.color << timer.type;

	return stream;
}

QDataStream& operator >> (QDataStream& stream, Timer& timer)
{
	stream >> timer.name >> timer.defaultDelayHours >> timer.defaultDelayMinutes >> timer.defaultDelaySeconds >> timer.color;

	if (stream.device()->property("version") == 2)
	{
		stream >> timer.type;
	}

	// update rest delay
	timer.updateRestDelay();

	return stream;
}
