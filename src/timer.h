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

#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
	Timer():delayHours(0), delayMinutes(0), delaySeconds(0), restDelay(0), timer(nullptr)
	{
	}

	Timer(const Timer& other)
	{
		set(other);
	}

	QString name;

	int delayHours;
	int delayMinutes;
	int delaySeconds;

	int restDelay;

	QTimer *timer;

	void set(const Timer& other)
	{
		name = other.name;
		delayHours = other.delayHours;
		delayMinutes = other.delayMinutes;
		delaySeconds = other.delaySeconds;
	}
};

QDataStream& operator << (QDataStream& stream, const Timer& timer);
QDataStream& operator >> (QDataStream& stream, Timer& timer);

#endif
