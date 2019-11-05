/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "usbselectview.h"

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QListWidget>
#include <QMessageBox>
#include <QIcon>
#include <QPalette>

#include <DCheckBox>

#include <ddialog.h>

#include "suggestbutton.h"
#include "widgetutil.h"
#include "deviceinfoitem.h"
#include "devicelistwidget.h"

#include <bminterface.h>

DWIDGET_USE_NAMESPACE

static QString usageString(quint32 usage, quint32 total)
{
    if (total <= 0 || usage > total) {
        return "0/0G";
    }

    if (total <= 1024) {
        return QString("%1/%2M").arg(usage).arg(total);
    }

    return QString("%1/%2G")
           .arg(QString::number(static_cast<double>(usage) / 1024, 'f', 2))
           .arg(QString::number(static_cast<double>(total) / 1024, 'f', 2));
}


static int percent(quint32 usage, quint32 total)
{
    if (total <= 0) {
        return 0;
    }

    if (usage > total) {
        return 100;
    }

    return static_cast<int>(usage * 100 / total);
}

UsbSelectView::UsbSelectView(QWidget *parent) : QFrame(parent)
{
    setObjectName("UsbSelectView");
    setAutoFillBackground(true);
    QPalette pa;
    pa.setColor(QPalette::Background, QColor(255, 255, 255));
    setPalette(pa);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 9, 0, 0);

    QLabel *m_title = new QLabel(tr("Select a disk"));
    m_title->setFixedHeight(35);
    QFont ft = m_title->font();
    ft.setPixelSize(24);
    m_title->setFont(ft);
//    m_title->setStyleSheet("font-size: 26px;");

    QFrame *usbDeviceListPanel = new QFrame;
    usbDeviceListPanel->setObjectName("UsbDeviceListPanel");
    usbDeviceListPanel->setFixedSize(410, 320);
    usbDeviceListPanel->setAutoFillBackground(true);
    pa = usbDeviceListPanel->palette();
    pa.setColor(QPalette::Background, QColor(255, 255, 255, 128));
    usbDeviceListPanel->setPalette(pa);

    QVBoxLayout *usbPanelLayout = new QVBoxLayout(usbDeviceListPanel);
    usbPanelLayout->setContentsMargins(10, 0, 10, 0);

    QHBoxLayout *checkBoxLayout = new QHBoxLayout;
    checkBoxLayout->setContentsMargins(0, 0, 0, 0);
    checkBoxLayout->setSpacing(0);

    DCheckBox *m_formatDiskCheck = new DCheckBox(this);
    m_formatDiskCheck->setText(tr("Format the disk to increase the burning success rate"));
    m_formatDiskCheck->setObjectName("UsbFormatCheckBox");
    m_formatDiskCheck->setFixedHeight(34);
    m_formatDiskCheck->setFocusPolicy(Qt::NoFocus);
    m_formatDiskCheck->hide();
    ft = m_formatDiskCheck->font();
    ft.setPixelSize(14);
    m_formatDiskCheck->setFont(ft);

//    QLabel *checkBoxHints = new QLabel(this);
//    checkBoxHints->setMinimumHeight(34);
//    checkBoxHints->setWordWrap(true);
//    checkBoxHints->setText(tr("Format the disk to increase the burning success rate"));
////    checkBoxHints->setStyleSheet(WidgetUtil::getQss(":/theme/light/UCheckBox.theme"));
////    checkBoxHints->setMinimumWidth(this->width());
//    ft = checkBoxHints->font();
//    ft.setPointSize(10);
//    checkBoxHints->setFont(ft);
//    checkBoxHints->hide();

    checkBoxLayout->addStretch();
    checkBoxLayout->addWidget(m_formatDiskCheck);
