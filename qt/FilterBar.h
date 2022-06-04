/*
 * This file Copyright (C) 2010-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QStandardItem;
class QStandardItemModel;
class QTimer;

class FilterBarComboBox;
class Prefs;
class TorrentFilter;
class TorrentModel;

class FilterBar : public QWidget
{
    Q_OBJECT

public:
    FilterBar(Prefs& prefs, TorrentModel& torrents, TorrentFilter const& filter, QWidget* parent = nullptr);
    virtual ~FilterBar();

public slots:
    void clear();

private:
    FilterBarComboBox* createTrackerCombo(QStandardItemModel*);
    FilterBarComboBox* createActivityCombo();
    FilterBarComboBox* createPathCombo(QStandardItemModel*);
    void refreshTrackers();

private slots:
    void recountSoon();
    void recount();
    void refreshPref(int key);
    void onActivityIndexChanged(int index);
    void onPathIndexChanged(int index);
    void onTrackerIndexChanged(int index);
    void onTextChanged(QString const&);

private:
    Prefs& myPrefs;
    TorrentModel& myTorrents;
    TorrentFilter const& myFilter;

    FilterBarComboBox* myActivityCombo;
    FilterBarComboBox* myPathCombo;
    FilterBarComboBox* myTrackerCombo;
    QLabel* myCountLabel;
    QTimer* myRecountTimer;
    bool myIsBootstrapping;
    QLineEdit* myLineEdit;
};
