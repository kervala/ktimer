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
#include "mainwindow.h"
#include "moc_mainwindow.cpp"
#include "ui_mainwindow.h"
#include "configfile.h"
#include "timermodel.h"
#include "timerdelegate.h"
#include "updatedialog.h"
#include "updater.h"
#include "utils.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

MainWindow::MainWindow() : QMainWindow(nullptr, Qt::WindowCloseButtonHint), m_selectedTimer(-1), m_model(nullptr), m_menu(nullptr),
m_resetAction(nullptr), m_startAction(nullptr), m_stopAction(nullptr)
{
	m_ui = new Ui::MainWindow();
	m_ui->setupUi(this);

	QSize size = ConfigFile::getInstance()->getWindowSize();
	if (!size.isNull()) resize(size);

	QPoint pos = ConfigFile::getInstance()->getWindowPosition();
	if (!pos.isNull()) move(pos);

	SystrayIcon *systray = new SystrayIcon(this);

	m_model = new TimerModel(this);
	m_ui->timersListView->setModel(m_model);
	m_ui->timersListView->setItemDelegate(new TimerDelegate(this));

	connect(m_model, &TimerModel::timerFinished, this, &MainWindow::onTimerFinished);

	// Selection model
	connect(m_ui->timersListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onTimerSelected);
	connect(m_ui->timersListView, &QListView::doubleClicked, this, &MainWindow::onTimerDoubleClicked);

	// check for a new version
	m_updater = new Updater(this);

	// File menu
	connect(m_ui->actionNew, &QAction::triggered, this, &MainWindow::onNew);
	connect(m_ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpen);
	connect(m_ui->actionSave, &QAction::triggered, this, &MainWindow::onSave);
	connect(m_ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveAs);
	connect(m_ui->actionExit, &QAction::triggered, this, &MainWindow::close);

	// Help menu
	connect(m_ui->actionCheckUpdates, &QAction::triggered, this, &MainWindow::onCheckUpdates);
	connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
	connect(m_ui->actionAboutQt, &QAction::triggered, this, &MainWindow::onAboutQt);

	// Buttons
	connect(m_ui->colorButton, &QPushButton::clicked, this, &MainWindow::onColorClicked);
	connect(m_ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddClicked);
	connect(m_ui->removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveClicked);

	connect(m_ui->nameEdit, &QLineEdit::textChanged, this, &MainWindow::onNameChanged);

	connect(m_ui->detailsCheckBox, &QCheckBox::toggled, this, &MainWindow::onDetailsToggled);
	connect(m_ui->activeCheckBox, &QCheckBox::toggled, this, &MainWindow::onActiveToggled);

	connect(m_ui->actionShowDetails, &QAction::toggled, this, &MainWindow::onDetailsToggled);
	connect(m_ui->actionShowOnlyActiveTimers, &QAction::toggled, this, &MainWindow::onActiveToggled);
	connect(m_ui->actionAlwaysOnTop, &QAction::toggled, this, &MainWindow::onTopToggled);

	// Delay

	// current
	connect(m_ui->currentHoursSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);
	connect(m_ui->currentMinutesSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);
	connect(m_ui->currentSecondsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);

	// default
	connect(m_ui->defaultHoursSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);
	connect(m_ui->defaultMinutesSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);
	connect(m_ui->defaultSecondsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);

	connect(m_ui->absoluteTimeCheckBox, &QCheckBox::toggled, this, &MainWindow::onAbsoluteTimeChecked);

	// Systray
	connect(systray, &SystrayIcon::requestMinimize, this, &MainWindow::onMinimize);
	connect(systray, &SystrayIcon::requestRestore, this, &MainWindow::onRestore);
	connect(systray, &SystrayIcon::requestClose, this, &MainWindow::close);
	connect(systray, &SystrayIcon::requestAction, this, &MainWindow::onSystrayAction);

	// Updater
	connect(m_updater, &Updater::newVersionDetected, this, &MainWindow::onNewVersion);
	connect(m_updater, &Updater::noNewVersionDetected, this, &MainWindow::onNoNewVersion);

	m_updater->checkUpdates(true);

	m_ui->timersListView->setSizeAdjustPolicy(QListView::AdjustToContents);

	createMenu();

	const QString filename = ConfigFile::getInstance()->getLocalDataDirectory() + "/autosave.ktf";

	if (m_model->load(filename))
	{
		m_ui->timersListView->selectionModel()->setCurrentIndex(m_model->index(m_model->rowCount() - 1, 0), QItemSelectionModel::ClearAndSelect);

		updateGeometry();
	}
	else
	{
		// create a default timer
		onNew();
	}
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::showEvent(QShowEvent *e)
{
	onTopToggled(m_ui->actionAlwaysOnTop->isChecked());

	e->accept();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	const QString filename = ConfigFile::getInstance()->getLocalDataDirectory() + "/autosave.ktf";

	// save current timers
	m_model->save(filename, true);

	hide();

	e->accept();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
	ConfigFile::getInstance()->setWindowSize(e->size());

	e->accept();
}

void MainWindow::moveEvent(QMoveEvent *e)
{
	ConfigFile::getInstance()->setWindowPosition(QPoint(x(), y()));

	e->accept();
}

void MainWindow::onChangeSystrayIcon()
{
	SystrayIcon::Status status = SystrayIcon::getInstance()->getStatus() == SystrayIcon::Status::Click ? SystrayIcon::Status::Normal : SystrayIcon::Status::Click;

	SystrayIcon::getInstance()->setStatus(status);
}

void MainWindow::onNew()
{
	m_model->reset();

	onAddClicked();
}

void MainWindow::onOpen()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open timers"), ConfigFile::getInstance()->getLocalDataDirectory(), tr("kTimer Files (*.ktf)"));

	if (filename.isEmpty()) return;

	if (m_model->load(filename))
	{
		m_ui->timersListView->selectionModel()->setCurrentIndex(m_model->index(m_model->rowCount() - 1, 0), QItemSelectionModel::ClearAndSelect);

		updateGeometry();
	}
}

