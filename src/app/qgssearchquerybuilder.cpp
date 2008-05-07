/***************************************************************************
    qgssearchquerybuilder.cpp  - Query builder for search strings
    ----------------------
    begin                : March 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <iostream>
#include <QListView>
#include <QMessageBox>
#include <QStandardItem>
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgssearchquerybuilder.h"
#include "qgssearchstring.h"
#include "qgssearchtreenode.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#if QT_VERSION < 0x040300
#define toPlainText() text()
#endif


QgsSearchQueryBuilder::QgsSearchQueryBuilder(QgsVectorLayer* layer,
                                             QWidget *parent, Qt::WFlags fl)
  : QDialog(parent, fl), mLayer(layer)
{
  setupUi(this);
  setupListViews();
  
  setWindowTitle(tr("Search query builder"));
  
  // disable unsupported operators
  btnIn->setEnabled(false);
  btnNotIn->setEnabled(false);
  btnPct->setEnabled(false);
  
  // change to ~
  btnILike->setText("~");
  
  lblDataUri->setText(layer->name());
  populateFields();
}

QgsSearchQueryBuilder::~QgsSearchQueryBuilder()
{
}


void QgsSearchQueryBuilder::populateFields()
{
#ifdef QGISDEBUG
  std::cout << "QgsSearchQueryBuilder::populateFields" << std::endl;
#endif
  const QgsFieldMap& fields = mLayer->getDataProvider()->fields();
  for (QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it)
  {
    QString fieldName = it->name();
    mFieldMap[fieldName] = it.key();
    QStandardItem *myItem = new QStandardItem(fieldName);
    myItem->setEditable(false);
    mModelFields->insertRow(mModelFields->rowCount(),myItem);
  }
}

void QgsSearchQueryBuilder::setupListViews()
{
#ifdef QGISDEBUG
  std::cout << "QgsSearchQueryBuilder::setupListViews" << std::endl;
#endif
  //Models
  mModelFields = new QStandardItemModel();
  mModelValues = new QStandardItemModel();
  lstFields->setModel(mModelFields);
  lstValues->setModel(mModelValues);
  // Modes
  lstFields->setViewMode(QListView::ListMode);
  lstValues->setViewMode(QListView::ListMode);
  lstFields->setSelectionBehavior(QAbstractItemView::SelectRows);
  lstValues->setSelectionBehavior(QAbstractItemView::SelectRows);
  // Performance tip since Qt 4.1
  lstFields->setUniformItemSizes(true);
  lstValues->setUniformItemSizes(true);
}

void QgsSearchQueryBuilder::getFieldValues(uint limit)
{
  // clear the values list 
  mModelValues->clear();
  
  QgsVectorDataProvider* provider = mLayer->getDataProvider();
  
  // determine the field type
  QString fieldName = mModelFields->data(lstFields->currentIndex()).toString();
  int fieldIndex = mFieldMap[fieldName];
  QgsField field = provider->fields()[fieldIndex];
  bool numeric = (field.type() == QVariant::Int || field.type() == QVariant::Double);
  
  QgsFeature feat;
  QString value;

  QgsAttributeList attrs;
  attrs.append(fieldIndex);
  
  provider->select(attrs, QgsRect(), false);
  
  lstValues->setCursor(Qt::WaitCursor);
  // Block for better performance
  mModelValues->blockSignals(true);
  lstValues->setUpdatesEnabled(false);
  
  while (provider->getNextFeature(feat) &&
         (limit == 0 || mModelValues->rowCount() != limit))
  {
    const QgsAttributeMap& attributes = feat.attributeMap();
    value = attributes[fieldIndex].toString();
     
    if (!numeric)
    {
      // put string in single quotes
      value = "'" + value + "'";
    }
    
    // add item only if it's not there already
    QList<QStandardItem *> items = mModelValues->findItems(value);
    if (items.isEmpty())
    {
      QStandardItem *myItem = new QStandardItem(value);
      myItem->setEditable(false);
      mModelValues->insertRow(mModelValues->rowCount(),myItem);
    }
  }
  // Unblock for normal use
  mModelValues->blockSignals(false);
  lstValues->setUpdatesEnabled(true);
  // TODO: already sorted, signal emit to refresh model 
  mModelValues->sort(0);
  lstValues->setCursor(Qt::ArrowCursor);
}

void QgsSearchQueryBuilder::on_btnSampleValues_clicked()
{
  getFieldValues(25);
}

void QgsSearchQueryBuilder::on_btnGetAllValues_clicked()
{
  getFieldValues(0);
}

void QgsSearchQueryBuilder::on_btnTest_clicked()
{
  long count = countRecords(txtSQL->toPlainText());
  
  // error?
  if (count == -1)
    return;

  QString str;
  if (count)
    str.sprintf(tr("Found %d matching features.","",count).toUtf8(), count);
  else
    str = tr("No matching features found.");
  QMessageBox::information(this, tr("Search results"), str);
}

// This method tests the number of records that would be returned
long QgsSearchQueryBuilder::countRecords(QString searchString) 
{
  QgsSearchString search;
  if (!search.setString(searchString))
  {
    QMessageBox::critical(this, tr("Search string parsing error"), search.parserErrorMsg());
    return -1;
  }
  
  QgsSearchTreeNode* searchTree = search.tree();
  if (searchTree == NULL)
  {
    // entered empty search string
    return mLayer->featureCount();
  }
  
  QApplication::setOverrideCursor(Qt::waitCursor);
  
  int count = 0;
  QgsFeature feat;
  QgsVectorDataProvider* provider = mLayer->getDataProvider();
  const QgsFieldMap& fields = provider->fields();
  QgsAttributeList allAttributes = provider->allAttributesList();

  provider->select(allAttributes, QgsRect(), false);

  while (provider->getNextFeature(feat))
  {
    if (searchTree->checkAgainst(fields, feat.attributeMap()))
    {
      count++;
    }
    
    // check if there were errors during evaulating
    if (searchTree->hasError())
      break;
  }

  QApplication::restoreOverrideCursor();
  
  return count;
}


void QgsSearchQueryBuilder::on_btnOk_clicked()
{
  // if user hits Ok and there is no query, skip the validation
  if(txtSQL->toPlainText().stripWhiteSpace().length() > 0)
  {
    accept();
    return;
  }

  // test the query to see if it will result in a valid layer
  long numRecs = countRecords(txtSQL->toPlainText());
  if (numRecs == -1)
  {
    // error shown in countRecords
  }
  else if (numRecs == 0)
  {
    QMessageBox::warning(this, tr("No Records"), tr("The query you specified results in zero records being returned."));
  }
  else
  {
    accept();
  }

}

void QgsSearchQueryBuilder::on_btnEqual_clicked()
{
  txtSQL->insert(" = ");
}

void QgsSearchQueryBuilder::on_btnLessThan_clicked()
{
  txtSQL->insert(" < ");
}

void QgsSearchQueryBuilder::on_btnGreaterThan_clicked()
{
  txtSQL->insert(" > ");
}

void QgsSearchQueryBuilder::on_btnPct_clicked()
{
  txtSQL->insert(" % ");
}

void QgsSearchQueryBuilder::on_btnIn_clicked()
{
  txtSQL->insert(" IN ");
}

void QgsSearchQueryBuilder::on_btnNotIn_clicked()
{
  txtSQL->insert(" NOT IN ");
}

void QgsSearchQueryBuilder::on_btnLike_clicked()
{
  txtSQL->insert(" LIKE ");
}

QString QgsSearchQueryBuilder::searchString()
{
  return txtSQL->toPlainText();
}

void QgsSearchQueryBuilder::setSearchString(QString searchString)
{
  txtSQL->setText(searchString);
}

void QgsSearchQueryBuilder::on_lstFields_doubleClicked( const QModelIndex &index )
{
  txtSQL->insert(mModelFields->data(index).toString());
}

void QgsSearchQueryBuilder::on_lstValues_doubleClicked( const QModelIndex &index )
{
  txtSQL->insert(mModelValues->data(index).toString());
}

void QgsSearchQueryBuilder::on_btnLessEqual_clicked()
{
  txtSQL->insert(" <= ");
}

void QgsSearchQueryBuilder::on_btnGreaterEqual_clicked()
{
  txtSQL->insert(" >= ");
}

void QgsSearchQueryBuilder::on_btnNotEqual_clicked()
{
  txtSQL->insert(" != ");
}

void QgsSearchQueryBuilder::on_btnAnd_clicked()
{
  txtSQL->insert(" AND ");
}

void QgsSearchQueryBuilder::on_btnNot_clicked()
{
  txtSQL->insert(" NOT ");
}

void QgsSearchQueryBuilder::on_btnOr_clicked()
{
  txtSQL->insert(" OR ");
}

void QgsSearchQueryBuilder::on_btnClear_clicked()
{
  txtSQL->clear();
}

void QgsSearchQueryBuilder::on_btnILike_clicked()
{
  //txtSQL->insert(" ILIKE ");
  txtSQL->insert(" ~ ");
}

