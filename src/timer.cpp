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

Timer::Timer():delayHours(0), delayMinutes(0), delaySeconds(0), restDelay(0), timer(nullptr)
{
}

Timer::Timer(const Timer& other):restDelay(0), timer(nullptr)
{
	set(other, true);
}

void Timer::set(const Timer& other, bool updateRest)
{
	name = other.name;
	delayHours = other.delayHours;
	delayMinutes = other.delayMinutes;
	delaySeconds = other.delaySeconds;
	color = other.color;

	if (updateRest)
	{
		restDelay = other.restDelay;
	}
}

QString Timer::getDelayString() const
{
	//return QString("%1:%2:%3").arg(delayHours, 2, 10, QChar('0')).arg(delayMinutes, 2, 10, QChar('0')).arg(delaySeconds, 2, 10, QChar('0'));

	int h, m, s;

	if (!fromTimeStamp(restDelay, &h, &m, &s)) return QString("##:##:##");

	return QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

QDataStream& operator << (QDataStream& stream, const Timer &timer)
{
	stream << timer.name << timer.delayHours << timer.delayMinutes << timer.delaySeconds << timer.color;

	return stream;
}

QDataStream& operator >> (QDataStream& stream, Timer& timer)
{
	stream >> timer.name >> timer.delayHours << timer.delayMinutes << timer.delaySeconds << timer.color;

	return stream;
}
