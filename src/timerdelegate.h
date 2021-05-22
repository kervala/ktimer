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

#ifndef TIMERDELEGATE_H
#define TIMERDELEGATE_H

class TimerDelegate : public QAbstractItemDelegate
{
public:
	TimerDelegate(QObject* parent);

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	void drawString(const QString& s, const QStyleOptionViewItem& options, QPainter& p) const;
	void drawDigit(const QPoint& pos, const QStyleOptionViewItem& options, QPainter& p, int segLen, char newCh, char oldCh) const;
	void drawSegment(const QPoint& pos, char segmentNo, const QStyleOptionViewItem& options, QPainter& p, int segLen, bool erase) const;

	int ndigits;
	bool smallPoint;
	bool shadow;
};

#endif
