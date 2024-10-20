// This file Copyright © 2010-2022 Mnemosyne LLC.
// It may be used under GPLv2 (SPDX: GPL-2.0), GPLv3 (SPDX: GPL-3.0),
// or any future license endorsed by Mnemosyne LLC.
// License text can be found in the licenses/ folder.

#pragma once

#include <QComboBox>

#include <libtransmission/tr-macros.h>

class FilterBarComboBox : public QComboBox
{
    Q_OBJECT
    TR_DISABLE_COPY_MOVE(FilterBarComboBox)

public:
    explicit FilterBarComboBox(QWidget* parent = nullptr);

    // QWidget
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    // QWidget
    void paintEvent(QPaintEvent* e) override;

private:
    QSize calculateSize(QSize const& text_size, QSize const& count_size) const;
};
