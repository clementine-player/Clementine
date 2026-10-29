/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "networkremotesettingspage.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QHostInfo>
#include <QInputDialog>
#include <QNetworkInterface>
#include <QSettings>
#include <QStyle>
#include <QUrl>
#include <algorithm>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

#include "core/application.h"
#include "networkremote/networkremote.h"
#include "networkremote/networkremotehelper.h"
#include "transcoder/transcoder.h"
#include "transcoder/transcoderoptionsdialog.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"
#include "ui_networkremotesettingspage.h"

const char* NetworkRemoteSettingsPage::kPlayStoreUrl =
    "https://play.google.com/store/apps/"
    "details?id=org.clementine_player.remote";

static bool ComparePresetsByName(const TranscoderPreset& left,
                                 const TranscoderPreset& right) {
  return left.name_ < right.name_;
}

NetworkRemoteSettingsPage::NetworkRemoteSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_NetworkRemoteSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("ipodtouchicon", IconLoader::Base));

  // Set here rather than in the .ui file, which can't give translators notes.
  ui_->cast_note->setText(
      // TRANSLATORS: On the Network Remote settings page, under "Use a
      // network remote control". Chromecasts, and other devices Clementine
      // can play on, download the audio from Clementine through the network
      // remote, so they only work while it's on, listening on an address in
      // the device's network.
      tr("Chromecasts and other devices you play on get their music from "
         "Clementine through the network remote, so they need it turned on, "
         "and listening on an address in their network."));
  ui_->allow_streaming->setToolTip(
      // TRANSLATORS: The tooltip of "Allow playing on remote devices". It's
      // for phones running the Clementine Remote app, which can act as the
      // speaker and choose where Clementine plays; Chromecasts don't need it.
      tr("Let a phone or tablet running Clementine Remote act as the speaker, "
         "and choose where Clementine plays: Clementine keeps the playlist and "
         "streams the audio to the device."));

  connect(ui_->options, SIGNAL(clicked()), SLOT(Options()));
  connect(ui_->listen_on_all_addresses, &QCheckBox::toggled, this,
          [this](bool all) {
            ui_->listen_networks->setEnabled(!all);
            ui_->add_network->setEnabled(!all);
            UpdateRemoveButton();
          });
  connect(ui_->add_network, SIGNAL(clicked()), SLOT(AddNetwork()));
  connect(ui_->remove_network, SIGNAL(clicked()), SLOT(RemoveNetwork()));
  connect(ui_->listen_networks, &QListWidget::currentItemChanged, this,
          &NetworkRemoteSettingsPage::UpdateRemoveButton);

  // As the About action.
  const int info_size = style()->pixelMetric(QStyle::PM_SmallIconSize);
  ui_->all_addresses_info->setPixmap(
      IconLoader::Load("help-about", IconLoader::Base)
          .pixmap(info_size, info_size));

  ui_->listen_warning->hide();
  connect(ui_->only_non_public_ip, &QCheckBox::toggled, this,
          &NetworkRemoteSettingsPage::UpdateListenWarning);
  connect(ui_->listen_on_all_addresses, &QCheckBox::toggled, this,
          &NetworkRemoteSettingsPage::UpdateListenWarning);
  connect(ui_->listen_networks, &QListWidget::itemChanged, this,
          &NetworkRemoteSettingsPage::UpdateListenWarning);

  ui_->play_store->installEventFilter(this);

  // Get presets
  QList<TranscoderPreset> presets = Transcoder::GetAllPresets();
  std::sort(presets.begin(), presets.end(), ComparePresetsByName);
  for (const TranscoderPreset& preset : presets) {
    ui_->format->addItem(
        QString("%1 (.%2)").arg(preset.name_, preset.extension_),
        QVariant::fromValue(preset));
  }
}

NetworkRemoteSettingsPage::~NetworkRemoteSettingsPage() { delete ui_; }

bool NetworkRemoteSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == ui_->play_store &&
      event->type() == QEvent::MouseButtonRelease) {
    QDesktopServices::openUrl(QUrl(kPlayStoreUrl));
    return true;
  }

  return SettingsPage::eventFilter(object, event);
}

