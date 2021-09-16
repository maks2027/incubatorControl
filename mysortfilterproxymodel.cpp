/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mysortfilterproxymodel.h"

#include <QtWidgets>

MySortFilterProxyModel::MySortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    box1RegExp.setCaseSensitivity(Qt::CaseInsensitive);
    box2RegExp.setCaseSensitivity(Qt::CaseInsensitive);
}

void MySortFilterProxyModel::setFilterMinimumDate(QDate date)
{
    minDate = date;
    invalidateFilter();
    emit filterChanged();
}

void MySortFilterProxyModel::setFilterMaximumDate(QDate date)
{
    maxDate = date;
    invalidateFilter();
    emit filterChanged();
}

bool MySortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    QModelIndex indexBox1 = sourceModel()->index(sourceRow, 1, sourceParent);
    QModelIndex indexBox2 = sourceModel()->index(sourceRow, 4, sourceParent);
    QModelIndex indexDateMin = sourceModel()->index(sourceRow, 3, sourceParent);
    QModelIndex indexDateMax = sourceModel()->index(sourceRow, 7, sourceParent);

    return (sourceModel()->data(indexBox1).toString().contains(box1RegExp)
            && sourceModel()->data(indexBox2).toString().contains(box2RegExp))
            && dateInRange(sourceModel()->data(indexDateMin).toDate())
            && dateInRange(sourceModel()->data(indexDateMax).toDate());
}

bool MySortFilterProxyModel::dateInRange(QDate date) const
{
    if(enableMinDate && !minDate.isValid())return true;
    if(enableMaxDate && !maxDate.isValid())return true;

    return (!enableMinDate || date > minDate)
            && (!enableMaxDate || date < maxDate);
}

void MySortFilterProxyModel::setEnableMaxDate(bool value)
{
    enableMaxDate = value;
    invalidateFilter();
    emit filterChanged();
}

void MySortFilterProxyModel::setBox1Filter(QString str)
{
    box1RegExp.setPattern(str);
    invalidateFilter();
    emit filterChanged();
}

void MySortFilterProxyModel::setBox2Filter(QString str)
{
     box2RegExp.setPattern(str);
     invalidateFilter();
     emit filterChanged();
}

void MySortFilterProxyModel::setEnableMinDate(bool value)
{
    enableMinDate = value;
    invalidateFilter();
    emit filterChanged();
}