//    checkBoxLayout->addSpacing(4);
//    checkBoxLayout->addWidget(checkBoxHints);
    checkBoxLayout->addStretch();
    checkBoxLayout->setAlignment(Qt::AlignCenter);

    DeviceListWidget *m_deviceList = new DeviceListWidget;
    m_deviceList->setObjectName("UsbDeviceList");
    m_deviceList->setFixedSize(390, 270);
    m_deviceList->hide();

    DeviceDelegate *m_devicedelegate = new DeviceDelegate(m_deviceList);
    m_deviceList->setItemDelegate(m_devicedelegate);

    QLabel *m_warningHint = new  QLabel("");
    m_warningHint->setObjectName("WarningHint");
    m_warningHint->setFixedWidth(370);
    m_warningHint->setMinimumHeight(17);
    m_warningHint->setWordWrap(true);
    ft = m_warningHint->font();
    ft.setPixelSize(11);
    m_warningHint->setFont(ft);
    m_warningHint->setAlignment(Qt::AlignCenter);
    pa = m_warningHint->palette();
    pa.setColor(QPalette::WindowText, QColor("#FF5800"));
    m_warningHint->setPalette(pa);

    QLabel *m_emptyHint = new  QLabel(tr("No disk available"));
    m_emptyHint->setObjectName("EmptyHintTitle");
    m_emptyHint->setFixedHeight(29);
    m_emptyHint->setAlignment(Qt::AlignCenter);
    ft = m_emptyHint->font();
    ft.setPixelSize(20);
    m_emptyHint->setFont(ft);
    pa = m_emptyHint->palette();
    pa.setColor(QPalette::WindowText, QColor(85, 85, 85, 102));
    m_emptyHint->setPalette(pa);

    usbPanelLayout->addStretch();
    usbPanelLayout->addWidget(m_emptyHint, 0, Qt::AlignCenter);
    usbPanelLayout->addStretch();
    usbPanelLayout->addWidget(m_deviceList, 0, Qt::AlignLeft);
    usbPanelLayout->addSpacing(15);
    usbPanelLayout->addLayout(checkBoxLayout);

    SuggestButton *start = new SuggestButton();
    start->setObjectName("StartMake");
    start->setText(tr("Start"));
    start->setDisabled(true);

    mainLayout->addWidget(m_title, 0, Qt::AlignCenter);
    mainLayout->addSpacing(41);
    mainLayout->addWidget(usbDeviceListPanel, 0, Qt::AlignCenter);
    mainLayout->addWidget(m_warningHint, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->addWidget(start, 0, Qt::AlignCenter);

//    this->setStyleSheet(WidgetUtil::getQss(":/theme/light/UsbSelectView.theme"));
    auto handleFormat = [ = ](bool checked) {
        if (!checked) {
            m_warningHint->setText("");
            return;
        }
        m_warningHint->setText(tr("Formatting will erase all data on the disk, please confirm and continue"));
        this->adjustSize();
    };
    connect(m_formatDiskCheck, &QCheckBox::clicked, this, [ = ](bool checked) {
        this->setProperty("user_format", checked);
        handleFormat(checked);
    });

    connect(BMInterface::instance(), &BMInterface::deviceListChanged,
    this, [ = ](const QList<DeviceInfo> &partitions) {
        bool hasPartitionSelected = false;
        m_formatDiskCheck->setVisible(partitions.size());
//        checkBoxHints->setVisible(partitions.size());
        m_emptyHint->setVisible(!partitions.size());
        m_deviceList->setVisible(partitions.size());
//        m_formatDiskCheck->setEnabled(partitions.size());

        m_deviceList->clear();
        foreach (const DeviceInfo &partition, partitions) {
            QListWidgetItem *listItem = new QListWidgetItem;
            DeviceInfoItem *infoItem = new DeviceInfoItem(
                partition.label,
                partition.path,
                usageString(partition.used, partition.total),
                percent(partition.used, partition.total));
            infoItem->setNeedFormat(partition.needFormat);
            listItem->setSizeHint(QSize(infoItem->size().width(), infoItem->size().height() + 10));
            m_deviceList->addItem(listItem);
            m_deviceList->setItemWidget(listItem, infoItem);
            infoItem->setProperty("path", partition.path);
            infoItem->setProperty("fstype", partition.fstype);
            if (partition.path == this->property("last_path").toString()) {
                infoItem->setCheck(true);
                m_deviceList->setCurrentItem(listItem);
                hasPartitionSelected = true;
            }
        }

        start->setDisabled(!hasPartitionSelected);

        if (!hasPartitionSelected) {
            this->setProperty("last_path", "");
        }
    });

    connect(m_deviceList, &DeviceListWidget::currentItemChanged,
    this, [ = ](QListWidgetItem * current, QListWidgetItem * previous) {
        DeviceInfoItem *infoItem = qobject_cast<DeviceInfoItem *>(m_deviceList->itemWidget(previous));
        if (infoItem) {
            infoItem->setCheck(false);
        }

        infoItem = qobject_cast<DeviceInfoItem *>(m_deviceList->itemWidget(current));
        if (infoItem) {
            if (infoItem->needFormat()) {
                m_formatDiskCheck->setChecked(true);
                m_formatDiskCheck->setDisabled(true);
                handleFormat(true);
            } else {
                auto format = this->property("user_format").toBool();
                m_formatDiskCheck->setChecked(format);
                m_formatDiskCheck->setDisabled(false);
                handleFormat(format);
            }

            infoItem->setCheck(true);
            this->setProperty("last_path", infoItem->property("path").toString());
            this->setProperty("last_fstype", infoItem->property("fstype").toString());
            start->setDisabled(false);
        }
    });

    connect(start, &SuggestButton::clicked, this, [ = ] {
        auto format = m_formatDiskCheck->isChecked();

        if (format)
        {
            DDialog msgbox(this);
            msgbox.setFixedWidth(400);
            msgbox.setIcon(QMessageBox::standardIcon(QMessageBox::Warning));
            msgbox.setTitle(tr("Format USB flash drive"));
            msgbox.setTextFormat(Qt::AutoText);
            msgbox.setMessage(tr("Formatting the disk will overwrite all data, please have a backup before proceeding."));
            msgbox.insertButton(0, tr("Cancel"), true, DDialog::ButtonRecommend);
            msgbox.insertButton(1, tr("Ok"), false, DDialog::ButtonWarning);

            auto ret = msgbox.exec();
            if (ret != 1) {
                return;
            }
        }

        start->setEnabled(false);
        if (!m_formatDiskCheck->isChecked() && "vfat" != this->property("last_fstype").toString())
        {
            emit finish(2, "install failed", tr("Disk Format Error: Please format the disk with FAT32"));
            return;
        }
        QString path = this->property("last_path").toString();
        qDebug() << "Select usb device" << path;
        emit this->deviceSelected(path, m_formatDiskCheck->isChecked());
    });
}