void NetworkRemoteSettingsPage::Load() {
  QSettings s;

  s.beginGroup(NetworkRemote::kSettingsGroup);

  ui_->use_remote->setChecked(s.value("use_remote").toBool());
  ui_->remote_port->setValue(
      s.value("port", NetworkRemote::kDefaultServerPort).toInt());
  ui_->only_non_public_ip->setChecked(
      s.value("only_non_public_ip", true).toBool());

  ui_->listen_on_all_addresses->setChecked(
      s.value("listen_on_all_addresses", true).toBool());
  const bool all = ui_->listen_on_all_addresses->isChecked();
  ui_->listen_networks->setEnabled(!all);
  ui_->add_network->setEnabled(!all);
  PopulateListenNetworks(s.contains("listen_networks")
                             ? s.value("listen_networks").toStringList()
                             : NetworkRemote::NetworksOfAddresses(
                                   s.value("listen_addresses").toStringList(),
                                   NetworkRemote::PresentAddresses()));
  UpdateListenWarning();
  UpdateRemoveButton();

  // Auth Code, 5 digits
  ui_->use_auth_code->setChecked(s.value("use_auth_code", false).toBool());
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
  ui_->auth_code->setValue(s.value("auth_code", qrand() % 100000).toInt());
#else
  ui_->auth_code->setValue(
      s.value("auth_code", QRandomGenerator::global()->bounded(100000))
          .toInt());
#endif

  ui_->allow_downloads->setChecked(s.value("allow_downloads", false).toBool());
  ui_->allow_streaming->setChecked(s.value("allow_streaming", false).toBool());
  ui_->convert_lossless->setChecked(
      s.value("convert_lossless", false).toBool());

  // Load settings
  QString last_output_format =
      s.value("last_output_format", "audio/x-vorbis").toString();
  for (int i = 0; i < ui_->format->count(); ++i) {
    if (last_output_format ==
        ui_->format->itemData(i).value<TranscoderPreset>().codec_mimetype_) {
      ui_->format->setCurrentIndex(i);
      break;
    }
  }

  ui_->files_root_folder->SetPath(s.value("files_root_folder", "").toString());
  ui_->files_music_extensions->setText(
      s.value("files_music_extensions",
              Application::kDefaultMusicExtensionsAllowedRemotely)
          .toStringList()
          .join(","));

  s.endGroup();

  // Get the right play store badge for this language.
  QString language = dialog()->app()->language_without_region();

  QString badge_filename = ":/playstore/" + language + "_generic_rgb_wo_45.png";
  if (!QFile::exists(badge_filename)) {
    badge_filename = ":/playstore/en_generic_rgb_wo_45.png";
  }

  ui_->play_store->setPixmap(QPixmap(badge_filename));
}

namespace {

// Whether |iface| is worth offering in the "Listen on" list: not loopback,
// connected, and not a bridge that only joins this computer's containers or
// virtual machines, such as docker0. A bridge a network card is part of, as
// when a VM host's LAN address is on br0, is kept.
bool OffersListenNetwork(const QNetworkInterface& iface) {
  const QNetworkInterface::InterfaceFlags flags = iface.flags();
  if ((flags & QNetworkInterface::IsLoopBack) ||
      !(flags & QNetworkInterface::IsRunning)) {
    return false;
  }

#ifdef Q_OS_LINUX
  const QString sys_net = "/sys/class/net/";
  const QDir ports(sys_net + iface.name() + "/brif");
  if (ports.exists()) {
    for (const QString& port :
         ports.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
      // Only network cards have a device.
      if (QFile::exists(sys_net + port + "/device")) return true;
    }
    return false;
  }
#endif

  return true;
}

}  // namespace

