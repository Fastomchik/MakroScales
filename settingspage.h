#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage();

    QString getServerIp() const;
    int getServerPort() const;
    QString getClientIp() const;
    int getClientPort() const;

    void loadSettings();

signals:
    void settingsSaved();

private slots:
    void onSaveButtonClicked();

private:
    void setupUI();

    QLineEdit *serverIpEdit;
    QSpinBox *serverPortEdit;
    QLineEdit *clientIpEdit;
    QSpinBox *clientPortEdit;
    QPushButton *setSettings;
};

#endif // SETTINGSPAGE_H
