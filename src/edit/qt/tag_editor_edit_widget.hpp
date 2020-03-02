// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_EDIT_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_EDIT_WIDGET_HPP

#include "tag_editor_widget.hpp"
#include <QLabel>
#include <QHBoxLayout>

class QLineEdit;

namespace Invader::EditQt {
    class TagEditorEditWidget : public TagEditorWidget {
        Q_OBJECT
        friend class TagEditorWidget;

    protected:
        /**
         * Instantiate a simple edit widget
         * @param parent        parent widget
         * @param value         struct value
         * @param editor_window editor window
         * @param edit_widget   edit widget to use
         */
        TagEditorEditWidget(QWidget *parent, Parser::ParserStructValue *value, TagEditorWindow *editor_window);

    private slots:
        virtual void on_change();

    private:
        QLabel title_label;
        QHBoxLayout hbox_layout;

        void place_textbox(int size, QLabel *prefix);
        std::vector<std::unique_ptr<QWidget>> widgets;
        std::vector<QLineEdit *> textbox_widgets;

    };
}

#endif