void NetworkRemoteSettingsPage::PopulateListenNetworks(
    const QStringList& chosen) {
  ui_->listen_networks->clear();

  // A row for each interface, by name, with its networks and this
  // computer's addresses in them.
  QStringList present;
  QList<QHostAddress> present_addresses;
  // Every interface with an address, including ones the list leaves out.
  QStringList all_addresses;
  QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
  std::sort(interfaces.begin(), interfaces.end(),
            [](const QNetworkInterface& left, const QNetworkInterface& right) {
              return QString::compare(left.humanReadableName(),
                                      right.humanReadableName(),
                                      Qt::CaseInsensitive) < 0;
            });
  for (const QNetworkInterface& iface : interfaces) {
    if (!(iface.flags() & QNetworkInterface::IsUp)) continue;

    QStringList networks;
    QStringList addresses;
    for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
      // As NetworkRemote::PresentAddresses.
      if (entry.ip().protocol() == QAbstractSocket::IPv6Protocol &&
          entry.ip().isLinkLocal()) {
        continue;
      }
      const QString network =
          NetworkRemote::NetworkOf(entry.ip(), entry.prefixLength());
      if (!networks.contains(network)) networks << network;
      addresses << entry.ip().toString();
      present_addresses << entry.ip();
    }
    if (networks.isEmpty()) continue;
    present << networks;
    all_addresses << QString("%1: %2").arg(iface.humanReadableName(),
                                           addresses.join(", "));

    bool checked = false;
    for (const QString& network : networks) {
      if (chosen.contains(network)) checked = true;
    }
    // Still shown when it's chosen, so it can be unchosen, or removed.
    const bool offered = OffersListenNetwork(iface);
    if (!checked && !offered) continue;

    QListWidgetItem* item = new QListWidgetItem(
        // TRANSLATORS: A row in the Network Remote's "Listen on" list. %1 is
        // a network interface's name, such as "Wi-Fi" or "eth0", %2 its
        // networks, such as "192.168.1.0/24", separated by commas, and %3
        // this computer's addresses on them, such as "192.168.1.57",
        // separated by commas.
        tr("%1 · %2\nThis computer is %3")
            .arg(iface.humanReadableName(), networks.join(", "),
                 addresses.join(", ")),
        ui_->listen_networks);
    item->setData(Qt::UserRole, networks);
    item->setData(Qt::UserRole + 1, !offered);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
  }

  ui_->all_addresses_info->setToolTip(
      // TRANSLATORS: The tooltip of the information icon beside "All
      // networks" in the Network Remote's "Listen on" list. %1 is a list, one
      // per line, of this computer's network interfaces and their addresses,
      // such as "eth0: 192.168.1.57".
      tr("Listens on every address this computer has:\n%1")
          .arg(all_addresses.join("\n")));

  for (const QString& network : chosen) {
    if (present.contains(network)) continue;
    AddChosenNetwork(
        network, NetworkRemote::ListenAddresses(false, QStringList() << network,
                                                present_addresses))
        ->setCheckState(Qt::Checked);
  }
}

QListWidgetItem* NetworkRemoteSettingsPage::AddChosenNetwork(
    const QString& network, const QList<QHostAddress>& addresses) {
  QStringList texts;
  for (const QHostAddress& address : addresses) texts << address.toString();

  QListWidgetItem* item = new QListWidgetItem(
      addresses.isEmpty()
          // TRANSLATORS: A row in the Network Remote's "Listen on" list, for
          // a network this computer isn't connected to now, such as a home
          // network while it's at work. %1 is the network, such as
          // "192.168.1.0/24".
          ? tr("%1\nNot connected now. Clementine listens when this "
               "computer joins it.")
                .arg(network)
          // TRANSLATORS: A row in the Network Remote's "Listen on" list, for
          // a network that was added by hand. %1 is the network, such as
          // "192.168.1.0/24", and %2 this computer's addresses in it, such
          // as "192.168.1.57", separated by commas.
          : tr("%1\nThis computer is %2").arg(network, texts.join(", ")),
      ui_->listen_networks);
  item->setData(Qt::UserRole, QStringList() << network);
  item->setData(Qt::UserRole + 1, true);
  item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
  return item;
}

void NetworkRemoteSettingsPage::AddNetwork() {
  QString label =
      // TRANSLATORS: In the dialog that adds a network, or a single
      // address, to the Network Remote's "Listen on" list.
      tr("Network or address to listen on, such as 192.168.1.0/24 or "
         "192.168.1.23:");
  QString text;
  while (true) {
    bool ok = false;
    text = QInputDialog::getText(this, tr("Add network or address"), label,
                                 QLineEdit::Normal, text, &ok);
    if (!ok) return;

    const QString network = NetworkRemote::ParseNetwork(text);
    if (!network.isEmpty()) {
      // Filled again with it chosen, so a network on an interface the list
      // leaves out, such as loopback or docker0, gets that interface's row.
      QStringList chosen;
      for (int i = 0; i < ui_->listen_networks->count(); ++i) {
        const QListWidgetItem* item = ui_->listen_networks->item(i);
        if (item->checkState() == Qt::Checked) {
          chosen << item->data(Qt::UserRole).toStringList();
        }
      }
      if (!chosen.contains(network)) chosen << network;
      PopulateListenNetworks(chosen);

      for (int i = 0; i < ui_->listen_networks->count(); ++i) {
        QListWidgetItem* item = ui_->listen_networks->item(i);
        if (item->data(Qt::UserRole).toStringList().contains(network)) {
          ui_->listen_networks->setCurrentItem(item);
        }
      }
      UpdateListenWarning();
      return;
    }

    // TRANSLATORS: Shown in the dialog that adds a network, or a single
    // address, to the Network Remote's "Listen on" list, when what was typed
    // is neither.
    label =
        tr("Enter a network like 192.168.1.0/24 or fd00::/64, or an "
           "address like 192.168.1.23.");
  }
}

