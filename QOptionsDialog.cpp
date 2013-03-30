#include "QOptionsDialog.h"

#include <QtMaemo5>
#include <QMaemo5ValueButton>
#include <QStandardItemModel>
#include "QvernoteSettings.h"
#include "QvernoteAPI.h"
#include "QvernoteView.h"
#include <QDebug>
#include <QMessageBox>
#include "QNotebookPickList.h"

static void populateListModel(QStandardItemModel *model, QList<QString>& listValues);

QOptionsDialog::QOptionsDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	QvernoteSettings* settings = QvernoteSettings::Instance();
	QList<QString> listValues;

	//ui.leUsername->setInputMethodHints(Qt::ImhPreferLowercase|Qt::ImhNoAutoUppercase);
	//ui.lePassword->setInputMethodHints(Qt::ImhPreferLowercase|Qt::ImhNoAutoUppercase|Qt::ImhHiddenText);
	//ui.leUsername->setText(settings->getUsername());
	//ui.lePassword->setText(settings->getPassword());


	QStandardItemModel* onlineSelectionModel = new QStandardItemModel(0, 1);
	QMaemo5ValueButton* vbOnlineSelection = new QMaemo5ValueButton(trUtf8("Client mode"));
	vbOnlineSelection->setValueLayout(QMaemo5ValueButton::ValueBesideText);
	onlineSelector = new QMaemo5ListPickSelector;

	listValues.clear();
	listValues.append(trUtf8("Online"));
	listValues.append(trUtf8("Offline"));

	populateListModel(onlineSelectionModel, listValues);

	onlineSelector->setModel(onlineSelectionModel);
	onlineSelector->setCurrentIndex((settings->getWorkOnline())? 0 : 1);
	vbOnlineSelection->setPickSelector(onlineSelector);

	ui.gridLayout_2->addWidget(vbOnlineSelection, 4, 0, 1, 2);


	QStandardItemModel* displayOrientationModel = new QStandardItemModel(0, 1);
	QMaemo5ValueButton* vbDisplayOrientation = new QMaemo5ValueButton(trUtf8("Display orientation"));
	vbDisplayOrientation->setValueLayout(QMaemo5ValueButton::ValueBesideText);
	displayOrientationSelector = new QMaemo5ListPickSelector;

	listValues.clear();
	listValues.append(trUtf8("Portrait"));
	listValues.append(trUtf8("Landscape"));
	listValues.append(trUtf8("Auto"));

	populateListModel(displayOrientationModel, listValues);

	displayOrientationSelector->setModel(displayOrientationModel);
	displayOrientationSelector->setCurrentIndex(settings->getDisplayOrientation());
	vbDisplayOrientation->setPickSelector(displayOrientationSelector);

	ui.gridLayout_2->addWidget(vbDisplayOrientation, 5, 0, 1, 2);


	QStandardItemModel* protocolSelectionModel = new QStandardItemModel(0, 1);
	QMaemo5ValueButton* vbProtocolSelection = new QMaemo5ValueButton(trUtf8("Protocol"));
	vbProtocolSelection->setValueLayout(QMaemo5ValueButton::ValueBesideText);
	protocolSelector = new QMaemo5ListPickSelector;

	listValues.clear();
	listValues.append(trUtf8("HTTP"));
	listValues.append(trUtf8("HTTPS"));

	populateListModel(protocolSelectionModel, listValues);

	protocolSelector->setModel(protocolSelectionModel);
	protocolSelector->setCurrentIndex(settings->getUseSsl());
	vbProtocolSelection->setPickSelector(protocolSelector);

	ui.gridLayout_2->addWidget(vbProtocolSelection, 6, 0, 1, 2);

	QStandardItemModel* notesViewSelectionModel = new QStandardItemModel(0, 1);
	QMaemo5ValueButton* vbNotesViewSelection = new QMaemo5ValueButton(trUtf8("Notes view"));
	vbNotesViewSelection->setValueLayout(QMaemo5ValueButton::ValueBesideText);
	notesViewSelector = new QMaemo5ListPickSelector;

	listValues.clear();
	listValues.append(trUtf8("List"));
	listValues.append(trUtf8("Grid"));

	populateListModel(notesViewSelectionModel, listValues);

	notesViewSelector->setModel(notesViewSelectionModel);
	notesViewSelector->setCurrentIndex(settings->getNotesView());
	vbNotesViewSelection->setPickSelector(notesViewSelector);

	ui.gridLayout_2->addWidget(vbNotesViewSelection, 7, 0, 1, 2);

	QStandardItemModel* maxNoteCountModel = new QStandardItemModel(0, 1);
	QMaemo5ValueButton* vbMaxNoteCountSelection = new QMaemo5ValueButton(trUtf8("Note limit"));
	vbMaxNoteCountSelection->setValueLayout(QMaemo5ValueButton::ValueBesideText);
	maxNoteCountSelector = new QMaemo5ListPickSelector;

	listValues.clear();
	listValues.append(trUtf8("No limit"));
	listValues.append("50");
	listValues.append("100");
	listValues.append("150");
	listValues.append("200");
	listValues.append("250");
	listValues.append("300");

	populateListModel(maxNoteCountModel, listValues);

	maxNoteCountSelector->setModel(maxNoteCountModel);
	switch(settings->getMaxNoteCount()) {
	case 0:
		maxNoteCountSelector->setCurrentIndex(0);
		break;
	case 50:
		maxNoteCountSelector->setCurrentIndex(1);
		break;
	case 100:
		maxNoteCountSelector->setCurrentIndex(2);
		break;
	case 150:
		maxNoteCountSelector->setCurrentIndex(3);
		break;
	case 200:
		maxNoteCountSelector->setCurrentIndex(4);
		break;
	case 250:
		maxNoteCountSelector->setCurrentIndex(5);
		break;
	case 300:
		maxNoteCountSelector->setCurrentIndex(6);
		break;
	}
	vbMaxNoteCountSelection->setPickSelector(maxNoteCountSelector);

	ui.gridLayout_2->addWidget(vbMaxNoteCountSelection, 8, 0, 1, 2);

	QStandardItemModel* mapsProviderSelectionModel = new QStandardItemModel(0, 1);
	QMaemo5ValueButton* vbMapsProviderSelection = new QMaemo5ValueButton(trUtf8("Maps provider"));
	vbMapsProviderSelection->setValueLayout(QMaemo5ValueButton::ValueBesideText);
	mapsProviderSelector = new QMaemo5ListPickSelector;

	listValues.clear();
	listValues.append(trUtf8("Ovi maps"));
	listValues.append(trUtf8("Google maps"));

	populateListModel(mapsProviderSelectionModel, listValues);

	mapsProviderSelector->setModel(mapsProviderSelectionModel);
	mapsProviderSelector->setCurrentIndex(settings->getMapsProvider());
	vbMapsProviderSelection->setPickSelector(mapsProviderSelector);

	ui.gridLayout_2->addWidget(vbMapsProviderSelection, 9, 0, 1, 2);

	pbBlacklistedNotebooks.setText(trUtf8("Blacklisted notebooks"));
	m_slBlacklistedNotebooks = settings->getBlacklistedNotebooks().split(',');
	QObject::connect(&pbBlacklistedNotebooks, SIGNAL(clicked()), this, SLOT(selectBlacklistedNotebooks()));
	ui.gridLayout_2->addWidget(&pbBlacklistedNotebooks, 10, 0, 1, 2);

	pbDeleteThumbs.setText(trUtf8("Clear thumbnails"));
	QObject::connect(&pbDeleteThumbs, SIGNAL(clicked()), this, SLOT(deleteThumbnails()));
	ui.gridLayout_2->addWidget(&pbDeleteThumbs, 11, 0, 1, 2);
	pbDropDB.setText(trUtf8("Delete offline database"));
	QObject::connect(&pbDropDB, SIGNAL(clicked()), this, SLOT(onDropDBClick()));
	ui.gridLayout_2->addWidget(&pbDropDB, 12, 0, 1, 2);

	QObject::connect(ui.pbSave, SIGNAL(clicked()), SLOT(onSaveSettingsClick()));
	QObject::connect(this, SIGNAL(triggerDisplayError(QString, QString)), SLOT(onDisplayError(QString, QString)));
	QObject::connect(this, SIGNAL(triggerDisplayInfo(QString)), SLOT(onDisplayInfo(QString)));
	QObject::connect(this, SIGNAL(triggerReloadNotebookList()), SLOT(onReloadNotebookList()));
	QObject::connect(this, SIGNAL(triggerInitView()), SLOT(onInitView()));
	QObject::connect(this, SIGNAL(triggerClearNotebookList()), SLOT(onClearNotebookList()));
}

