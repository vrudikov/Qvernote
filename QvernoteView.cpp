/***********************************************************************
Copyright (C) 2010 Alex R <burbulator@gmail.com>

This file is part of Qvernote.

Qvernote is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Qvernote is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Qvernote.  If not, see <http://www.gnu.org/licenses/>.

***********************************************************************/
#include "QvernoteView.h"
#include "QvernoteAPI.h"
#include "QNewNotebookDialog.h"
#include "QNoteEditWindow.h"
#include "QNotebookListItemWidget.h"
#include "QvernoteTypes.h"
#include "QvernoteSettings.h"
#include "QNoteSearchDialog.h"
#include "QDeleteNotebookView.h"
#include "QAboutDialog.h"
#include <QGridLayout>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QDebug>
#include "QvernoteStorage.h"
#include "QTrashWindow.h"
#include "QSyncProgressWidget.h"
#include "QKeyGrabber.h"

#ifdef Q_WS_MAEMO_5
#include <QMaemo5InformationBox>
#include "QOptionsDialog.h"
#endif

using namespace boost;

QvernoteView::QvernoteView(QWidget *parent)
    : QvernoteWindow(parent), m_hEvernote(QvernoteAPI::Instance()), isSearchShown(false), isSynchronizing(false)
{
	QvernoteSettings* settings = QvernoteSettings::Instance();
	ui.setupUi(this);

	ui.gridLayout_2->addWidget(&syncWidget, 2, 0, 1, 2, Qt::AlignCenter);
	ui.actionUse_SSL->setChecked(settings->getUseSsl());
	ui.lvNotebookList->setVisible(false);
	ui.pbSync->setEnabled(false);

	if(!m_hEvernote->isOnline() && settings->getWorkOnline())
	{
		displayInformationBox(trUtf8("Network error occurred, working offline"));
		settings->setWorkOnline(false);
	}

	QObject::connect(ui.lvNotebookList, SIGNAL(itemClicked(QListWidgetItem *)),  this, SLOT(loadNotebook(QListWidgetItem* )));
	QObject::connect(ui.pbNewNote, SIGNAL(clicked()), this, SLOT(openNewNoteWindow()));
	QObject::connect(ui.pbSync, SIGNAL(clicked()), this, SLOT(/*synchronizeNoteStore()*/initView()));
	QObject::connect(ui.actionSearch, SIGNAL(triggered(bool)), this, SLOT(openSearchDialog()));
	QObject::connect(ui.actionDelete, SIGNAL(triggered(bool)), this, SLOT(openDeleteNotebookWindow()));
	QObject::connect(ui.actionNewNotebook, SIGNAL(triggered(bool)), this, SLOT(createNewNotebook()));
	QObject::connect(ui.actionAbout, SIGNAL(triggered(bool)), this, SLOT(openAboutDialog()));
	QObject::connect(ui.actionTrash, SIGNAL(triggered(bool)), this, SLOT(openTrashWindow()));
	QObject::connect(ui.actionOptions, SIGNAL(triggered(bool)), this, SLOT(openOptionsDialog()));
	QObject::connect(ui.actionFavorites, SIGNAL(triggered(bool)), this, SLOT(openFavoritesWindow()));

	QObject::connect(m_hEvernote, SIGNAL(progressUpdated(QString)), &syncWidget, SLOT(updateProgressData(QString)));
	qRegisterMetaType<Guid>("Guid");
	QObject::connect(m_hEvernote, SIGNAL(noteContentUpdated(Guid)), this, SLOT(createNoteSnapshot(Guid)));
	QObject::connect(m_hEvernote, SIGNAL(noteContentUpdated(Guid)), this, SLOT(reloadNotebookList()));
	//QObject::connect(this, SIGNAL(progressDataUpdated(QString)), &syncWidget, SLOT(updateProgressData(QString)));

	m_pKeyGrabber = shared_ptr<QKeyGrabber>(new QKeyGrabber("", this));
	m_pKeyGrabber->setVisible(false);
	QObject::connect(m_pKeyGrabber.get(), SIGNAL(redrawParentContents(QString)), this, SLOT(redrawNotebookList(QString)));
	ui.gridLayout->addWidget(m_pKeyGrabber.get(), 2, 0);

	setProgressIndicator(true);
}

