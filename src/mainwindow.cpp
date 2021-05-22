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
#include "mainwindow.h"
#include "moc_mainwindow.cpp"
#include "ui_mainwindow.h"
#include "configfile.h"
#include "timermodel.h"
#include "timerdelegate.h"
#include "updatedialog.h"
#include "updater.h"
#include "utils.h"

#ifdef Q_OS_WIN32
#include <QtWinExtras/QWinTaskbarProgress>
#include <QtWinExtras/QWinTaskbarButton>
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

MainWindow::MainWindow() : QMainWindow(nullptr, Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint), m_selectedTimer(-1), m_model(nullptr)
{
	m_ui = new Ui::MainWindow();
	m_ui->setupUi(this);

#ifdef Q_OS_WIN32
	m_button = new QWinTaskbarButton(this);
#endif

	QSize size = ConfigFile::getInstance()->getWindowSize();
	if (!size.isNull()) resize(size);

	QPoint pos = ConfigFile::getInstance()->getWindowPosition();
	if (!pos.isNull()) move(pos);

	SystrayIcon *systray = new SystrayIcon(this);

	m_model = new TimerModel(this);
	m_ui->timersListView->setModel(m_model);
	m_ui->timersListView->setItemDelegate(new TimerDelegate(this));

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
	connect(m_ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddClicked);
	connect(m_ui->removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveClicked);
	connect(m_ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
	connect(m_ui->stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);

	connect(m_ui->nameEdit, &QLineEdit::textChanged, this, &MainWindow::onNameChanged);

	// Delay
	connect(m_ui->hoursSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);
	connect(m_ui->minutesSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);
	connect(m_ui->secondsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDelayChanged);

	// Systray
	connect(systray, &SystrayIcon::requestMinimize, this, &MainWindow::onMinimize);
	connect(systray, &SystrayIcon::requestRestore, this, &MainWindow::onRestore);
	connect(systray, &SystrayIcon::requestClose, this, &MainWindow::close);
	connect(systray, &SystrayIcon::requestAction, this, &MainWindow::onSystrayAction);

	// Updater
	connect(m_updater, &Updater::newVersionDetected, this, &MainWindow::onNewVersion);
	connect(m_updater, &Updater::noNewVersionDetected, this, &MainWindow::onNoNewVersion);

	m_updater->checkUpdates(true);

	updateButtons();
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::showEvent(QShowEvent *e)
{
#ifdef Q_OS_WIN32
	m_button->setWindow(windowHandle());
#endif

	e->accept();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
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
	SystrayIcon::SystrayStatus status = SystrayIcon::getInstance()->getStatus() == SystrayIcon::StatusClick ? SystrayIcon::StatusNormal : SystrayIcon::StatusClick;

	SystrayIcon::getInstance()->setStatus(status);
}

void MainWindow::onNew()
{
}

void MainWindow::onOpen()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open script"), ConfigFile::getInstance()->getLocalDataDirectory(), "AutoClicker Files (*.acf)");

	if (filename.isEmpty()) return;

	if (m_model->load(filename))
	{
		// QString filename = QFileInfo(m_model->getFilename()).baseName();

		// updateStartButton();
	}
}

void MainWindow::onSave()
{
	m_model->save(m_model->getFilename());
}

void MainWindow::onSaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save actions"), /* ConfigFile::getInstance()->getLocalDataDirectory() */ m_model->getFilename(), "AutoClicker Files (*.acf)");

	if (filename.isEmpty()) return;

	m_model->save(filename);
}

void MainWindow::onAddClicked()
{
	m_model->addTimer(m_currentTimer);
}

void MainWindow::onRemoveClicked()
{
	if (m_selectedTimer < 0) return;

	m_model->removeTimer(m_selectedTimer);
}

void MainWindow::onStartClicked()
{
}

void MainWindow::onStopClicked()
{
}

void MainWindow::updateTimerFromCurrent()
{
	if (m_selectedTimer > -1)
	{
		m_model->setTimer(m_selectedTimer, m_currentTimer);
	}
}

void MainWindow::onNameChanged(const QString& name)
{
	m_currentTimer.name = name;

	updateTimerFromCurrent();
}

void MainWindow::onDelayChanged(int delay)
{
	if (sender() == m_ui->hoursSpinBox)
	{
		m_currentTimer.delayHours = delay;
	}
	else if (sender() == m_ui->minutesSpinBox)
	{
		m_currentTimer.delayMinutes = delay;
	}
	else if (sender() == m_ui->secondsSpinBox)
	{
		m_currentTimer.delaySeconds = delay;
	}

	updateTimerFromCurrent();
}

void MainWindow::displayTimer(int i)
{
	if (i >= m_model->rowCount()) return;

	m_currentTimer = m_model->getTimer(i);

	m_ui->nameEdit->setText(m_currentTimer.name);

	m_ui->hoursSpinBox->setValue(m_currentTimer.delayHours);
	m_ui->minutesSpinBox->setValue(m_currentTimer.delayMinutes);
	m_ui->secondsSpinBox->setValue(m_currentTimer.delaySeconds);
}

void MainWindow::updateButtons()
{
	m_ui->addButton->setEnabled(true);
	m_ui->removeButton->setEnabled(m_selectedTimer > -1);

	m_ui->startButton->setEnabled(m_selectedTimer > -1);
	m_ui->stopButton->setEnabled(m_selectedTimer > -1);
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
		tr("Simple timers notifier")+
		QString("<br><br>")+
		tr("Author: %1").arg("<a href=\"http://kervala.deviantart.com\">Kervala</a><br>")+
		tr("Support: %1").arg("<a href=\"http://dev.kervala.net/projects/simpletimer\">http://dev.kervala.net/projects/simpletimer</a>"));
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

void MainWindow::onSystrayAction(SystrayIcon::SystrayAction action)
{
	switch(action)
	{
		case SystrayIcon::ActionUpdate:
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
#ifdef Q_OS_WIN32
	QWinTaskbarProgress *progress = m_button->progress();

	if (readBytes == totalBytes)
	{
		// end
		progress->hide();
	}
	else if (readBytes == 0)
	{
//		TODO: see why it doesn't work
//		m_button->setOverlayIcon(style()->standardIcon(QStyle::SP_MediaPlay) /* QIcon(":/icons/upload.svg") */);
//		m_button->setOverlayAccessibleDescription(tr("Upload"));

		// beginning
		progress->show();
		progress->setRange(0, totalBytes);
	}
	else
	{
		progress->show();
		progress->setValue(readBytes);
	}
#else
	// TODO: for other OSes
#endif
}
