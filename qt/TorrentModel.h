/*
 * This file Copyright (C) 2010-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#pragma once

#include <optional>
#include <vector>

#include <QAbstractListModel>
// #include <QVector>

#include <Typedefs.h>

#include <map>
#include <unordered_map>

class Prefs;
class Speed;
class Torrent;
class QStandardItemModel;
class QStandardItem;

extern "C"
{
struct tr_variant;
}

class TorrentModel : public QAbstractListModel
{
    Q_OBJECT

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
    virtual ~TorrentModel() override;
    void clear();

    bool hasTorrent(QString const& hashString) const;

    Torrent* getTorrentFromId(int id);
    Torrent const* getTorrentFromId(int id) const;

    using torrents_t = QVector<Torrent*>;
    torrents_t const& torrents() const { return myTorrents; }

    // QAbstractItemModel
    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

    QAbstractItemModel* pathModel() const { return myPathProxy; };
    QStandardItemModel* pathFilterModel() const { return myPathModel; }
    QStandardItemModel* trackerFilterModel() const { return myTrackerModel; }

    void refreshModels();

public slots:
    void updateTorrents(tr_variant* torrentList, bool isCompleteList);
    void removeTorrents(tr_variant* torrentList);

signals:
    void torrentsAdded(torrent_ids_t const&);
    void torrentsChanged(torrent_ids_t const&);
    void torrentsCompleted(torrent_ids_t const&);
    void torrentsEdited(torrent_ids_t const&);
    void torrentsNeedInfo(torrent_ids_t const&);
    void filterChanged(int);

private:
    using Map = std::map<QString, int>;
    using MapIter = Map::const_iterator;
    using Counts = std::unordered_map<QString, int>;

private:
    void rowsAdd(torrents_t const& torrents);
    void rowsRemove(torrents_t const& torrents);
    void rowsEmitChanged(torrent_ids_t const& ids);
    void refreshFilter(Map& map, QStandardItemModel* model, Counts& counts, QStandardItem* (*update)(QStandardItem* i,
        MapIter const& it), int key);

    std::optional<int> getRow(int id) const;
    using span_t = std::pair<int, int>;
    std::vector<span_t> getSpans(torrent_ids_t const& ids) const;

    Prefs const& myPrefs;
    torrent_ids_t myAlreadyAdded;
    torrents_t myTorrents;
    QAbstractItemModel* myPathProxy;
    QStandardItemModel* myPathModel;
    QStandardItemModel* myTrackerModel;
    Map myPathCounts;
    Map myTrackerCounts;
};