QvernoteView::~QvernoteView()
{

}

void QvernoteView::initView()
{
	ui.lvNotebookList->setVisible(false);
	syncWidget.setVisible(true);

	synchronizeNoteStore();
}

void QvernoteView::loadNotebookList(QString notebookFilter)
{
	Notebook notebook;
	bool hasNextNotebook = false;
	int noteCount = 0;
	setProgressIndicator(true);

	//syncDlg.close();
	ui.lvNotebookList->clear();

	/* Add special "ALL" notebook */
	Notebook specialAllNotebook;
	specialAllNotebook.name = "All Notes";
	QListWidgetItem* iw = new QListWidgetItem(ui.lvNotebookList);
	iw->setSizeHint(QSize(0, 70));
	ui.lvNotebookList->addItem(iw);

	for(hasNextNotebook = m_hEvernote->getFirstNotebook(notebook); hasNextNotebook; hasNextNotebook = m_hEvernote->getNextNotebook(notebook))
	{
		if(QString::fromStdString(notebook.name).startsWith(notebookFilter, Qt::CaseInsensitive))
		{
			//Notebook* newNotebook = new Notebook(notebook);
			QListWidgetItem* iw = new QListWidgetItem(ui.lvNotebookList);
			iw->setSizeHint(QSize(0, 70));
			noteCount += m_hEvernote->getNotebookContentsCount(notebook);
			ui.lvNotebookList->setItemWidget(iw, new QNotebookListItemWidget(notebook, m_hEvernote, -1, false, ui.lvNotebookList));
		}
	}

	ui.lvNotebookList->setItemWidget(iw, new QNotebookListItemWidget(specialAllNotebook, m_hEvernote, noteCount, false, ui.lvNotebookList));

	syncWidget.setVisible(false);
	ui.lvNotebookList->setVisible(true);

	if(m_hEvernote->isOnline())
		ui.pbSync->setEnabled(true);

	setProgressIndicator(false);
}

void QvernoteView::loadNotebook(QListWidgetItem* p_notebookListItem)
{
	QNotebookListItemWidget* p = (QNotebookListItemWidget*)p_notebookListItem->listWidget()->itemWidget(p_notebookListItem);
	qDebug() << trUtf8("Loading Notebook ") << p->getNotebook().name.c_str();

	m_pNotesView = shared_ptr<QNotesView>(new QNotesView(p->getNotebook(), this));
	m_pNotesView->show();
}

void QvernoteView::createNewNotebook()
{
	QNewNotebookDialog pDlg(this);
	pDlg.exec();
}

void QvernoteView::openNewNoteWindow()
{
	qDebug() << "Loading New note window ";
	Notebook m_Notebook;
	m_hEvernote->getDefaultNotebook(m_Notebook);
	m_pNotesView = shared_ptr<QNotesView>(new QNotesView(m_Notebook, this));
	m_pNotesView->show();
	m_pNotesView->openNewNoteWindow();

}

void QvernoteView::synchronizeNoteStore()
{
	setProgressIndicator(true);
	m_pBackgroundJob->start();
	ui.pbSync->setEnabled(false);

	if(!m_hEvernote->isOnline())
		setProgressIndicator(false);
}

void QvernoteView::reloadNotebookList()
{
	initView();
}

void QvernoteView::deleteSelectedNotebook(QNotebookListItemWidget* notebookWidget)
{
	if(m_hEvernote->deleteNotebook(notebookWidget->getNotebook()) == false)
	{
		displayError(trUtf8("Failed to delete notebook"), QString(m_hEvernote->getLastErrorString().c_str()));
	}
}