void MainWindow::onSave()
{
	m_model->save(m_model->getFilename());
}

void MainWindow::onSaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save timers"), m_model->getFilename().isEmpty() ? ConfigFile::getInstance()->getLocalDataDirectory():m_model->getFilename(), tr("kTimer Files (*.ktf)"));

	if (filename.isEmpty()) return;

	m_model->save(filename);
}

void MainWindow::onColorClicked()
{
	if (m_selectedTimer < 0) return;

	Timer& timer = m_model->getTimer(m_selectedTimer);

	QColor color = QColorDialog::getColor(timer.color, this, tr("LCD color"));

	if (!color.isValid()) return;

	timer.color = color;

	m_model->updateTimer(m_selectedTimer);

	updateButtons();
}

void MainWindow::onAddClicked()
{
	m_model->newTimer();

	m_ui->timersListView->selectionModel()->setCurrentIndex(m_model->index(m_model->rowCount() - 1, 0), QItemSelectionModel::ClearAndSelect);

	updateGeometry();
}

void MainWindow::onRemoveClicked()
{
	if (m_selectedTimer < 0) return;

	m_model->removeTimer(m_selectedTimer);

	updateButtons();

	updateGeometry();
}

void MainWindow::onStartClicked()
{
	if (m_selectedTimer < 0) return;

	m_model->startTimer(m_selectedTimer);

	updateButtons();
}

void MainWindow::onStopClicked()
{
	if (m_selectedTimer < 0) return;

	m_model->stopTimer(m_selectedTimer);

	updateButtons();
}

void MainWindow::onResetClicked()
{
	if (m_selectedTimer < 0) return;

	m_model->resetTimer(m_selectedTimer);

	updateButtons();
}

int MainWindow::currentDelayToTimestamp()
{
	return ::toTimestamp(m_ui->currentHoursSpinBox->value(), m_ui->currentMinutesSpinBox->value(), m_ui->currentSecondsSpinBox->value());
}

bool MainWindow::currentDelayFromTimeStamp(int time)
{
	int h, m, s;

	if (!::fromTimeStamp(time, &h, &m, &s)) return false;

	m_ui->currentHoursSpinBox->setValue(h);
	m_ui->currentMinutesSpinBox->setValue(m);
	m_ui->currentSecondsSpinBox->setValue(s);

	return true;
}

void MainWindow::onNameChanged(const QString& name)
{
	if (m_selectedTimer < 0) return;

	Timer& timer = m_model->getTimer(m_selectedTimer);

	timer.name = name;

	m_model->updateTimer(m_selectedTimer);

	updateButtons();
}

void MainWindow::updateLayout()
{
	QTimer::singleShot(100, [this]()
	{
		resize(minimumSizeHint());
		adjustSize();
	});
}

void MainWindow::onDetailsToggled(bool details)
{
	m_ui->detailsGroupBox->setHidden(!details);

	updateLayout();
}

