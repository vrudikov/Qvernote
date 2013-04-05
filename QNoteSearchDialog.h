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
#ifndef QNOTESEARCHDIALOG_H
#define QNOTESEARCHDIALOG_H

#include <QtGui/QDialog>
#include "ui_QNoteSearchDialog.h"
#include "QvernoteAPI.h"

class QNoteSearchDialog : public QDialog
{
    Q_OBJECT

public:
    QNoteSearchDialog(QWidget *parent = 0);
    ~QNoteSearchDialog();

public slots:
	void searchNote();
	void openTagSelectionDialog();
	void openNotebookSelectionDialog();
	void setTag(bool state, Tag tag);

signals:
	void searchSubmit(QString searchString);

private:
    Ui::QNoteSearchDialogClass ui;
    QWidget*	m_parent;
    QList<QStringList>	selectedTagsList;
    QStringList			selectedNotebook;
};

#endif // QNOTESEARCHDIALOG_H
