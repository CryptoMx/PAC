// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_PROPOSALTABLEMODEL_H
#define BITCOIN_QT_PROPOSALTABLEMODEL_H

#include "bitcoinunits.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class PlatformStyle;
class ProposalRecord;
class ProposalTablePriv;

class ProposalTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ProposalTableModel(const PlatformStyle *platformStyle, QObject *parent = 0);
    ~ProposalTableModel();

    enum ColumnIndex {
        Proposal = 0,
        Amount = 1,
        StartDate = 2,
        EndDate = 3,
        YesVotes = 4,
        NoVotes = 5,
        AbsoluteYesVotes = 6,
        Percentage = 7
    };

    enum RoleIndex {
        ProposalRole = Qt::UserRole,
        AmountRole,
        StartDateRole,
        EndDateRole,
        YesVotesRole,
        NoVotesRole,
        AbsoluteYesVotesRole,
        PercentageRole,
        ProposalUrlRole,
        ProposalHashRole,
    };

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    void refreshProposals();
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;

private:
    QList<ProposalRecord*> proposalRecords;
    QStringList columns;
    const PlatformStyle *platformStyle;
};

#endif // BITCOIN_QT_PROPOSALTABLEMODEL_H
