/*
 * This file Copyright (C) 2012-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#include <map>
#include <unordered_map>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QStandardItemModel>

#include "Filters.h"
#include "FilterBar.h"
#include "FilterBarComboBox.h"
#include "FilterBarComboBoxDelegate.h"
#include "Prefs.h"
#include "Torrent.h"
#include "TorrentFilter.h"
#include "TorrentModel.h"
#include "Utils.h"

#define ActivityRole TorrentModel::ActivityRole
#define CountRole TorrentModel::CountRole
#define CountStringRole TorrentModel::CountStringRole
#define PathRole TorrentModel::PathRole
#define TrackerRole TorrentModel::TrackerRole

/***
****
***/

FilterBarComboBox* FilterBar::createActivityCombo()
{
    FilterBarComboBox* c = new FilterBarComboBox(this);
    FilterBarComboBoxDelegate* delegate = new FilterBarComboBoxDelegate(this, c);
    c->setItemDelegate(delegate);

    QStandardItemModel* model = new QStandardItemModel(this);

    QStandardItem* row = new QStandardItem(tr("All"));
    row->setData(FilterMode::SHOW_ALL, ActivityRole);
    model->appendRow(row);

    model->appendRow(new QStandardItem); // separator
    delegate->setSeparator(model, model->index(1, 0));

    row = new QStandardItem(QIcon::fromTheme(QLatin1String("system-run")), tr("Active"));
    row->setData(FilterMode::SHOW_ACTIVE, ActivityRole);
    model->appendRow(row);

    row = new QStandardItem(QIcon::fromTheme(QLatin1String("go-down")), tr("Downloading"));
    row->setData(FilterMode::SHOW_DOWNLOADING, ActivityRole);
    model->appendRow(row);

    row = new QStandardItem(QIcon::fromTheme(QLatin1String("go-up")), tr("Seeding"));
    row->setData(FilterMode::SHOW_SEEDING, ActivityRole);
    model->appendRow(row);

    row = new QStandardItem(QIcon::fromTheme(QLatin1String("media-playback-pause")), tr("Paused"));
    row->setData(FilterMode::SHOW_PAUSED, ActivityRole);
    model->appendRow(row);

    row = new QStandardItem(QIcon::fromTheme(QLatin1String("dialog-ok")), tr("Finished"));
    row->setData(FilterMode::SHOW_FINISHED, ActivityRole);
    model->appendRow(row);

    row = new QStandardItem(QIcon::fromTheme(QLatin1String("view-refresh")), tr("Verifying"));
    row->setData(FilterMode::SHOW_VERIFYING, ActivityRole);
    model->appendRow(row);

    row = new QStandardItem(QIcon::fromTheme(QLatin1String("process-stop")), tr("Error"));
    row->setData(FilterMode::SHOW_ERROR, ActivityRole);
    model->appendRow(row);

    c->setModel(model);
    return c;
}

/***
****
***/

namespace
{

QString getCountString(int n)
{
    return QString::fromLatin1("%L1").arg(n);
}

FilterBarComboBox* createCombo(QWidget* parent, int role, int count, QStandardItemModel* model)
{
    auto* c = new FilterBarComboBox(parent);
    auto* delegate = new FilterBarComboBoxDelegate(parent, c);
    c->setItemDelegate(delegate);

    auto* row = new QStandardItem(parent->tr("All"));
    row->setData(QString(), role);
    row->setData(count, CountRole);
    row->setData(getCountString(static_cast<size_t>(count)), CountStringRole);
    model->appendRow(row);

    model->appendRow(new QStandardItem); // separator
    delegate->setSeparator(model, model->index(1, 0));

    c->setModel(model);
    return c;
}

} // namespace

void FilterBar::refreshTrackers()
{
    myTorrents.refreshModels();
}

FilterBarComboBox* FilterBar::createTrackerCombo(QStandardItemModel* model)
{
    return createCombo(this, TrackerRole, myTorrents.rowCount(), model);
}

FilterBarComboBox* FilterBar::createPathCombo(QStandardItemModel* model)
{
    return createCombo(this, PathRole, myTorrents.rowCount(), model);
}

/***
****
***/