void NetworkRemoteSettingsPage::RemoveNetwork() {
  QListWidgetItem* item = ui_->listen_networks->currentItem();
  if (!item || !item->data(Qt::UserRole + 1).toBool()) return;
  delete item;
  UpdateListenWarning();
  UpdateRemoveButton();
}

void NetworkRemoteSettingsPage::UpdateRemoveButton() {
  // The interfaces the list offers come back whenever the page loads, so
  // only networks added by hand, and interfaces the list leaves out, can be
  // removed.
  const QListWidgetItem* item = ui_->listen_networks->currentItem();
  ui_->remove_network->setEnabled(!ui_->listen_on_all_addresses->isChecked() &&
                                  item &&
                                  item->data(Qt::UserRole + 1).toBool());
}

void NetworkRemoteSettingsPage::UpdateListenWarning() {
  QStringList unreachable;
  if (ui_->only_non_public_ip->isChecked() &&
      !ui_->listen_on_all_addresses->isChecked()) {
    for (int i = 0; i < ui_->listen_networks->count(); ++i) {
      const QListWidgetItem* item = ui_->listen_networks->item(i);
      if (item->checkState() != Qt::Checked) continue;
      for (const QString& network : item->data(Qt::UserRole).toStringList()) {
        if (!NetworkRemote::LocalClientsCanReach(
                QHostAddress::parseSubnet(network).first)) {
          unreachable << network;
        }
      }
    }
  }

  ui_->listen_warning->setText(
      tr("Only connections from the local network are allowed, so nothing "
         "can connect through: %1")
          .arg(unreachable.join(", ")));
  ui_->listen_warning->setVisible(!unreachable.isEmpty());
}

void NetworkRemoteSettingsPage::Save() {
  QSettings s;

  s.beginGroup(NetworkRemote::kSettingsGroup);
  s.setValue("port", ui_->remote_port->value());
  s.setValue("use_remote", ui_->use_remote->isChecked());
  s.setValue("only_non_public_ip", ui_->only_non_public_ip->isChecked());
  s.setValue("listen_on_all_addresses",
             ui_->listen_on_all_addresses->isChecked());
  QStringList listen_networks;
  for (int i = 0; i < ui_->listen_networks->count(); ++i) {
    const QListWidgetItem* item = ui_->listen_networks->item(i);
    if (item->checkState() == Qt::Checked) {
      listen_networks << item->data(Qt::UserRole).toStringList();
    }
  }
  s.setValue("listen_networks", listen_networks);
  s.setValue("use_auth_code", ui_->use_auth_code->isChecked());
  s.setValue("auth_code", ui_->auth_code->value());
  s.setValue("allow_downloads", ui_->allow_downloads->isChecked());
  s.setValue("allow_streaming", ui_->allow_streaming->isChecked());
  s.setValue("convert_lossless", ui_->convert_lossless->isChecked());

  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();
  s.setValue("last_output_format", preset.codec_mimetype_);

  s.setValue("files_root_folder", ui_->files_root_folder->Path());

  QStringList files_music_extensions;
  for (const QString& extension :
       ui_->files_music_extensions->text().split(",")) {
    QString ext = extension.trimmed();
    if (ext.size() > 0 && ext.size() < 8)  // no empty string, less than 8 char
      files_music_extensions << ext;
  }
  s.setValue("files_music_extensions", files_music_extensions);

  s.endGroup();

  if (NetworkRemoteHelper::Instance()) {
    NetworkRemoteHelper::Instance()->ReloadSettings();
  }
}

void NetworkRemoteSettingsPage::Options() {
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();

  TranscoderOptionsDialog dialog(preset, this);
  dialog.set_settings_postfix(NetworkRemote::kTranscoderSettingPostfix);
  dialog.exec();
}
