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

#ifndef TIMER_H
#define TIMER_H

int toTimestamp(int h, int m, int s);
bool fromTimeStamp(int time, int* h, int* m, int* s);

class Timer
{
public:
	enum class Type
	{
		Timer,
		Alarm
	};

	Timer();
	Timer(const Timer& other);

	QString name;
	Type type;

	int currentDelayHours;
	int currentDelayMinutes;
	int currentDelaySeconds;

	int defaultDelayHours;
	int defaultDelayMinutes;
	int defaultDelaySeconds;

	QColor color;

	int restDelay;

	QTimer *timer;

	bool timerRunning;

	QString getDelayString() const;
	QString getRestString() const;

	void decreaseRestDelay();
	void updateRestDelay();
	void updateCurrentDelay();
};

QDataStream& operator << (QDataStream& stream, const Timer& timer);
QDataStream& operator >> (QDataStream& stream, Timer& timer);

#endif
