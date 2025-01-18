// This file Copyright © 2010-2023 Mnemosyne LLC.
// It may be used under GPLv2 (SPDX: GPL-2.0-only), GPLv3 (SPDX: GPL-3.0-only),
// or any future license endorsed by Mnemosyne LLC.
// License text can be found in the licenses/ folder.

#pragma once

#include <bitset>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <QAbstractListModel>
#include <QStandardItemModel>
#include <QTimer>

#include <libtransmission/tr-macros.h>

#include "Torrent.h"
#include "Typedefs.h"

class Prefs;
class Speed;

extern "C"
{
    struct tr_variant;
}

class TorrentModel : public QAbstractListModel
{
    Q_OBJECT
    TR_DISABLE_COPY_MOVE(TorrentModel)

public:
    enum Role
    {
        TorrentRole = Qt::UserRole,
        CountRole,
        CountStringRole,
        ActivityRole,
        PathRole,
        TrackerRole,
    };

    explicit TorrentModel(Prefs const& prefs);
    ~TorrentModel() override;
    void clear();

    bool hasTorrent(TorrentHash const& hash) const;

    Torrent* getTorrentFromId(int id);
    Torrent const* getTorrentFromId(int id) const;

    using torrents_t = std::vector<Torrent*>;

    torrents_t const& torrents() const
    {
        return torrents_;
    }

    QStandardItemModel* pathFilterModel() const
    {
        return path_model_;
    }

    QStandardItemModel* trackerFilterModel() const
    {
        return tracker_model_;
    }

    // QAbstractItemModel
    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

public slots:
    void updateTorrents(tr_variant* torrent_list, bool is_complete_list);
    void removeTorrents(tr_variant* torrent_list);

signals:
    void torrentsAdded(torrent_ids_t const&);
    void torrentsChanged(torrent_ids_t const&, Torrent::fields_t const& fields);
    void torrentsCompleted(torrent_ids_t const&);
    void torrentsEdited(torrent_ids_t const&);
    void torrentsNeedInfo(torrent_ids_t const&);
    void filterChanged(int);

private:
    enum
    {
        ACTIVITY,
        FILTERS,

        NUM_FLAGS
    };

    using Pending = std::bitset<NUM_FLAGS>;
    using Map = std::map<QString, int>;
    using MapIter = Map::const_iterator;
    using Counts = std::unordered_map<QString, int>;
    using MapUpdate = QStandardItem* (*)(QStandardItem* i, MapIter const& it);

    void rowsAdd(torrents_t const& torrents);
    void rowsRemove(torrents_t const& torrents);
    void rowsEmitChanged(torrent_ids_t const& ids);
    void recountSoon(Pending const& fields);
    void refreshFilter(Map& map, QStandardItemModel* model, Counts const& counts, MapUpdate update, int key);
    void refreshFilters();

    std::optional<int> getRow(int id) const;
    using span_t = std::pair<int, int>;
    std::vector<span_t> getSpans(torrent_ids_t const& ids) const;

    Prefs const& prefs_;
    torrent_ids_t already_added_;
    torrents_t torrents_;
    QStandardItemModel* path_model_;
    QStandardItemModel* tracker_model_;
    Pending pending_;
    QTimer recount_timer_;
    Map path_counts_;
    Map sitename_counts_;

private slots:
    void recount();

    void recountAllSoon()
    {
        recountSoon(Pending().set(ACTIVITY).set(FILTERS));
    }

    void recountFiltersSoon()
    {
        recountSoon(Pending().set(FILTERS));
    }
};
