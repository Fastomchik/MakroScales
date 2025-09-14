#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

class SettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPage(QWidget *parent = nullptr);

private:
    QLineEdit *serverIpEdit;
    QSpinBox *serverPortEdit;
    QLineEdit *clientIpEdit;
    QSpinBox *clientPortEdit;
    QPushButton *setSettings;

};

#endif // SETTINGSPAGE_H