FilterBar::FilterBar(Prefs& prefs, TorrentModel& torrents, TorrentFilter const& filter, QWidget* parent) :
    QWidget(parent),
    myPrefs(prefs),
    myTorrents(torrents),
    myFilter(filter),
    myRecountTimer(new QTimer(this)),
    myIsBootstrapping(true)
{
    QHBoxLayout* h = new QHBoxLayout(this);
    h->setContentsMargins(3, 3, 3, 3);

    myCountLabel = new QLabel(tr("Show:"), this);
    h->addWidget(myCountLabel);

    myActivityCombo = createActivityCombo();
    h->addWidget(myActivityCombo);

    myPathCombo = createPathCombo(myTorrents.pathFilterModel());
    h->addWidget(myPathCombo);

    myTrackerCombo = createTrackerCombo(myTorrents.trackerFilterModel());
    h->addWidget(myTrackerCombo);

    h->addStretch();

    myLineEdit = new QLineEdit(this);
    myLineEdit->setClearButtonEnabled(true);
    myLineEdit->setPlaceholderText(tr("Search..."));
    myLineEdit->setMaximumWidth(250);
    h->addWidget(myLineEdit, 1);
    connect(myLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));

    // listen for changes from the other players
    connect(&myTorrents, SIGNAL(filterChanged(int)), this, SLOT(refreshPref(int)));
    connect(&myPrefs, SIGNAL(changed(int)), this, SLOT(refreshPref(int)));
    connect(myActivityCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onActivityIndexChanged(int)));
    connect(myPathCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onPathIndexChanged(int)));
    connect(myTrackerCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onTrackerIndexChanged(int)));
    connect(&myTorrents, SIGNAL(modelReset()), this, SLOT(recountSoon()));
    connect(&myTorrents, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(recountSoon()));
    connect(&myTorrents, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(recountSoon()));
    connect(&myTorrents, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(recountSoon()));
    connect(myRecountTimer, SIGNAL(timeout()), this, SLOT(recount()));

    recountSoon();
    refreshTrackers();
    myIsBootstrapping = false;

    // initialize our state
    for (int const key : { Prefs::FILTER_MODE, Prefs::FILTER_TRACKERS })
    {
        refreshPref(key);
    }
}

FilterBar::~FilterBar()
{
    delete myRecountTimer;
}

/***
****
***/

void FilterBar::clear()
{
    myActivityCombo->setCurrentIndex(0);
    myTrackerCombo->setCurrentIndex(0);
    myLineEdit->clear();
}

/***
****
***/

void FilterBar::refreshPref(int key)
{
    switch (key)
    {
    case Prefs::FILTER_MODE:
        {
            FilterMode const m = myPrefs.get<FilterMode>(key);
            QAbstractItemModel* model = myActivityCombo->model();
            QModelIndexList indices = model->match(model->index(0, 0), ActivityRole, m.mode());
            myActivityCombo->setCurrentIndex(indices.isEmpty() ? 0 : indices.first().row());
            break;
        }

    case Prefs::FILTER_TRACKERS:
        {
            auto const displayName = myPrefs.getString(key);
            auto rows = myTorrents.trackerFilterModel()->findItems(displayName);
            if (!rows.isEmpty())
            {
                myTrackerCombo->setCurrentIndex(rows.front()->row());
            }
            else // hm, we don't seem to have this tracker anymore...
            {
                bool const isBootstrapping = myTorrents.trackerFilterModel()->rowCount() <= 2;

                if (!isBootstrapping)
                {
                    myPrefs.set(key, QString());
                }
            }

            break;
        }
    }
}

void FilterBar::onTextChanged(QString const& str)
{
    if (!myIsBootstrapping)
    {
        myPrefs.set(Prefs::FILTER_TEXT, str.trimmed());
    }
}

void FilterBar::onPathIndexChanged(int i)
{
    if (myIsBootstrapping)
    {
        return;
    }

    QString path = myPathCombo->itemData(i, PathRole).toString();
    myPrefs.set(Prefs::FILTER_PATH, path);
}

void FilterBar::onTrackerIndexChanged(int i)
{
    if (!myIsBootstrapping)
    {
        QString str;
        bool const isTracker = !myTrackerCombo->itemData(i, TrackerRole).toString().isEmpty();

        if (!isTracker)
        {
            // show all
        }
        else
        {
            str = myTrackerCombo->itemData(i, TrackerRole).toString();
            int const pos = str.lastIndexOf(QLatin1Char('.'));

            if (pos >= 0)
            {
                str.truncate(pos + 1);
            }
        }

        myPrefs.set(Prefs::FILTER_TRACKERS, str);
    }
}

void FilterBar::onActivityIndexChanged(int i)
{
    if (!myIsBootstrapping)
    {
        FilterMode const mode = myActivityCombo->itemData(i, ActivityRole).toInt();
        myPrefs.set(Prefs::FILTER_MODE, mode);
    }
}

/***
****
***/

void FilterBar::recountSoon()
{
    if (!myRecountTimer->isActive())
    {
        myRecountTimer->setSingleShot(true);
        myRecountTimer->start(800);
    }
}

void FilterBar::recount()
{
    QAbstractItemModel* model = myActivityCombo->model();

    int torrentsPerMode[FilterMode::NUM_MODES] = {};
    myFilter.countTorrentsPerMode(torrentsPerMode);

    for (int row = 0, n = model->rowCount(); row < n; ++row)
    {
        QModelIndex index = model->index(row, 0);
        int const mode = index.data(ActivityRole).toInt();
        int const count = torrentsPerMode[mode];
        model->setData(index, count, CountRole);
        model->setData(index, getCountString(count), CountStringRole);
    }

    refreshTrackers();
}