void MainWindow::onActiveToggled(bool active)
{
	for (int row = 0; row < m_model->rowCount(); ++row)
	{
		const Timer& timer = m_model->getTimer(row);

		m_ui->timersListView->setRowHidden(row, !timer.timerRunning && active);
	}

	updateLayout();
}

void MainWindow::onTopToggled(bool top)
{
#ifdef Q_OS_WIN
	if (top)
	{
		SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos((HWND)winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
#else
	Qt::WindowFlags flags = windowFlags();

	if (top)
	{
		setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
	}
	else
	{
		setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
	}

	showNormal();
#endif

}

static void setHMS(QObject* sender, Ui::MainWindow *ui, QTime& time, int delay)
{
	if (sender == ui->currentHoursSpinBox)
	{
		time.setHMS(delay, time.minute(), time.second());
	}
	else if (sender == ui->currentMinutesSpinBox)
	{
		time.setHMS(time.hour(), delay, time.second());
	}
	else if (sender == ui->currentSecondsSpinBox)
	{
		time.setHMS(time.hour(), time.minute(), delay);
	}
	else if (sender == ui->defaultHoursSpinBox)
	{
		time.setHMS(delay, time.minute(), time.second());
	}
	else if (sender == ui->defaultMinutesSpinBox)
	{
		time.setHMS(time.hour(), delay, time.second());
	}
	else if (sender == ui->defaultSecondsSpinBox)
	{
		time.setHMS(time.hour(), time.minute(), delay);
	}
}

void MainWindow::onDelayChanged(int delay)
{
	if (m_selectedTimer < 0 || m_model->isTimerRunning(m_selectedTimer)) return;

	Timer& timer = m_model->getTimer(m_selectedTimer);

	setHMS(sender(), m_ui, timer.currentDelay, delay);

	m_model->updateTimer(m_selectedTimer);
}

void MainWindow::onAbsoluteTimeChecked(bool checked)
{
	if (m_selectedTimer < 0 || m_model->isTimerRunning(m_selectedTimer)) return;

	Timer& timer = m_model->getTimer(m_selectedTimer);

	timer.type = m_ui->absoluteTimeCheckBox->isChecked() ? Timer::Type::Alarm : Timer::Type::Timer;
}

void MainWindow::onTimerFinished(int row)
{
	const Timer& timer = m_model->getTimer(row);

	SystrayIcon::getInstance()->displayMessage(tr("kTimer notification"),
		tr("End of timer %1 after %2. You can restart it if you need.").arg(timer.name.isEmpty() ? tr("Unknown #%1").arg(row): timer.name).arg(timer.getCurrentDelayString()),
		SystrayIcon::Action::None);
}

void MainWindow::onTimerSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
	if (selected.isEmpty())
	{
		m_selectedTimer = -1;
	}
	else
	{
		m_selectedTimer = selected.indexes().front().row();

		displayTimer(m_selectedTimer);
	}

	updateButtons();
}

void MainWindow::onTimerDoubleClicked(const QModelIndex& item)
{
	if (m_model->isTimerRunning(item.row()))
	{
		m_model->stopTimer(item.row());

		updateButtons();
	}
	else
	{
		m_model->startTimer(item.row());

		updateButtons();
	}
}

void MainWindow::displayTimer(int i)
{
	if (i >= m_model->rowCount()) return;

	const Timer& timer = m_model->getTimer(i);

	m_ui->nameEdit->setText(timer.name);

	m_ui->absoluteTimeCheckBox->setChecked(timer.type == Timer::Type::Alarm);

	m_ui->currentHoursSpinBox->setValue(timer.currentDelay.hour());
	m_ui->currentMinutesSpinBox->setValue(timer.currentDelay.minute());
	m_ui->currentSecondsSpinBox->setValue(timer.currentDelay.second());

	m_ui->defaultHoursSpinBox->setValue(timer.defaultDelay.hour());
	m_ui->defaultMinutesSpinBox->setValue(timer.defaultDelay.minute());
	m_ui->defaultSecondsSpinBox->setValue(timer.defaultDelay.second());
}

void MainWindow::updateButtons()
{
	bool timerRunning = m_selectedTimer > -1 && m_model->isTimerRunning(m_selectedTimer);

	const Timer& timer = m_model->getTimer(m_selectedTimer);

	QPixmap pixmap(16, 16);
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);
	painter.setBrush(timer.color);
	painter.drawRect(QRect(0, 2, 10, 12));

	QIcon icon(pixmap);
	m_ui->colorButton->setIcon(icon);

	m_ui->currentHoursSpinBox->setEnabled(!timerRunning);
	m_ui->currentMinutesSpinBox->setEnabled(!timerRunning);
	m_ui->currentSecondsSpinBox->setEnabled(!timerRunning);

	m_ui->defaultHoursSpinBox->setEnabled(!timerRunning);
	m_ui->defaultMinutesSpinBox->setEnabled(!timerRunning);
	m_ui->defaultSecondsSpinBox->setEnabled(!timerRunning);

	m_ui->addButton->setEnabled(true);
	m_ui->removeButton->setEnabled(m_selectedTimer > -1 && m_model->rowCount() > 1);

	m_resetAction->setEnabled(timerRunning);
	m_startAction->setEnabled(!timerRunning);
	m_stopAction->setEnabled(timerRunning);
}