void QvernoteView::openSearchDialog()
{
	QNoteSearchDialog pDlg(this);
	pDlg.exec();
}

void QvernoteView::searchNote(const QString& searchString, const std::vector<std::string>& tagGuids, const Guid& notebookGuid)
{
	qDebug() << "Searching" << searchString;
	NoteFilter filter;
	setProgressIndicator(true);

	if(searchString.length() > 0)
	{
		filter.words = searchString.toStdString();
		filter.__isset.words = true;
	}

	if(tagGuids.size() > 0)
	{
		filter.tagGuids = tagGuids;
		filter.__isset.tagGuids = true;
	}

	if(notebookGuid.size() > 0)
	{
		filter.notebookGuid = notebookGuid;
		filter.__isset.notebookGuid = true;
	}

	if(m_hEvernote->searchNotes(filter, 100) == false)
	{
		displayError(trUtf8("Note search failed"), QString(m_hEvernote->getLastErrorString().c_str()));
		return;
	}

	setProgressIndicator(false);

	m_pNotesView = shared_ptr<QNotesView>(new QNotesView(trUtf8("Search results for ") + searchString, this));
	m_pNotesView->show();
}

void QvernoteView::orientationChanged()
{
	qDebug() << "Orientation changed";

	 QRect screenGeometry = QApplication::desktop()->screenGeometry();
	 if (screenGeometry.width() > screenGeometry.height())
	 {
		 // Landscape

	 }
	 else
	 {
		 // Portrait
	 }
}

bool QvernoteView::backgroundInit()
{
	bool retVal;
	isSynchronizing = true;
	if((retVal = m_hEvernote->synchronizeLocalStorage()) == true)
		m_hEvernote->loadNotebookList();

	isSynchronizing = false;
	return retVal;
}

void QvernoteView::openDeleteNotebookWindow()
{
	m_pDeleteNotebookView = shared_ptr<QDeleteNotebookView>(new QDeleteNotebookView(this));
	m_pDeleteNotebookView->show();
}

void QvernoteView::openAboutDialog()
{
	QAboutDialog pDlg;
	pDlg.exec();
}

void QvernoteView::openTrashWindow()
{
	m_pTrashWindow = shared_ptr<QTrashWindow>(new QTrashWindow(this));
	m_pTrashWindow->show();
}

void QvernoteView::openFavoritesWindow()
{
	QvernoteStorage::Instance()->getFavoriteNotes(m_hEvernote->getNoteList().notes);
	m_pNotesView = shared_ptr<QNotesView>(new QNotesView(trUtf8("Favorites"), this));
	m_pNotesView->show();
}

void QvernoteView::openOptionsDialog()
{
#ifdef Q_WS_MAEMO_5
	QOptionsDialog pDlg(this);
	pDlg.exec();
#else
	displayError(trUtf8("Cannot open options"), trUtf8("Maemo5 specific code"));
#endif
}

void QvernoteView::keyPressEvent(QKeyEvent* event)
{
	if(!isSearchShown && !isSynchronizing && !event->text().isEmpty())
	{
		isSearchShown = true;
		m_pKeyGrabber->setText(event->text());
		m_pKeyGrabber->setVisible(true);
	}

	QvernoteWindow::keyPressEvent(event);
}

void QvernoteView::redrawNotebookList(QString searchString)
{
	//qDebug() << __FUNCTION__ << searchString;
	if(searchString.isEmpty())
		isSearchShown = false;

	loadNotebookList(searchString);
}

void QvernoteView::clearNotebookList()
{
	ui.lvNotebookList->clear();
	m_hEvernote->loadNotebookList();
}

void QvernoteView::createNoteSnapshot(Guid guid)
{
	Note note;
	note.guid = guid;
	QvernoteAPI::Instance()->getNoteContent(note);
	QEnmlReader* reader = new QEnmlReader(note);
	reader->setLoadExternalReferences(false);
	reader->parse();
	reader->createThumbnail(true);
}
