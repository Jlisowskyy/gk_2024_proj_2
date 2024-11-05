//
// Created by Jlisowskyy on 11/5/24.
//

#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

/* internal includes */

/* external includes */
#include <Qt>
#include <QAction>

class TextButton : public QWidget {
    Q_OBJECT

public:
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit TextButton(QWidget *parent, const char *toolTip, const char *title, const char *icon);

    ~TextButton() override = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] QAction *getAction() const { return m_button; }

    // ------------------------------
    // Class fields
    // ------------------------------
protected:
    QAction *m_button;
};


#endif //TEXTBUTTON_H
