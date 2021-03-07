// This file Copyright © 2010-2022 Mnemosyne LLC.
// It may be used under GPLv2 (SPDX: GPL-2.0-only), GPLv3 (SPDX: GPL-3.0-only),
// or any future license endorsed by Mnemosyne LLC.
// License text can be found in the licenses/ folder.

#pragma once

#include <bitset>
#include <map>
#include <unordered_map>

#include <QLineEdit>
#include <QStandardItemModel>
#include <QTimer>
#include <QWidget>

#include <libtransmission/tr-macros.h>

#include "FaviconCache.h"
#include "Torrent.h"
#include "Typedefs.h"

class QLabel;
class QLineEdit;
class QStandardItem;
class QString;

class FilterBarComboBox;
class Prefs;
class TorrentFilter;
class TorrentModel;

class FilterBar : public QWidget
{
    Q_OBJECT
    TR_DISABLE_COPY_MOVE(FilterBar)

public:
    FilterBar(Prefs& prefs, TorrentModel const& torrents, TorrentFilter const& filter, QWidget* parent = nullptr);

public slots:
    void clear();

private:
    using Map = std::map<QString, int>;
    using MapIter = Map::const_iterator;
    using Counts = std::unordered_map<QString, int>;
    using MapUpdate = QStandardItem* (*)(QStandardItem* i, MapIter const& it);

    FilterBarComboBox* createTrackerCombo(QStandardItemModel*);
    FilterBarComboBox* createActivityCombo();
    FilterBarComboBox* createPathCombo(QStandardItemModel*);
    void refreshFilter(Map& map, QStandardItemModel* model, Counts& counts, MapUpdate itemUpdate, int key);
    void refreshTrackers();

    enum
    {
        ACTIVITY,
        TRACKERS,

        NUM_FLAGS
    };

    using Pending = std::bitset<NUM_FLAGS>;

    Prefs& prefs_;
    TorrentModel const& torrents_;
    TorrentFilter const& filter_;

    Map path_counts_;
    Map sitename_counts_;
    FilterBarComboBox* activity_combo_ = createActivityCombo();
    FilterBarComboBox* path_combo_ = {};
    FilterBarComboBox* tracker_combo_ = {};
    QLabel* count_label_ = {};
    QStandardItemModel* const path_model_ = new QStandardItemModel{ this };
    QStandardItemModel* const tracker_model_ = new QStandardItemModel{ this };
    QTimer recount_timer_;
    QLineEdit* const line_edit_ = new QLineEdit{ this };
    Pending pending_ = {};
    bool is_bootstrapping_ = {};

private slots:
    void recount();
    void recountSoon(Pending const& fields);

    void recountActivitySoon()
    {
        recountSoon(Pending().set(ACTIVITY));
    }

    void recountTrackersSoon()
    {
        recountSoon(Pending().set(TRACKERS));
    }

    void recountAllSoon()
    {
        recountSoon(Pending().set(ACTIVITY).set(TRACKERS));
    }

    void refreshPref(int key);
    void onActivityIndexChanged(int index);
    void onPathIndexChanged(int index);
    void onTextChanged(QString const&);
    void onTorrentsChanged(torrent_ids_t const&, Torrent::fields_t const& fields);
    void onTrackerIndexChanged(int index);
};
