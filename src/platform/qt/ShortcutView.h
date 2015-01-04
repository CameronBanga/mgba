/* Copyright (c) 2013-2015 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef QGBA_SHORTCUT_VIEW
#define QGBA_SHORTCUT_VIEW

#include <QWidget>

#include "ui_ShortcutView.h"

namespace QGBA {

class InputController;
class ShortcutController;

// TODO: suspend shortcuts (keyboard and gamepad) while window is open
class ShortcutView : public QWidget {
Q_OBJECT

public:
	ShortcutView(QWidget* parent = nullptr);

	void setController(ShortcutController* controller);
	void setInputController(InputController* controller);

private slots:
	void loadKey(const QModelIndex&);
	void updateKey();
	void updateButton(int button);

private:
	Ui::ShortcutView m_ui;

	ShortcutController* m_controller;
	InputController* m_inputController;
};

}

#endif