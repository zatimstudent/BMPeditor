// styles.h
#ifndef STYLES_H
#define STYLES_H

#include <QString>

namespace Styles {
    const QString ButtonStyle =
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px 16px;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1f6dad;"
        "}";

    const QString InfoPanelStyle =
        "QTextEdit {"
        "  background-color: #f5f5f5;"
        "  border: 1px solid #ddd;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-family: 'Courier New';"
        "}";

    const QString InfoHeaderStyle =
        "QLabel {"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  padding: 5px;"
        "  border-bottom: 1px solid #ddd;"
        "}";

}

#endif // STYLES_H