QOptionsDialog::~QOptionsDialog()
{

}

void QOptionsDialog::onSaveSettingsClick()
{
	setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
	extern void saveSettingsCallback(QOptionsDialog* dlg);
	connect(&watcher, SIGNAL(finished()), this, SLOT(close()));
	future = QtConcurrent::run(saveSettingsCallback, this);
	watcher.setFuture(future);
}

void QOptionsDialog::onDropDBClick()
{
	extern void dropDBCallback(QOptionsDialog* dlg);
	if(QMessageBox::question(this, trUtf8("Delete offline database"), trUtf8("Your online store will be intact. Are you sure?"),
					QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
	{
		setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
		QFuture<void> future = QtConcurrent::run(dropDBCallback, this);
		future.waitForFinished();
		setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
	}
}

void QOptionsDialog::onDisplayError(QString errorTitle, QString errorString)
{
	dynamic_cast<QvernoteView*>(parentWidget())->displayError(errorTitle, errorString);
}

void QOptionsDialog::onDisplayInfo(QString infoString)
{
	dynamic_cast<QvernoteView*>(parentWidget())->displayInformationBox(infoString);
}

void QOptionsDialog::onReloadNotebookList()
{
	dynamic_cast<QvernoteView*>(parentWidget())->reloadNotebookList();
}

void QOptionsDialog::onInitView()
{
	dynamic_cast<QvernoteView*>(parentWidget())->initView();
}

void QOptionsDialog::onClearNotebookList()
{
	dynamic_cast<QvernoteView*>(parentWidget())->clearNotebookList();
}

void QOptionsDialog::saveSettings()
{
	QvernoteSettings* settings = QvernoteSettings::Instance();
	//setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);

	if(!onlineSelector->currentIndex() != settings->getWorkOnline())
	{
		settings->setWorkOnline(!onlineSelector->currentIndex());
		configureOnlineMode(settings->getWorkOnline());
	}


	if(displayOrientationSelector->currentIndex() != (int)settings->getDisplayOrientation())
	{
		settings->setDisplayOrientation((QvernoteSettings::DisplayOrientation)displayOrientationSelector->currentIndex());
		configureOrientation(settings->getDisplayOrientation());
	}

	//if(ui.leUsername->text() != settings->getUsername() || ui.lePassword->text() != settings->getPassword())
	//{
	//	settings->setUsername(ui.leUsername->text());
	//	settings->setPassword(ui.lePassword->text());
	//	configureAccount();
	//}

	if(protocolSelector->currentIndex() != settings->getUseSsl())
	{
		settings->setUseSsl(protocolSelector->currentIndex());
		configureSslProtocol(settings->getUseSsl());
	}

	settings->setNotesView((QvernoteSettings::NotesView)notesViewSelector->currentIndex());
	settings->setMaxNoteCount(maxNoteCountSelector->currentValueText().toInt());
	settings->setMapsProvider((QvernoteSettings::MapsProvider)mapsProviderSelector->currentIndex());

	if(settings->getBlacklistedNotebooks() != m_slBlacklistedNotebooks.join(","))
	{
		settings->setBlacklistedNotebooks(m_slBlacklistedNotebooks.join(","));
		emit triggerReloadNotebookList();
	}
	//close();
}

void QOptionsDialog::configureSslProtocol(bool checked)
{
	QvernoteAPI* h = QvernoteAPI::Instance();

	if(h->reinitNoteStore() == false)
	{
		//dynamic_cast<QvernoteView*>(parentWidget())->displayError(trUtf8("Failed to configure SSL"), QString::fromStdString(h->getLastErrorString()));
		emit triggerDisplayError(trUtf8("Failed to configure SSL"), QString::fromStdString(h->getLastErrorString()));
	}
	else
	{
		//dynamic_cast<QvernoteView*>(parentWidget())->reloadNotebookList();
		emit triggerReloadNotebookList();
	}
}

void QOptionsDialog::configureOnlineMode(bool checked)
{
	QvernoteAPI* h = QvernoteAPI::Instance();

	if(h->setOnline(checked) == false)
	{
		//dynamic_cast<QvernoteView*>(parentWidget())->displayError(QString(trUtf8("Failed to go %1").arg((checked)? "online" : "offline")), QString::fromStdString(h->getLastErrorString()));
		emit triggerDisplayError(QString(trUtf8("Failed to go %1").arg((checked)? trUtf8("online") : trUtf8("offline"))), QString::fromStdString(h->getLastErrorString()));
	}
	else
	{
		//dynamic_cast<QvernoteView*>(parentWidget())->displayInformationBox(QString(trUtf8("Working %1").arg((checked)? "online" : "offline")));
		emit triggerDisplayInfo(QString(trUtf8("Working %1").arg((checked)? trUtf8("online") : trUtf8("offline"))));

		if(h->isOnline())
			//dynamic_cast<QvernoteView*>(parentWidget())->initView();
			emit triggerInitView();
	}
}

void QOptionsDialog::configureAccount()
{
	QvernoteAPI* h = QvernoteAPI::Instance();
	QvernoteSettings* settings = QvernoteSettings::Instance();

	if(h->getAuthenticationToken())
	{
		if(h->initNoteStore() == true)
		{
			//dynamic_cast<QvernoteView*>(parentWidget())->reloadNotebookList();
			emit triggerReloadNotebookList();
		}
	}
	else
	{
		//dynamic_cast<QvernoteView*>(parentWidget())->displayError(trUtf8("Login error"), QString::fromStdString(h->getLastErrorString()));
		emit triggerDisplayError(trUtf8("Login error"), QString::fromStdString(h->getLastErrorString()));
	}
}

void QOptionsDialog::configureOrientation(QvernoteSettings::DisplayOrientation o)
{
	dynamic_cast<QvernoteView*>(parentWidget())->setDisplayOrientation(o);
}

void QOptionsDialog::selectBlacklistedNotebooks()
{
	QNotebookPickList dlg("", this);
	dlg.setMultipleSelection(true);
	QObject::connect(&dlg, SIGNAL(notebookSelected(bool, QString, QString)), this, SLOT(onBlacklistedNotebookSelected(bool, QString, QString)));
	m_slBlacklistedNotebooks = QvernoteSettings::Instance()->getBlacklistedNotebooks().split(',');
	dlg.loadNotebooks(m_slBlacklistedNotebooks);
	dlg.exec();
}

void QOptionsDialog::onBlacklistedNotebookSelected(bool checked, QString notebookName, QString notebookGuid)
{
	if(checked)
		m_slBlacklistedNotebooks.append(notebookGuid);
	else
		m_slBlacklistedNotebooks.removeAll(notebookGuid);
}

void QOptionsDialog::deleteThumbnails()
{
	if(QMessageBox::question(this, trUtf8("Delete thumbnails"), trUtf8("Are you sure?"),
			QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
	{
		QvernoteStorage::Instance()->clearThumbnails();
	}
}

void QOptionsDialog::dropDB()
{
	QvernoteStorage::Instance()->clearDatabase();
	QvernoteSettings::Instance()->setlastUpdateCount(0);
	emit triggerClearNotebookList();
}

void populateListModel(QStandardItemModel *model, QList<QString>& listValues)
 {
     for (int i = 0; i < listValues.size(); ++i) {
         QStandardItem *item = new QStandardItem(listValues[i]);
         item->setTextAlignment(Qt::AlignCenter); // the Maemo 5 design spec recommends this.
         item->setEditable(false); // prevent editing of the item
         model->appendRow(item);
     }
 }

void saveSettingsCallback(QOptionsDialog* dlg)
{
	dlg->saveSettings();
}

void dropDBCallback(QOptionsDialog* dlg)
{
	dlg->dropDB();
}