void MainWindow::createMenu()
{
	m_resetAction = new QAction(tr("&Reset"), this);
	m_resetAction->setStatusTip(tr("Reset timer"));
	connect(m_resetAction, &QAction::triggered, this, &MainWindow::onResetClicked);

	m_startAction = new QAction(tr("Start"), this);
	m_startAction->setStatusTip(tr("Start timer"));
	connect(m_startAction, &QAction::triggered, this, &MainWindow::onStartClicked);

	m_stopAction = new QAction(tr("Stop"), this);
	m_stopAction->setStatusTip(tr("Stop timer"));
	connect(m_stopAction, &QAction::triggered, this, &MainWindow::onStopClicked);

	m_menu = new QMenu(this);
	m_menu->addAction(m_resetAction);
	m_menu->addAction(m_startAction);
	m_menu->addAction(m_stopAction);
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
	if (m_menu)
	{
		m_menu->exec(event->globalPos());
	}
}

void MainWindow::onCheckUpdates()
{
	m_updater->checkUpdates(false);
}

void MainWindow::onAbout()
{
	QMessageBox::about(this,
		tr("About %1").arg(QApplication::applicationName()),
		QString("%1 %2<br>").arg(QApplication::applicationName()).arg(QApplication::applicationVersion())+
		tr("Simple timers manager")+
		QString("<br><br>")+
		tr("Author: %1").arg("<a href=\"http://kervala.deviantart.com\">Kervala</a><br>")+
		tr("Support: %1").arg("<a href=\"http://dev.kervala.net/projects/ktimer\">http://dev.kervala.net/projects/ktimer</a>"));
}

void MainWindow::onAboutQt()
{
	QMessageBox::aboutQt(this);
}

void MainWindow::onMinimize()
{
	// only hide window if using systray and enabled hide minized window
	if (isVisible())
	{
		hide();
	}
}

void MainWindow::onRestore()
{
	if (!isVisible())
	{
		showNormal();
	}

	raise();
	activateWindow();
}

void MainWindow::onSystrayAction(SystrayIcon::Action action)
{
	switch(action)
	{
		case SystrayIcon::Action::Update:
		break;

		default:
		break;
	}
}

bool MainWindow::event(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		m_ui->retranslateUi(this);
	}
	else if (e->type() == QEvent::WindowStateChange)
	{
		if (windowState() & Qt::WindowMinimized)
		{
			QTimer::singleShot(250, this, SLOT(onMinimize()));
		}
	}

	return QMainWindow::event(e);
}

void MainWindow::onNewVersion(const QString &url, const QString &date, uint size, const QString &version)
{
	QMessageBox::StandardButton reply = QMessageBox::question(this,
		tr("New version"),
		tr("Version %1 is available since %2.\n\nDo you want to download it now?").arg(version).arg(date),
		QMessageBox::Yes|QMessageBox::No);

	if (reply != QMessageBox::Yes) return;

	UpdateDialog dialog(this);

	connect(&dialog, &UpdateDialog::downloadProgress, this, &MainWindow::onProgress);

	dialog.download(url, size);

	if (dialog.exec() == QDialog::Accepted)
	{
		// if user clicked on Install, close kdAmn
		close();
	}
}

void MainWindow::onNoNewVersion()
{
	QMessageBox::information(this,
		tr("No update found"),
		tr("You already have the last %1 version (%2).").arg(QApplication::applicationName()).arg(QApplication::applicationVersion()));
}

void MainWindow::onProgress(qint64 readBytes, qint64 totalBytes)
{
	// TODO: for other OSes
}
