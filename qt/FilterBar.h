/*
 * This file Copyright (C) 2010-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#pragma once

#include <map>
#include <unordered_map>

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
    FilterBar(Prefs& prefs, TorrentModel const& torrents, TorrentFilter const& filter, QWidget* parent = nullptr);
    virtual ~FilterBar();

public slots:
    void clear();

private:
    using Map = std::map<QString, int>;
    using MapIter = Map::const_iterator;
    using Counts = std::unordered_map<QString, int>;

    FilterBarComboBox* createTrackerCombo(QStandardItemModel*);
    FilterBarComboBox* createActivityCombo();
    FilterBarComboBox* createPathCombo(QStandardItemModel*);
    void refreshFilter(Map& map, QStandardItemModel* model, Counts& counts, QStandardItem* (*update)(QStandardItem* i,
        MapIter const& it), int key);
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
    TorrentModel const& myTorrents;
    TorrentFilter const& myFilter;

    FilterBarComboBox* myActivityCombo;
    FilterBarComboBox* myPathCombo;
    FilterBarComboBox* myTrackerCombo;
    QLabel* myCountLabel;
    QStandardItemModel* myPathModel;
    QStandardItemModel* myTrackerModel;
    QTimer* myRecountTimer;
    bool myIsBootstrapping;
    QLineEdit* myLineEdit;
    Map myPathCounts;
    Map myTrackerCounts;
};
