#include "extsmartplaylistdialog.h"

#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>

ExtSmartPlaylistDialog::ExtSmartPlaylistDialog(PlaylistManager* playlist_manager, QWidget* parent)
    : QDialog(parent), playlist_manager_(playlist_manager) {
  setupUi();
}

ExtSmartPlaylistDialog::~ExtSmartPlaylistDialog() {}

void ExtSmartPlaylistDialog::setupUi() {
  // Main layout
  auto* layout = new QVBoxLayout(this);

  // Name section
  auto* name_layout = new QHBoxLayout;
  auto* name_label = new QLabel(tr("Name:"), this);
  name_edit_ = new QLineEdit(this);
  name_layout->addWidget(name_label);
  name_layout->addWidget(name_edit_);
  layout->addLayout(name_layout);

  // Rules table
  rules_table_ = new QTableWidget(this);
  rules_table_->setColumnCount(4);
  rules_table_->setHorizontalHeaderLabels(
      {tr("Field"), tr("Criteria"), tr("Value"), tr("Action")});
  rules_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  layout->addWidget(rules_table_);

  // Buttons layout
  auto* buttons_layout = new QHBoxLayout;
  auto* add_button = new QPushButton(tr("Add Rule"), this);
  auto* remove_button = new QPushButton(tr("Remove Rule"), this);
  auto* ok_button = new QPushButton(tr("OK"), this);
  auto* cancel_button = new QPushButton(tr("Cancel"), this);

  buttons_layout->addWidget(add_button);
  buttons_layout->addWidget(remove_button);
  buttons_layout->addStretch();
  buttons_layout->addWidget(ok_button);
  buttons_layout->addWidget(cancel_button);
  layout->addLayout(buttons_layout);

  // Connect signals
  connect(add_button, &QPushButton::clicked, this, &ExtSmartPlaylistDialog::addRule);
  connect(remove_button, &QPushButton::clicked, this, &ExtSmartPlaylistDialog::removeRule);
  connect(ok_button, &QPushButton::clicked, this, &QDialog::accept);
  connect(cancel_button, &QPushButton::clicked, this, &QDialog::reject);

  setWindowTitle(tr("Create Smart Playlist"));
  resize(600, 400);
}

void ExtSmartPlaylistDialog::addRule() {
  const int row = rules_table_->rowCount();
  rules_table_->insertRow(row);
  // Add combo boxes and line edits for the new row
}

void ExtSmartPlaylistDialog::removeRule() {
  const QList<QTableWidgetItem*> selected = rules_table_->selectedItems();
  if (!selected.isEmpty()) {
    rules_table_->removeRow(selected.first()->row());
  }
}
