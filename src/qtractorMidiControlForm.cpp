// qtractorMidiControlForm.cpp
//
/****************************************************************************
   Copyright (C) 2005-2009, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qtractorMidiControlForm.h"

#include "qtractorAbout.h"
#include "qtractorMidiControl.h"
#include "qtractorOptions.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QUrl>


//----------------------------------------------------------------------
// class qtractorMidiControlForm -- MIDI controller file manager form.
//

// Constructor.
qtractorMidiControlForm::qtractorMidiControlForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	m_iDirtyCount = 0;

	QHeaderView *pHeader = m_ui.FilesListView->header();
//	pHeader->setResizeMode(QHeaderView::Custom);
	pHeader->setDefaultAlignment(Qt::AlignLeft);
	pHeader->setMovable(false);

	refreshForm();
	adjustSize();

	// UI signal/slot connections...
	QObject::connect(m_ui.FilesListView,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		SLOT(stabilizeForm()));
	QObject::connect(m_ui.ImportPushButton,
		SIGNAL(clicked()),
		SLOT(importSlot()));
	QObject::connect(m_ui.RemovePushButton,
		SIGNAL(clicked()),
		SLOT(removeSlot()));
	QObject::connect(m_ui.MoveUpPushButton,
		SIGNAL(clicked()),
		SLOT(moveUpSlot()));
	QObject::connect(m_ui.MoveDownPushButton,
		SIGNAL(clicked()),
		SLOT(moveDownSlot()));
	QObject::connect(m_ui.ExportPushButton,
		SIGNAL(clicked()),
		SLOT(exportSlot()));
	QObject::connect(m_ui.ApplyPushButton,
		SIGNAL(clicked()),
		SLOT(applySlot()));
	QObject::connect(m_ui.ClosePushButton,
		SIGNAL(clicked()),
		SLOT(reject()));
}


// Destructor.
qtractorMidiControlForm::~qtractorMidiControlForm (void)
{
}


// Accept settings (OK button slot).
void qtractorMidiControlForm::accept (void)
{
	// If we're dirty do a complete and final reload...
	if (m_iDirtyCount > 0)
		applySlot();

	// Just go with dialog acceptance.
	QDialog::accept();
}


// Reject settings (Cancel button slot).
void qtractorMidiControlForm::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("Controller settings have been changed.") + "\n\n" +
			tr("Do you want to apply the changes?"),
			QMessageBox::Apply |
			QMessageBox::Discard |
			QMessageBox::Cancel)) {
		case QMessageBox::Apply:
			accept();
			return;
		case QMessageBox::Discard:
			break;
		default:    // Cancel.
			bReject = false;
		}
	}

	if (bReject)
		QDialog::reject();
}


// Import new intrument file(s) into listing.
void qtractorMidiControlForm::importSlot (void)
{
	qtractorMidiControl *pMidiControl = qtractorMidiControl::getInstance();
	if (pMidiControl == NULL)
		return;

	qtractorOptions *pOptions = qtractorOptions::getInstance();
	if (pOptions == NULL)
		return;

	QStringList files;

	const QString  sExt("qtc");
	const QString& sTitle  = tr("Import MIDI Controller Files") + " - " QTRACTOR_TITLE;
	const QString& sFilter = tr("MIDI Controller files (*.%1)").arg(sExt);
#if QT_VERSION < 0x040400
	// Ask for the filename to open...
	files = QFileDialog::getOpenFileNames(this,
		sTitle, pOptions->sMidiControlDir, sFilter);
#else
	// Construct open-files dialog...
	QFileDialog fileDialog(this,
		sTitle, pOptions->sMidiControlDir, sFilter);
	// Set proper open-file modes...
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setFileMode(QFileDialog::ExistingFiles);
	fileDialog.setDefaultSuffix(sExt);
	// Stuff sidebar...
	QList<QUrl> urls(fileDialog.sidebarUrls());
	urls.append(QUrl::fromLocalFile(pOptions->sSessionDir));
	urls.append(QUrl::fromLocalFile(pOptions->sMidiControlDir));
	fileDialog.setSidebarUrls(urls);
	// Show dialog...
	if (fileDialog.exec())
		files = fileDialog.selectedFiles();
#endif

	if (files.isEmpty())
		return;

	// Remember this last directory...
	
	// For avery selected controller file to load...
	QTreeWidgetItem *pItem = NULL;
	QStringListIterator iter(files);
	while (iter.hasNext()) {
		// Merge the file contents into global container...
		const QString& sPath = iter.next();
		if (pMidiControl->loadDocument(sPath)) {
			// Start inserting in the current selected or last item...
			if (pItem == NULL)
				pItem = m_ui.FilesListView->currentItem();
			if (pItem == NULL) {
				int iLastItem = m_ui.FilesListView->topLevelItemCount() - 1;
				if (iLastItem >= 0)
					pItem = m_ui.FilesListView->topLevelItem(iLastItem);
			}
			// New item on the block :-)
			pItem = new QTreeWidgetItem(m_ui.FilesListView, pItem);
			if (pItem) {
				QFileInfo info(sPath);
				pItem->setIcon(0, QIcon(":/icons/itemFile.png"));
				pItem->setText(0, info.fileName());
				pItem->setText(1, sPath);
				m_ui.FilesListView->setCurrentItem(pItem);
				pOptions->sMidiControlDir = info.absolutePath();
				m_iDirtyCount++;
			}
		}
	}

	stabilizeForm();
}


// Remove a file from controller list.
void qtractorMidiControlForm::removeSlot (void)
{
	QTreeWidgetItem *pItem = m_ui.FilesListView->currentItem();
	if (pItem) {
		delete pItem;
		m_iDirtyCount++;
	}

	stabilizeForm();
}


// Move a file up on the controller list.
void qtractorMidiControlForm::moveUpSlot (void)
{
	QTreeWidgetItem *pItem = m_ui.FilesListView->currentItem();
	if (pItem) {
		int iItem = m_ui.FilesListView->indexOfTopLevelItem(pItem);
		if (iItem > 0) {
			pItem = m_ui.FilesListView->takeTopLevelItem(iItem);
			m_ui.FilesListView->insertTopLevelItem(iItem - 1, pItem);
			m_ui.FilesListView->setCurrentItem(pItem);
			m_iDirtyCount++;
		}
	}

	stabilizeForm();
}


// Move a file down on the controller list.
void qtractorMidiControlForm::moveDownSlot (void)
{
	QTreeWidgetItem *pItem = m_ui.FilesListView->currentItem();
	if (pItem) {
		int iItem = m_ui.FilesListView->indexOfTopLevelItem(pItem);
		if (iItem < m_ui.FilesListView->topLevelItemCount() - 1) {
			pItem = m_ui.FilesListView->takeTopLevelItem(iItem);
			m_ui.FilesListView->insertTopLevelItem(iItem + 1, pItem);
			m_ui.FilesListView->setCurrentItem(pItem);
			m_iDirtyCount++;
		}
	}

	stabilizeForm();
}


// Export the whole state into a single controller file.
void qtractorMidiControlForm::exportSlot (void)
{
	qtractorMidiControl *pMidiControl = qtractorMidiControl::getInstance();
	if (pMidiControl == NULL)
		return;

	qtractorOptions *pOptions = qtractorOptions::getInstance();
	if (pOptions == NULL)
		return;

	QString sPath;

	const QString  sExt("qtc");
	const QString& sTitle  = tr("Export MIDI Controller File") + " - " QTRACTOR_TITLE;
	const QString& sFilter = tr("MIDI Controller files (*.%1)").arg(sExt);
#if QT_VERSION < 0x040400
	// Ask for the filename to open...
	sPath = QFileDialog::getOpenFileName(this,
		sTitle, pOptions->sMidiControlDir, sFilter);
#else
	// Construct open-files dialog...
	QFileDialog fileDialog(this,
		sTitle, pOptions->sMidiControlDir, sFilter);
	// Set proper open-file modes...
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	fileDialog.setFileMode(QFileDialog::AnyFile);
	fileDialog.setDefaultSuffix(sExt);
	// Stuff sidebar...
	QList<QUrl> urls(fileDialog.sidebarUrls());
	urls.append(QUrl::fromLocalFile(pOptions->sSessionDir));
	urls.append(QUrl::fromLocalFile(pOptions->sMidiControlDir));
	fileDialog.setSidebarUrls(urls);
	// Show dialog...
	if (fileDialog.exec())
		sPath = fileDialog.selectedFiles().first();
#endif

	if (sPath.isEmpty())
		return;

	// Enforce .qtc extension...
	if (QFileInfo(sPath).suffix().isEmpty())
		sPath += '.' + sExt;

	// Check if already exists...
	if (QFileInfo(sPath).exists()) {
		if (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("The controller file already exists:\n\n"
			"\"%1\"\n\n"
			"Do you want to replace it?")
			.arg(sPath),
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
			return;
	}

	// Just save the whole bunch...
	if (pMidiControl->saveDocument(sPath))
		pOptions->sMidiControlDir = QFileInfo(sPath).absolutePath();
}


// Reload the complete controller configurations, from list.
void qtractorMidiControlForm::applySlot (void)
{
	qtractorMidiControl *pMidiControl = qtractorMidiControl::getInstance();
	if (pMidiControl == NULL)
		return;

	qtractorOptions *pOptions = qtractorOptions::getInstance();
	if (pOptions == NULL)
		return;

	// Ooops...
	pMidiControl->clear();
	pOptions->midiControlFiles.clear();

	// Load each file in order...
	int iItemCount = m_ui.FilesListView->topLevelItemCount();
	for (int iItem = 0; iItem < iItemCount; ++iItem) {
		QTreeWidgetItem *pItem = m_ui.FilesListView->topLevelItem(iItem);
		if (pItem) {
			const QString& sPath = pItem->text(1);
			if (pMidiControl->loadDocument(sPath))
				pOptions->midiControlFiles.append(sPath);
		}
	}

	// Not dirty anymore...
	m_iDirtyCount = 0;

	stabilizeForm();
}


// Refresh all controller definition views.
void qtractorMidiControlForm::refreshForm (void)
{
	qtractorMidiControl *pMidiControl = qtractorMidiControl::getInstance();
	if (pMidiControl == NULL)
		return;

	qtractorOptions *pOptions = qtractorOptions::getInstance();
	if (pOptions == NULL)
		return;

	// Freeze...
	m_ui.FilesListView->setUpdatesEnabled(false);

	// Files list view...
	m_ui.FilesListView->clear();
	QList<QTreeWidgetItem *> files;
	QStringListIterator iter(pOptions->midiControlFiles);
	while (iter.hasNext()) {
		const QString& sPath = iter.next();
		QTreeWidgetItem *pFileItem = new QTreeWidgetItem();
		pFileItem->setIcon(0, QIcon(":/icons/itemFile.png"));
		pFileItem->setText(0, QFileInfo(sPath).fileName());
		pFileItem->setText(1, sPath);
		files.append(pFileItem);
	}
	m_ui.FilesListView->addTopLevelItems(files);

	// Bail out...
	m_ui.FilesListView->setUpdatesEnabled(true);

	stabilizeForm();
}


// Stabilize form status.
void qtractorMidiControlForm::stabilizeForm (void)
{
	QTreeWidgetItem *pItem = m_ui.FilesListView->currentItem();
	if (pItem) {
		int iItem = m_ui.FilesListView->indexOfTopLevelItem(pItem);
		int iItemCount = m_ui.FilesListView->topLevelItemCount();
		m_ui.RemovePushButton->setEnabled(true);
		m_ui.MoveUpPushButton->setEnabled(iItem > 0);
		m_ui.MoveDownPushButton->setEnabled(iItem < iItemCount - 1);
	} else {
		m_ui.RemovePushButton->setEnabled(false);
		m_ui.MoveUpPushButton->setEnabled(false);
		m_ui.MoveDownPushButton->setEnabled(false);
	}

	m_ui.ExportPushButton->setEnabled(m_iDirtyCount > 0);
	m_ui.ApplyPushButton->setEnabled(m_iDirtyCount > 0);
}


// end of qtractorMidiControlForm.cpp