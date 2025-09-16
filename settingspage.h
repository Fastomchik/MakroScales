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

private slots:
    void onSaveButtonClicked();

private:
    QLineEdit *serverIpEdit;
    QSpinBox *serverPortEdit;
    QLineEdit *clientIpEdit;
    QSpinBox *clientPortEdit;
    QPushButton *setSettings;

    void setupUI();

};

#endif // SETTINGSPAGE_H
