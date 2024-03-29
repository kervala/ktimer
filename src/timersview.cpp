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
#include "timersview.h"

TimersView::TimersView(QWidget *parent):QListView(parent)
{
}

QSize TimersView::sizeHint() const
{
	if (model()->rowCount() == 0) return QSize(0, 0);

	//qDebug() << "sizeHint view" << sizeHintForColumn(0);

	return QSize(sizeHintForColumn(0), visibleItemsCount() * sizeHintForRow(0));
}

QSize TimersView::minimumSizeHint() const
{
	return QSize(0, 0);
}

int TimersView::visibleItemsCount() const
{
	int count = 0;

	for (int row = 0, nrow = model()->rowCount(); row < nrow; ++row)
	{
		if (!isRowHidden(row))
		{
			++count;
		}
	}

	return count;
}
